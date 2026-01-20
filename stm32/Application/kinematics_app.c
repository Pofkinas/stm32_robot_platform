/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "kinematics_app.h"

#include <math.h>
#include "cmsis_os2.h"
#include "esp_comm_app.h"
#include "esp_protocol.h"
#include "motor_api.h"
#include "odometry_api.h"
#include "debug_api.h"
#include "message.h"
#include "math_utils.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sKinematicsDesc {
    sPID_t heading_pid;
    sOdometryData_t *odometry_data;
    sHeadingControl_t current_command;
    float prev_angular_vel;
} sKinematicsDesc_t;
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_KINEMATICS_APP
CREATE_MODULE_NAME(KINEMATICS_APP)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_KINEMATICS_APP */

static const osMutexAttr_t g_kinematics_mutex_attributes = {
    .name = "Kinematics_APP",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = NULL,
    .cb_size = 0
};

static const osTimerAttr_t g_control_timer_attributes = {
    .name = "Kinematics_APP",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};
 
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;
static sKinematicsDesc_t g_kinematics_desc = {0};

static osMutexId_t g_kinematics_mutex = NULL;
static osTimerId_t g_control_timer = NULL;
 
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool Kinematics_APP_UpdateMotors (const sKinematicsDesc_t heading, const float angular_vel);
static float Kinematics_APP_NormalizeAngle (float angle);
static void Kinematics_APP_ControlTimerCallback (void *arg);
static bool Kinematics_APP_RequestRPM_Callback (void); 
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool Kinematics_APP_UpdateMotors (const sKinematicsDesc_t heading, const float angular_vel) {
    // Differential drive kinematics
    // v_left = v_linear - (angular_vel * wheelbase/2)
    // v_right = v_linear + (angular_vel * wheelbase/2)
    
    const float half_wheelbase = WHEEL_BASE_MM / 2.0f;
    float left_vel_mms = heading.current_command.linear_vel_mms - (angular_vel * half_wheelbase);
    float right_vel_mms = heading.current_command.linear_vel_mms + (angular_vel * half_wheelbase);
    
    float left_rpm = (left_vel_mms * 60.0f) / WHEEL_CIRCUMFERENCE_MM;
    float right_rpm = (right_vel_mms * 60.0f) / WHEEL_CIRCUMFERENCE_MM;

    for (eMotor_t motor = eMotor_First; motor < eMotor_Last; motor++) {
        const sMotor_t *motor_desc = Motor_Config_GetMotorDesc(motor);

        if (motor_desc == NULL) {
            TRACE_ERR("UpdateMotors: Failed to get motor [%d] description\n", motor);
            
            return false;
        }

        if (motor_desc->position == eMotorPosition_Right) {
            if (!Motor_API_SetTargetRPM(motor, right_rpm, eMotorControl_PID)) {
                TRACE_ERR("UpdateMotors: Failed to set right motor RPM\n");
                
                return false;
            }
        } else if (motor_desc->position == eMotorPosition_Left) {
            if (!Motor_API_SetTargetRPM(motor, left_rpm, eMotorControl_PID)) {
                TRACE_ERR("UpdateMotors: Failed to set left motor RPM\n");
                
                return false;
            }
        }
    }

    TRACE_INFO("UpdateMotors: Set Left RPM: %d, Right RPM: %d\n", (int32_t)(left_rpm + 0.5f), (int32_t)(right_rpm + 0.5f));

    return true;
}

static float Kinematics_APP_NormalizeAngle (float angle) {
    while (angle > M_PI) {
        angle -= 2.0f * M_PI;
    }

    while (angle < -M_PI) {
        angle += 2.0f * M_PI;
    }

    return angle;
}

static void Kinematics_APP_ControlTimerCallback (void *arg) {
    if (osMutexAcquire(g_kinematics_mutex, KINEMATICS_MUTEX_TIMEOUT) != osOK) {
        return;
    }

    float current_heading = g_kinematics_desc.odometry_data->heading;
    float angular_vel = g_kinematics_desc.current_command.angular_vel_rads;
    float target_heading = g_kinematics_desc.current_command.target_heading_rad;
    float heading_error = Kinematics_APP_NormalizeAngle(target_heading - current_heading);
    
    osMutexRelease(g_kinematics_mutex);

    if (fabs(heading_error) <= HEADING_ERROR_DEADBAND_RAD) {
        return;
    }

    // Setpoint is zero error, process value is negative because we want error→0
    float angular_correction = Math_Utils_PID_Update(&g_kinematics_desc.heading_pid, 0.0f, -heading_error, (float)KINEMATICS_CONTROL_TIMER_MS / 1000.0f);
    angular_vel += angular_correction;

    Kinematics_APP_UpdateMotors(g_kinematics_desc, angular_vel);

    return;
}

