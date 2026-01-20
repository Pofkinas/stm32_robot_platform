/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "action_wall_follow.h"

#include <stddef.h>
#include <string.h>
#include <math.h>
#include "cmsis_os2.h"
#include "action_measure.h"
#include "motor_api.h"
#include "motor_config.h"
#include "cmd_api_helper.h"
#include "debug_api.h"
#include "heap_api.h"
#include "float_parts.h"

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eWallState {
    eWallState_First = 0,
    eWallState_Follow = eWallState_First,
    eWallState_SpinRight,
    eWallState_SpinLeft,
    eWallState_Last
} eWallState_t;

typedef struct sWallFollowData {
    const sWallFollow_t *config;
    void *instance;
    uint32_t start_time;
    uint32_t last_tick;
    eWallFollowStrategy_t current_strategy;
    eWallState_t current_state;
    sPID_t pid;
    uint16_t right_distance_mm[DATA_SAMPLES];
    uint16_t right_distance_avg_mm;
    uint16_t left_distance_mm[DATA_SAMPLES];
    uint16_t left_distance_avg_mm;
    uint16_t front_distance_mm[DATA_SAMPLES];
    uint16_t front_distance_avg_mm;
    uint16_t front_distance_last_mm;
    uint8_t sample_index;
    uint8_t sample_count;
    bool front_blocked;
    bool right_blocked;
    bool left_blocked;
    bool slow_down;
    osEventFlagsId_t *measure_event;
} sWallFollowData_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 
#ifdef DEBUG_ACTION_WALL_FOLLOW
CREATE_MODULE_NAME (ACTION_WALL_FOLLOW)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_ACTION_WALL_FOLLOW */

static const osMutexAttr_t g_action_mutex_attributes = {
    .name = "Action_WallFollow",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = NULL,
    .cb_size = 0
};

