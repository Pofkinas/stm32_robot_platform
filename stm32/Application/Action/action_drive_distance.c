/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "action_drive_distance.h"

#include <stddef.h>
#include <string.h>
#include "cmsis_os2.h"
#include "kinematics_app.h"
#include "odometry_api.h"
#include "cmd_api_helper.h"
#include "debug_api.h"
#include "heap_api.h"
#include "math_utils.h"

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sDriveData {
    float current_distance_mm;
    float current_heading_rad;
    const sDriveDistance_t *target;
    eOdomDataState_t data_state;
    sOdometryData_t *odometry;
    uint32_t start_time;
    uint32_t remaining_timeout;
    void *instance;
} sDriveData_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_ACTION_DRIVE_DISTANCE
CREATE_MODULE_NAME (DRIVE_DISTANCE)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_ACTION_DRIVE_DISTANCE */

static const osTimerAttr_t g_timeout_timer_attributes = {
    .name = "Action_DriveDistance",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

static const osMutexAttr_t g_action_mutex_attributes = {
    .name = "Action_DriveDistance",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = NULL,
    .cb_size = 0
};

static const osEventFlagsAttr_t g_event_flag_attributes = {
    .name = "Action_DriveDistance",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};
 
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static eActionState_t g_action_state = eActionState_Last;
static sDriveData_t g_action_data = {0};

static osTimerId_t g_timeout_timer_id = NULL;
static osMutexId_t g_action_mutex = NULL;
static osEventFlagsId_t g_event_flag = NULL;

static void (*g_em_stop_callback)(const void *instance, eEmergencyType_t type, const char *response) = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void Action_DriveDistance_TimeoutCallback (void *arg);
static bool Action_DriveDistance_Continue (eActionState_t *state);
static bool Action_DriveDistance_Pause (eActionState_t *state);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void Action_DriveDistance_TimeoutCallback (void *arg) {
    if (g_em_stop_callback != NULL) {
        g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Timeout reached\n");
    }

    return;
}

static bool Action_DriveDistance_Continue (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }
    
    if (*state != eActionState_Pause) {
        return false;
    }

    if (g_action_data.remaining_timeout != 0) {
        if (osTimerStart(g_timeout_timer_id, g_action_data.remaining_timeout) != osOK) {
            *state = eActionState_Error;

            return false;
        }
    }

    *state = eActionState_Running;

    return true;
}

