#ifndef APPLICATION_KINEMATICS_APP_H_
#define APPLICATION_KINEMATICS_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define HEADING_KP 1.0f
#define HEADING_KI 0.01f
#define HEADING_KD 0.05f
#define HEADING_INTEGRAL_LIMIT 10.0f
#define HEADING_MAX_ANGULAR_VEL 2.0f // rad/s
#define HEADING_ERROR_DEADBAND_RAD 0.01f

#define KINEMATICS_MUTEX_TIMEOUT 0U
#define KINEMATICS_CONTROL_TIMER_MS 50U
#define RPM_REQUEST_TIMEOUT_MS 100U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sHeadingControl {
    float linear_vel_mms;
    float angular_vel_rads;
    float target_heading_rad;
    bool heading_control;
} sHeadingControl_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Kinematics_APP_Init (void);
bool Kinematics_APP_Set (const sHeadingControl_t heading);
bool Kinematics_APP_Stop (void);

float Kinematics_APP_CalcAngularVel (const float heading_rad, const float speed_mms, const float distance_mm);

#endif /* APPLICATION_KINEMATICS_APP_H_ */