static const osTimerAttr_t g_timeout_timer_attributes = {
    .name = "Action_WallFollow",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static eActionState_t g_action_state = eActionState_Last;
static sWallFollowData_t g_action_data = {0};
static sPID_t g_pid_controller = {0};

static osMutexId_t g_action_mutex = NULL;
static osTimerId_t g_timeout_timer = NULL;
 
static void (*g_em_stop_callback)(const void *instance, eEmergencyType_t type, const char *response) = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void Action_WallFollow_TimeoutCallback (void *arg);
static bool Action_WallFollow_UpdateSensorData (sWallFollowData_t *data);
static void Action_WallFollow_AverageSensorData (sWallFollowData_t *data);
static bool Action_WallFollow_DetectWalls (sWallFollowData_t *data, bool *is_collision);
static void Action_WallFollow_UpdateState (sWallFollowData_t *data);
static int16_t Action_WallFollow_GetError (const sWallFollowData_t data);
static bool Action_WallFollow_UpdateMotors (const float right_speed, const float left_speed);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void Action_WallFollow_TimeoutCallback (void *arg) {
    if (g_em_stop_callback != NULL) {
        g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Wall follow timeout\n");
    }

    return;
}

static bool Action_WallFollow_UpdateSensorData (sWallFollowData_t *data) {
    if (data == NULL) {
        return false;
    }
    
    uint16_t front_dist = 0;
    uint16_t right_dist = 0;
    uint16_t left_dist = 0;

    if (!Action_Measure_GetDistance(FRONT_DISTANCE_SENSOR, &front_dist)) {
        return false;
    }

    if (!Action_Measure_GetDistance(RIGHT_DISTANCE_SENSOR, &right_dist)) {
        return false;
    }

    if (!Action_Measure_GetDistance(LEFT_DISTANCE_SENSOR, &left_dist)) {
        return false;
    }

    data->front_distance_mm[data->sample_index] = front_dist;
    data->right_distance_mm[data->sample_index] = right_dist;
    data->left_distance_mm[data->sample_index] = left_dist;

    data->sample_index = (data->sample_index + 1) % DATA_SAMPLES;

    if (data->sample_count < DATA_SAMPLES) {
        data->sample_count++;
    }

    return true;
}

static void Action_WallFollow_AverageSensorData (sWallFollowData_t *data) {
    if (data == NULL) {
        return;
    }

    uint32_t front_sum = 0;
    uint32_t right_sum = 0;
    uint32_t left_sum = 0;

    for (uint8_t sample = 0; sample < data->sample_count; sample++) {
        front_sum += data->front_distance_mm[sample];
        right_sum += data->right_distance_mm[sample];
        left_sum += data->left_distance_mm[sample];
    }

    data->front_distance_avg_mm = (uint16_t)(front_sum / data->sample_count);
    data->right_distance_avg_mm = (uint16_t)(right_sum / data->sample_count);
    data->left_distance_avg_mm = (uint16_t)(left_sum / data->sample_count);

    return;
}
 
static bool Action_WallFollow_DetectWalls (sWallFollowData_t *data, bool *is_collision) {
    if ((data == NULL) || (is_collision == NULL)) {
        return false;
    }

    if ((data->front_distance_avg_mm != 0) && (data->front_distance_avg_mm < MIN_SAFE_DISTANCE_MM)) {
        *is_collision = true;

        return false;
    }
    
    switch (data->current_strategy) {
        case eWallFollowStrategy_Center: {
            if (data->left_distance_avg_mm < MIN_SAFE_DISTANCE_MM || data->right_distance_avg_mm < MIN_SAFE_DISTANCE_MM) {
                *is_collision = true;

                return false;
            }

            data->front_blocked = (data->front_distance_avg_mm != 0) && (data->front_distance_avg_mm < FRONT_WALL_THRESHOLD_MM);
            data->right_blocked = (data->right_distance_avg_mm < SIDE_WALL_THRESHOLD_MM);
            data->left_blocked = (data->left_distance_avg_mm < SIDE_WALL_THRESHOLD_MM);
        } break;
        case eWallFollowStrategy_Right: {
            if (data->right_distance_avg_mm < MIN_SAFE_DISTANCE_MM) {
                *is_collision = true;

                return false;
            }

            data->front_blocked = (data->front_distance_avg_mm != 0) && (data->front_distance_avg_mm < (FRONT_TO_SIDE_DIFFERENCE + data->config->target_offset_mm));
            data->right_blocked = (data->right_distance_avg_mm < (SIDE_WALL_THRESHOLD_MM + data->config->target_offset_mm));
        } break;
        case eWallFollowStrategy_Left: {
            if (data->left_distance_avg_mm < MIN_SAFE_DISTANCE_MM) {
                *is_collision = true;

                return false;
            }

            data->front_blocked = (data->front_distance_avg_mm != 0) && (data->front_distance_avg_mm < (FRONT_TO_SIDE_DIFFERENCE + data->config->target_offset_mm));
            data->left_blocked = (data->left_distance_avg_mm < (SIDE_WALL_THRESHOLD_MM + data->config->target_offset_mm));
        } break;
        default: {
            TRACE_ERR("CheckSafety: Invalid strategy [%d]\n", data->current_strategy);
            
            return false;
        }  
    }

    return true;
}

static void Action_WallFollow_UpdateState (sWallFollowData_t *data) {
    if (data == NULL) {
        return;
    }

    switch (data->current_state) {
        case eWallState_Follow: {
            if (data->front_blocked) {
                if (!data->right_blocked) {
                    data->current_state = eWallState_SpinRight;
                } else if (!data->left_blocked) {
                    data->current_state = eWallState_SpinLeft;
                } else {
                    // By default assuming robot driving in CW direction
                    data->current_state = eWallState_SpinRight;
                }

                data->front_distance_last_mm = data->front_distance_avg_mm;
                data->slow_down = false;

                return;
            }

            data->slow_down = (data->front_distance_avg_mm != 0) && (data->front_distance_avg_mm < FRONT_WALL_SLOW_DOWN_TRESHOLD_MM);
        } break;
        case eWallState_SpinRight: {
            if (data->front_blocked) {
                return;
            }

            if (data->left_blocked && (data->left_distance_avg_mm < (data->front_distance_last_mm + FRONT_TO_SIDE_DIFFERENCE))) {
                data->current_state = eWallState_Follow;
                data->last_tick = osKernelGetTickCount();
                data->pid.integral = 0.0f;
                data->pid.prev_error = 0.0f;
            }
        } break;
        case eWallState_SpinLeft: {
            if (data->front_blocked) {
                return;
            }

            if (data->right_blocked && (data->right_distance_avg_mm < (data->front_distance_last_mm + FRONT_TO_SIDE_DIFFERENCE))) {
                data->current_state = eWallState_Follow;
                data->last_tick = osKernelGetTickCount();
                data->pid.integral = 0.0f;
                data->pid.prev_error = 0.0f;
            }
        } break;
        default: {
            TRACE_ERR("UpdateState: Invalid wall follow state [%d]\n", data->current_state);
        }
    }

    return;
}

static int16_t Action_WallFollow_GetError (const sWallFollowData_t data) {
    int16_t error = 0;

    // Possitive error -> robot turns right
    switch (data.current_strategy) {
        case eWallFollowStrategy_Center: {
            // Center between walls
            error = ((data.right_distance_avg_mm - data.left_distance_avg_mm) / 2) - data.config->target_offset_mm;
        } break;
        case eWallFollowStrategy_Right: {
            // Follow right wall at target distance
            error = data.right_distance_avg_mm - data.config->target_offset_mm;
        } break;
        case eWallFollowStrategy_Left: {
            // Follow left wall at target distance
            error = data.config->target_offset_mm - data.left_distance_avg_mm;
        } break;
        default: {
            TRACE_ERR("CalculateLateralError: Invalid strategy [%d]\n", data.current_strategy);
            
            return 0;
        }  
    }

    return error;
}

static bool Action_WallFollow_UpdateMotors (const float right_speed, const float left_speed) {
    float right_speed_abs = fabsf(right_speed);
    float left_speed_abs = fabsf(left_speed);

    eMotorDirection_t right_direction = (right_speed > 0.0f) ? eMotorDirection_Forward : eMotorDirection_Reverse;
    eMotorDirection_t left_direction = (left_speed > 0.0f) ? eMotorDirection_Forward : eMotorDirection_Reverse;
    
    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        const sMotor_t *motor_desc = Motor_Config_GetMotorDesc(motor);

        if (motor_desc == NULL) {
            TRACE_ERR("UpdateMotors: Failed to get motor [%d] description\n", motor);
            return false;
        }

        float speed = 0.0f;
        eMotorDirection_t direction = eMotorDirection_Stop;

        if (motor_desc->position == eMotorPosition_Right) {
            speed = right_speed_abs;
            direction = right_direction;
        } else if (motor_desc->position == eMotorPosition_Left) {
            speed = left_speed_abs;
            direction = left_direction;
        }

        if (!Motor_API_SetMotorSpeed(motor, speed, direction, eMotorControl_None)) {
            TRACE_ERR("UpdateMotors: Failed to set motor [%d] speed\n", motor);
            return false;
        }
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Action_WallFollow_Init (void (*em_stop)(const void *instance, eEmergencyType_t type, const char *response)) {
    if (g_action_state != eActionState_Last) {
        return true;
    }

    if (em_stop == NULL) {
        g_action_state = eActionState_Error;
        return false;
    }

    if (!Heap_API_Init()) {
        g_action_state = eActionState_Error;
        return false;
    }

    if (!Motor_API_Init()) {
        g_action_state = eActionState_Error;
        return false;
    }

    g_action_mutex = osMutexNew(&g_action_mutex_attributes);
    
    if (g_action_mutex == NULL) {
        g_action_state = eActionState_Error;
        return false;
    }

    g_timeout_timer = osTimerNew(Action_WallFollow_TimeoutCallback, osTimerOnce, NULL, &g_timeout_timer_attributes);
    
    if (g_timeout_timer == NULL) {
        g_action_state = eActionState_Error;
        return false;
    }

    g_pid_controller.kp = KP;
    g_pid_controller.ki = KI;
    g_pid_controller.kd = KD;
    g_pid_controller.integral_limit = INTEGRAL_LIMIT;

    g_em_stop_callback = em_stop;
    g_action_state = eActionState_Init;

    return true;
}

bool Action_WallFollow_Run (const void *data, void *instance, eActionState_t *state) {
    if ((g_action_state != eActionState_Init) && (g_action_state != eActionState_Completed) && (g_action_state != eActionState_Error)) {
        return false;
    }
    
    if ((state == NULL) || (data == NULL) || (instance == NULL)) {
        g_action_state = eActionState_Error;
        
        return false;
    }

    const sWallFollow_t *params = (const sWallFollow_t *)data;

    if (!Action_WallFollow_IsCorrectSpeed(params->speed) || !Action_WallFollow_IsCorrectStrategy(params->strategy)) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (osMutexAcquire(g_action_mutex, WALL_FOLLOW_MUTEX_TIMEOUT) != osOK) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        return false;
    }

    memset(&g_action_data, 0, sizeof(sWallFollowData_t));
    
    g_action_data.config = params;
    g_action_data.current_strategy = params->strategy;
    g_action_data.current_state = eWallState_Follow;
    g_action_data.instance = instance;
    g_action_data.start_time = osKernelGetTickCount();

    g_action_data.pid = g_pid_controller;
    g_action_data.pid.integral = 0.0f;
    g_action_data.pid.prev_error = 0.0f;
    g_action_data.pid.output_max = (float)(g_action_data.config->speed * MAX_SPEED_CORRECTION_COEF);
    g_action_data.pid.output_min = (float)(-g_action_data.config->speed * MAX_SPEED_CORRECTION_COEF);

    g_action_data.measure_event = Action_Measure_GetEventFlag();

    if (g_action_data.measure_event == NULL) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (params->timeout_ms > 0) {
        if (osTimerStart(g_timeout_timer, params->timeout_ms) != osOK) {
            g_action_state = eActionState_Error;
            *state = g_action_state;
            
            return false;
        }
    }

    g_action_state = eActionState_Running;
    *state = g_action_state;

    osMutexRelease(g_action_mutex);

    if (!Motor_API_EnableAllMotors()) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    return true;
}

bool Action_WallFollow_Service (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, WALL_FOLLOW_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    eActionState_t current_state = g_action_state;
    sWallFollowData_t data = g_action_data;

    osMutexRelease(g_action_mutex);

    switch (current_state) {
        case eActionState_Running: {
            uint32_t flags = osEventFlagsWait(data.measure_event, MEASURE_EVENT_FLAG_NEW_DATA, osFlagsWaitAny, 0);

            if (!(flags & MEASURE_EVENT_FLAG_NEW_DATA)) {
                *state = g_action_state;
                
                return true;
            }

            if (!Action_WallFollow_UpdateSensorData(&data)) {
                TRACE_ERR("Service: Failed to update sensor data\n");
                
                *state = eActionState_Error;
                
                return false;
            }
            
            Action_WallFollow_AverageSensorData(&data);

            if ((data.right_distance_avg_mm == 0) || (data.left_distance_avg_mm == 0)) {
                TRACE_WRN("Service: Invalid sensor data (R:%umm L:%umm)\n", data.right_distance_avg_mm, data.left_distance_avg_mm);
                
                return true;
            }

            bool is_collision = false;

            if (!Action_WallFollow_DetectWalls(&data, &is_collision)) {
                if (is_collision) {
                    if (g_em_stop_callback != NULL) {
                        g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Wall collision\n");
                    }
                } else {
                    TRACE_WRN("Service: Failed to detect walls\n");
                }

                *state = eActionState_Error;
                
                return false;
            }

            Action_WallFollow_UpdateState(&data);

            int16_t error = 0;
            float correction = 0.0f;

            float right_speed = 0;
            float left_speed = 0;

            uint32_t tick = osKernelGetTickCount();

            switch (data.current_state) {
                case eWallState_Follow: {
                    if (data.last_tick != 0) {
                        error = Action_WallFollow_GetError(data);

                        if (abs(error) < ERROR_DEADBAND_MM) {
                            error = 0;
                        }

                        if (error == 0) {
                            correction = 0.0f;
                            data.pid.integral = 0.0f;
                            data.pid.prev_error = 0.0f;
                        } else {
                            float dt = (tick - data.last_tick);
                            correction = Math_Utils_PID_Update(&data.pid, 0.0f, (float)error, dt);
                        }
                    }

                    data.last_tick = tick;

                    right_speed = data.config->speed + correction;
                    left_speed = data.config->speed - correction;

                    if ((right_speed < 0) || (left_speed < 0)) {
                        data.pid.integral = 0.0f;
                    }

                    if (data.slow_down) {
                        right_speed *= SLOW_DOWN_REDUCTION_COEF;
                        left_speed *= SLOW_DOWN_REDUCTION_COEF;
                    }
                } break;
                case eWallState_SpinRight: {
                    right_speed = -data.config->speed * SPIN_SPEED_REDUCTION_COEF;
                    left_speed = data.config->speed * SPIN_SPEED_REDUCTION_COEF;
                } break;
                case eWallState_SpinLeft: {
                    right_speed = data.config->speed * SPIN_SPEED_REDUCTION_COEF;
                    left_speed = -data.config->speed * SPIN_SPEED_REDUCTION_COEF;
                } break;
                default: {
                    TRACE_ERR("Service: Invalid wall follow state [%d]\n", data.current_state);
                    
                    *state = eActionState_Error;
                    
                    return false;
                }  
            }
            
            if (!Action_WallFollow_UpdateMotors(right_speed, left_speed)) {
                *state = eActionState_Error;
                
                return false;
            }

            if (osMutexAcquire(g_action_mutex, WALL_FOLLOW_MUTEX_TIMEOUT) != osOK) {
                *state = eActionState_Error;
                
                return false;
            }

            g_action_data = data;

            osMutexRelease(g_action_mutex);
            
            #ifdef DEBUG_ACTION_WALL_FOLLOW_DATA
            TRACE_INFO("Service: L:%umm F:%umm R:%umm %s | blocked:%d %d %d | L:%ld.%03u%% R:%ld.%03u%% | err:%d corr:%ld.%03u | state:%d strat:%d | time: %ld\n", data.left_distance_avg_mm, data.front_distance_avg_mm, data.right_distance_avg_mm, (data.slow_down) ? "(SLOW)" : "" , data.left_blocked, data.front_blocked, data.right_blocked, FLOAT_INTEGER_PART(left_speed), FLOAT_FRACTIONAL_PART(left_speed, 3), FLOAT_INTEGER_PART(right_speed), FLOAT_FRACTIONAL_PART(right_speed, 3), error, FLOAT_INTEGER_PART(correction), FLOAT_FRACTIONAL_PART(correction, 3), data.current_state, data.current_strategy, tick);
            #endif /* DEBUG_ACTION_WALL_FOLLOW_DATA */
        } break;
        case eActionState_Pause: {} break;
        case eActionState_Continue: {} break;
        default: {
            *state = eActionState_Error;

            return false;
        } break;
    }

    *state = g_action_state;

    return (current_state != eActionState_Error);
}

void Action_WallFollow_Stop (void) {
    if (osMutexAcquire(g_action_mutex, WALL_FOLLOW_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Stop: Failed to acquire mutex\n");
        
        return;
    }

    eActionState_t current_state = g_action_state;

    if (current_state == eActionState_Init) {
        osMutexRelease(g_action_mutex);
        return;
    }

    osMutexRelease(g_action_mutex);

    if (osTimerIsRunning(g_timeout_timer)) {
        osTimerStop(g_timeout_timer);
    }

    if (!Motor_API_StopAllMotors()) {
        TRACE_ERR("Stop: Failed to stop motors\n");
        g_action_state = eActionState_Error;
        
        return;
    }

    if (!Motor_API_DisableAllMotors()) {
        TRACE_ERR("Stop: Failed to disable motors\n");
        g_action_state = eActionState_Error;
        
        return;
    }

    if (!Heap_API_Free((void *)g_action_data.config)) {
        g_action_state = eActionState_Error;
        
        return;
    }

    if (osMutexAcquire(g_action_mutex, WALL_FOLLOW_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Stop: Failed to re-acquire mutex\n");
        
        return;
    }

    uint32_t elapsed_time = osKernelGetTickCount() - g_action_data.start_time;
    g_action_state = eActionState_Init;

    osMutexRelease(g_action_mutex);

    TRACE_INFO("Stop: Wall Follow Stopped; elapsed time: %ld ms\n", elapsed_time);

    return;
}

eErrorCode_t Action_WallFollow_Parse (sMessage_t *arguments, void **data, sMessage_t *response) {
    if ((arguments == NULL) || (data == NULL) || (response == NULL)) {
        return eErrorCode_NULLPTR;
    }

    size_t strategy_value = 0;
    float speed = 0;
    size_t target_offset = 0;
    size_t timeout = 0;

    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgFloat(arguments, &speed, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(arguments, &strategy_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(arguments, &target_offset, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(arguments, &timeout, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    sWallFollow_t *params = Heap_API_Malloc(sizeof(sWallFollow_t));
    
    if (params == NULL) {
        return eErrorCode_NOMEM;
    }

    params->speed = (uint8_t)speed;
    params->strategy = (eWallFollowStrategy_t)strategy_value;
    params->target_offset_mm = (int16_t)target_offset;
    params->timeout_ms = (uint32_t)timeout;

    *data = params;

    return eErrorCode_OK;
}

bool Action_WallFollow_IsCorrectSpeed (const uint8_t speed_percent) {
    return (speed_percent >= MIN_WALL_FOLLOW_SPEED && speed_percent <= MAX_WALL_FOLLOW_SPEED);
}

bool Action_WallFollow_IsCorrectStrategy (const eWallFollowStrategy_t strategy) {
    return (strategy >= eWallFollowStrategy_First && strategy < eWallFollowStrategy_Last);
}

bool Action_WallFollow_SetPID (const sPID_t *pid_params) {
    if (pid_params == NULL) {
        TRACE_ERR("SetPID: NULL argument\n");
        
        return false;
    }

    if (osMutexAcquire(g_action_mutex, WALL_FOLLOW_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("SetPID: Failed to acquire mutex\n");
        
        return false;
    }

    if (pid_params->kp >= 0.0f) {
        g_pid_controller.kp = pid_params->kp;
    }
    
    if (pid_params->ki >= 0.0f) {
        g_pid_controller.ki = pid_params->ki;
    }

    if (pid_params->kd >= 0.0f) {
        g_pid_controller.kd = pid_params->kd;
    }

    if (pid_params->integral_limit >= 0.0f) {
        g_pid_controller.integral_limit = pid_params->integral_limit;
    }

    osMutexRelease(g_action_mutex);

    return true;
}