static bool Kinematics_APP_RequestRPM_Callback (void) {
    sMessage_t rpm_request = {.data = NULL, .size = 0};

    return ESP_Comm_APP_Send(ESP_MAKE_REQUEST(eEspCmd_Rpm), rpm_request, RPM_REQUEST_TIMEOUT_MS);
}
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Kinematics_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }
    
    if (!Motor_API_Init()) {
        TRACE_ERR("Init: Failed to initialize Motor API\n");
        
        return false;
    }
    
    if (!Odometry_API_Init(NULL)) {
        TRACE_ERR("Init: Failed to initialize Odometry API\n");
        
        return false;
    }

    Odometry_API_SetRequestRpmCallback(Kinematics_APP_RequestRPM_Callback);
    
    g_kinematics_desc.heading_pid.kp = HEADING_KP;
    g_kinematics_desc.heading_pid.ki = HEADING_KI;
    g_kinematics_desc.heading_pid.kd = HEADING_KD;
    g_kinematics_desc.heading_pid.integral = 0.0f;
    g_kinematics_desc.heading_pid.prev_error = 0.0f;
    g_kinematics_desc.heading_pid.integral_limit = HEADING_INTEGRAL_LIMIT;
    g_kinematics_desc.heading_pid.output_min = -HEADING_MAX_ANGULAR_VEL;
    g_kinematics_desc.heading_pid.output_max = HEADING_MAX_ANGULAR_VEL;
    
    g_kinematics_desc.odometry_data = Odometry_API_GetDataPointer();

    if (g_kinematics_desc.odometry_data == NULL) {
        TRACE_ERR("Init: Failed to get odometry data pointer\n");
        
        return false;
    }

    g_kinematics_mutex = osMutexNew(&g_kinematics_mutex_attributes);

    if (g_kinematics_mutex == NULL) {
        TRACE_ERR("Init: Failed to create Kinematics mutex\n");
        
        return false;
    }

    g_control_timer = osTimerNew(Kinematics_APP_ControlTimerCallback, osTimerPeriodic, NULL, &g_control_timer_attributes);

    if (g_control_timer == NULL) {
        TRACE_ERR("Init: Failed to create Kinematics control timer\n");
        
        return false;
    }
    
    g_is_initialized = true;

    return g_is_initialized;
}

bool Kinematics_APP_Set (const sHeadingControl_t heading) {
    if (!g_is_initialized) {
        TRACE_ERR("Set: Not initialized\n");
        return false;
    }

    if (osMutexAcquire(g_kinematics_mutex, KINEMATICS_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Set: Failed to acquire Kinematics mutex\n");
        
        return false;
    }

    memcpy(&g_kinematics_desc.current_command, &heading, sizeof(sHeadingControl_t));
    float angular_vel = g_kinematics_desc.current_command.angular_vel_rads;
    
    if (g_kinematics_desc.current_command.heading_control) {
        if (!osTimerIsRunning(g_control_timer)) {
            if (osTimerStart(g_control_timer, KINEMATICS_CONTROL_TIMER_MS) != osOK) {
                TRACE_ERR("Set: Failed to start Kinematics control timer\n");
                
                osMutexRelease(g_kinematics_mutex);
                
                return false;
            }
        }
    } else {
        if (osTimerIsRunning(g_control_timer)) {
            if (!osTimerStop(g_control_timer)) {
                TRACE_ERR("Set: Failed to stop Kinematics control timer\n");
                
                osMutexRelease(g_kinematics_mutex);
                
                return false;
            }
        }
    }

    osMutexRelease(g_kinematics_mutex);

    if (!Motor_API_EnableAllMotors()) {
        TRACE_ERR("Set: Failed to enable motors\n");
        
        return false;
    }
    
    if (!Kinematics_APP_UpdateMotors(g_kinematics_desc, angular_vel)) {
        return false;
    }

    return true;
}

bool Kinematics_APP_Stop (void) {
    if (!g_is_initialized) {
        return false;
    }

    if (osTimerIsRunning(g_control_timer)) {
        if (osTimerStop(g_control_timer) != osOK) {
            TRACE_ERR("Stop: Failed to stop Kinematics control timer\n");

            return false;
        }
    }
    
    if (!Motor_API_StopAllMotors()) {
        TRACE_ERR("Stop: Failed to stop motors\n");
        
        return false;
    }

    if (!Motor_API_DisableAllMotors()) {
        TRACE_ERR("Stop: Failed to disable motors\n");
        
        return false;
    }

    if (osMutexAcquire(g_kinematics_mutex, KINEMATICS_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Stop: Failed to acquire Kinematics mutex\n");
        
        return false;
    }
    
    g_kinematics_desc.heading_pid.integral = 0.0f;
    g_kinematics_desc.heading_pid.prev_error = 0.0f;
    g_kinematics_desc.prev_angular_vel = 0.0f;

    osMutexRelease(g_kinematics_mutex);
    
    return true;
}

float Kinematics_APP_CalcAngularVel (const float heading_rad, const float speed_mms, const float distance_mm) {
    return (speed_mms / distance_mm) * tanf(heading_rad);
}