static bool Action_DriveDistance_Pause (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }
    
    if (*state != eActionState_Running) {
        return false;
    }

    if (g_action_data.target->timeout_ms != 0) {
        if (osTimerIsRunning(g_timeout_timer_id)) {
            osTimerStop(g_timeout_timer_id);
        }

        uint32_t elapsed = osKernelGetTickCount() - g_action_data.start_time;
        g_action_data.remaining_timeout = (g_action_data.target->timeout_ms > elapsed) ? (g_action_data.target->timeout_ms - elapsed) : 0;
    }
    
    *state = eActionState_Pause;

    sHeadingControl_t new_heading = {0};
    new_heading.linear_vel_mms = 0;
    new_heading.angular_vel_rads = 0;
    new_heading.target_heading_rad = g_action_data.current_heading_rad;
    new_heading.heading_control = true;
    
    if (!Kinematics_APP_Set(new_heading)) {
        if (g_em_stop_callback != NULL) {
            g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Failed to set Kinematics heading\n");
        }

        *state = g_action_state;
        
        return false;
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Action_DriveDistance_Init (void (*em_stop)(const void *instance, eEmergencyType_t type, const char *response)) {
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

    g_event_flag = osEventFlagsNew(&g_event_flag_attributes);
    
    if (g_event_flag == NULL) {
        g_action_state = eActionState_Error;

        return false;
    }

    if (!Odometry_API_Init(&g_event_flag)) {
        g_action_state = eActionState_Error;

        return false;
    }

    if (!Kinematics_APP_Init()) {
        g_action_state = eActionState_Error;

        return false;
    }

    g_timeout_timer_id = osTimerNew(Action_DriveDistance_TimeoutCallback, osTimerOnce, NULL, &g_timeout_timer_attributes);
    
    if (g_timeout_timer_id == NULL) {
        g_action_state = eActionState_Error;

        return false;
    }

    g_action_mutex = osMutexNew(&g_action_mutex_attributes);

    if (g_action_mutex == NULL) {
        g_action_state = eActionState_Error;

        return false;
    }

    g_em_stop_callback = em_stop;
    g_action_state = eActionState_Init;
    
    return true;
}

bool Action_DriveDistance_Run (const void *data, void *instance, eActionState_t *state) {
    if ((g_action_state != eActionState_Init) && (g_action_state != eActionState_Completed) && (g_action_state != eActionState_Error)) {
        return false;
    }
    
    if ((state == NULL) || (data == NULL) || (instance == NULL)) {
        g_action_state = eActionState_Error;
        
        return false;
    }

    const sDriveDistance_t *params = (const sDriveDistance_t *)data;

    if (!Action_DriveDistance_IsCorrectDistance(params->distance_mm)) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (!Action_DriveDistance_IsCorrectSpeed(params->speed_mms)) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (!Action_DriveDistance_IsCorrectHeading(params->heading_deg)) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (osMutexAcquire(g_action_mutex, DRIVE_DISTANCE_MUTEX_TIMEOUT) != osOK) {
        g_action_state = eActionState_Error;
        *state = g_action_state;

        return false;
    }

    memset(&g_action_data, 0, sizeof(sDriveData_t));
    
    g_action_data.target = params;
    g_action_data.odometry = Odometry_API_GetDataPointer();
    g_action_data.instance = instance;

    if (g_action_data.odometry == NULL) {
        g_action_state = eActionState_Error;
        *state = g_action_state;

        osMutexRelease(g_action_mutex);

        return false;
    }

    if (osTimerIsRunning(g_timeout_timer_id)) {
        osTimerStop(g_timeout_timer_id);
    }

    if (g_action_data.target->timeout_ms != 0) {
        if (osTimerStart(g_timeout_timer_id, g_action_data.target->timeout_ms) != osOK) {
            g_action_state = eActionState_Error;
            *state = g_action_state;

            osMutexRelease(g_action_mutex);

            return false;
        }

        g_action_data.start_time = osKernelGetTickCount();
    }

    g_action_state = eActionState_Running;
    *state = g_action_state;

    osMutexRelease(g_action_mutex);

    osEventFlagsSet(g_event_flag, NEW_DATA_FLAG);

    return true;
}

bool Action_DriveDistance_Service (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, DRIVE_DISTANCE_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    eActionState_t current_state = g_action_state;

    osMutexRelease(g_action_mutex);

    switch (current_state) {
        case eActionState_Running: {
            uint32_t flags = osEventFlagsWait(g_event_flag, ODOMETRY_DATA_FAULT_FLAG | NEW_DATA_FLAG, osFlagsWaitAny, EVENT_FLAG_TIMEOUT);            
            if (flags & ODOMETRY_DATA_FAULT_FLAG) {
                if (g_em_stop_callback != NULL) {
                    g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Odometry data fault\n");
                }

                *state = g_action_state;
                
                return false;
            }

            if (osMutexAcquire(g_action_mutex, DRIVE_DISTANCE_MUTEX_TIMEOUT) != osOK) {
                g_action_state = eActionState_Error;
                *state = g_action_state;

                return false;
            }

            g_action_data.current_distance_mm = g_action_data.odometry->distance;
            g_action_data.current_heading_rad = g_action_data.odometry->heading;
            g_action_data.data_state = g_action_data.odometry->data_state;
            float linear_vel = g_action_data.odometry->linear_vel;
            float angular_vel = g_action_data.odometry->angular_vel;

            if (g_action_data.current_distance_mm >= g_action_data.target->distance_mm) {
                if (osTimerIsRunning(g_timeout_timer_id) == osOK) {
                    osTimerStop(g_timeout_timer_id);
                }

                g_action_state = eActionState_Completed;
                osMutexRelease(g_action_mutex);
                *state = g_action_state;

                return true;
            }

            if (!(flags & NEW_DATA_FLAG) || (flags & osFlagsError) == osFlagsError) {
                osMutexRelease(g_action_mutex);
                *state = g_action_state;

                TRACE_INFO("DriveDistance: cur_dis: %d, cur_head %d, speed: %d, ang_vel: %d\n",(int32_t)(g_action_data.current_distance_mm + 0.5f), (int32_t)(Math_Utils_RadiansToDegrees(g_action_data.current_heading_rad) + 0.5f), (int32_t)(linear_vel + 0.5f), (int32_t)(angular_vel + 0.5f));

                return true;
            }

            float heading_rad = Math_Utils_DegreesToRadians(g_action_data.target->heading_deg);

            sHeadingControl_t new_heading = {0};
            new_heading.linear_vel_mms = g_action_data.target->speed_mms;
            new_heading.angular_vel_rads = Kinematics_APP_CalcAngularVel(heading_rad, g_action_data.target->speed_mms, (g_action_data.target->distance_mm - g_action_data.current_distance_mm));
            new_heading.target_heading_rad = heading_rad - g_action_data.current_heading_rad;
            new_heading.heading_control = true;

            osMutexRelease(g_action_mutex);

            if (!Kinematics_APP_Set(new_heading)) {
                if (g_em_stop_callback != NULL) {
                    g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Failed to set Kinematics heading\n");
                }
                
                *state = g_action_state;

                return false;
            }

            TRACE_INFO("DriveDistance: cur_dis: %d, cur_head %d, speed: %d, ang_vel: %d\n",(int32_t)(g_action_data.current_distance_mm + 0.5f), (int32_t)(Math_Utils_RadiansToDegrees(g_action_data.current_heading_rad) + 0.5f), (int32_t)(new_heading.linear_vel_mms + 0.5f), (int32_t)(Math_Utils_RadiansToDegrees(new_heading.angular_vel_rads) + 0.5f));

            *state = g_action_state;
        } break;
        case eActionState_Pause: {
            return true;
        } break;
        case eActionState_Continue: {
            return true;
        } break;
        default: {
            g_action_state = eActionState_Error;
            *state = g_action_state;

            return false;
        } break;
    }

    return true;
}

void Action_DriveDistance_Stop (void) {
    if (g_action_state == eActionState_Init) {
        return;
    }

    if (osTimerIsRunning(g_timeout_timer_id)) {
        osTimerStop(g_timeout_timer_id);
    }
    
    Kinematics_APP_Stop();
    Odometry_API_Reset();

    if (!Heap_API_Free((void *)g_action_data.target)) {
        g_action_state = eActionState_Error;
        
        return;
    }

    g_action_state = eActionState_Init;

    TRACE_INFO("Stop: Drive Distance Stopped\n");

    return;
}

eErrorCode_t Action_DriveDistance_Parse (sMessage_t *arguments, void **data, sMessage_t *response) {
    if ((arguments == NULL) || (data == NULL) || (response == NULL)) {
        return eErrorCode_NULLPTR;
    }

    size_t distance_mm = 0;
    size_t speed_mms = 0;
    float heading_deg = 0.0f;
    size_t timeout_ms = 0;

    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(arguments, &distance_mm, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(arguments, &speed_mms, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(arguments, &heading_deg, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgUInt(arguments, &timeout_ms, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    sDriveDistance_t *params = Heap_API_Malloc(sizeof(sDriveDistance_t));

    if (params == NULL) {
        return eErrorCode_NOMEM;
    }

    params->distance_mm = (uint16_t)distance_mm;
    params->speed_mms = (uint8_t)speed_mms;
    params->heading_deg = heading_deg;
    params->timeout_ms = (uint16_t)timeout_ms;

    *data = params;

    return eErrorCode_OK;
}

bool Action_DriveDistance_SetState (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }

    if ((*state <= eActionState_First) || (*state >= eActionState_Last)) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, DRIVE_DISTANCE_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    bool is_success = true;

    switch (*state) {
        case eActionState_Pause: {
            if (!Action_DriveDistance_Pause(&g_action_state)) {
                is_success = false;
            }
        } break;
        case eActionState_Continue: {
            if (!Action_DriveDistance_Continue(&g_action_state)) {
                is_success = false;
            }
        } break;
        default: {
            is_success = false;
        } break;
    }

    *state = g_action_state;
    osMutexRelease(g_action_mutex);

    return is_success;
}

bool Action_DriveDistance_UpdateData (sDriveDistance_t *data) {
    if (data == NULL) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, DRIVE_DISTANCE_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    memcpy((void *)data, (const void *)g_action_data.target, sizeof(sDriveDistance_t));

    osMutexRelease(g_action_mutex);

    osEventFlagsSet(g_event_flag, NEW_DATA_FLAG);

    return true;
}

bool Action_DriveDistance_IsCorrectDistance (const uint16_t distance_mm) {
    return (distance_mm >= MIN_DRIVE_DISTANCE_MM) && (distance_mm <= MAX_DRIVE_DISTANCE_MM);
}

bool Action_DriveDistance_IsCorrectSpeed (const uint8_t speed_mms) {
    return (speed_mms >= MIN_SPEED_MMS) && (speed_mms <= MAX_SPEED_MMS);
}

bool Action_DriveDistance_IsCorrectHeading (const float heading_deg) {
    return (heading_deg >= MIN_HEADING_DEG) && (heading_deg <= MAX_HEADING_DEG);
}
