#ifndef CONFIG_MOTOR_CONFIG_H_
#define CONFIG_MOTOR_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include "cmsis_os2.h"
#include "pwm_config.h"
#include "timer_config.h"

#ifdef USE_TB6612FNG
#include "gpio_config.h"
#endif /* USE_TB6612FNG */

#ifdef ENABLE_PID_CONTROL
#include "math_utils.h"
#endif /* ENABLE_PID_CONTROL */

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

// TODO: There should be a polynomial function to calculate offset
#define MOTOR_A_SPEED_OFFSET 1.0f // Speed offset to compensate motor inequality
#define MOTOR_B_SPEED_OFFSET 1.07f

#define STOP_SPEED 0
#define MOTOR_DIRECTION_CHANGE_DELAY_MS 50U

#define INPUT_MIN_SPEED 0.0f
#define INPUT_MAX_SPEED 100.0f

/// Speed scaling limits (%)
#define MIN_SCALED_SPEED 18.0f
#define MAX_SCALED_SPEED 100.0f
#define SOFT_TURN_SPEED_OFFSET 10.0f

/// Ramp configuration
#define MOTOR_RAMP_STEPS 10
#define MOTOR_RAMP_SPEED_THRESHOLD 25
#define MOTOR_RAMP_TIMER_MS 2U
#define MOTOR_BRAKE_TIME_MS 10U

/// PWM Control configuration
#define CONTROL_PERIOD_MS 50U

#define MAX_RPM 400.0f
#define MIN_RPM 10.0f
#define MAX_PWM 255
#define BRAKE_RPM_THRESHOLD 5.0f
#define CHANGE_DIR_THRESHOLD 0.1 

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eMotor {
    eMotor_First = 0,
    eMotor_A = eMotor_First,
    eMotor_B,
    eMotor_Last
} eMotor_t;

typedef enum eMotorPosition {
    eMotorPosition_First = 0,
    eMotorPosition_Right = eMotorPosition_First,
    eMotorPosition_Left,
    eMotorPosition_Center,
    eMotorPosition_Last
} eMotorPosition_t;

typedef enum eMotorRotation {
    eMotorRotation_First = 0,
    eMotorRotation_CW = eMotorRotation_First,
    eMotorRotation_CCW,
    eMotorRotation_Stop,
    eMotorRotation_Brake,
    eMotorRotation_Last
} eMotorRotation_t;

typedef enum eMotorDirection {
    eMotorDirection_First = 0,
    eMotorDirection_Forward = eMotorDirection_First,
    eMotorDirection_Reverse,
    eMotorDirection_Right,
    eMotorDirection_RightSoft,
    eMotorDirection_Left,
    eMotorDirection_LeftSoft,
    eMotorDirection_Brake,
    eMotorDirection_BrakeLock,
    eMotorDirection_Stop,
    eMotorDirection_Last
} eMotorDirection_t;

typedef struct sMotorDriverDesc {
    eTimer_t timer;
    ePwm_t pwm_1;
    #ifdef USE_MX1508
    ePwm_t pwm_2;
    #endif /* USE_MX1508 */
    #ifdef USE_TB6612FNG
    eGpio_t in1;
    eGpio_t in2;
    #endif /* USE_TB6612FNG */
} sMotorDriverDesc_t;

typedef struct sMotor {
    osMutexAttr_t mutex_attributes;
    osTimerAttr_t timer_attributes;
    float motor_speed_offset;
    eMotorPosition_t position;
    eMotorRotation_t rotation[eMotorDirection_Last];
} sMotor_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern const osThreadAttr_t g_motor_thread_attributes;
extern const osMessageQueueAttr_t g_motor_message_queue_attributes;

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_Config_IsCorrectMotor (const eMotor_t motor);
bool Motor_Config_IsCorrectDirection (const eMotorDirection_t direction);
const sMotorDriverDesc_t *Motor_Config_GetMotorDriverDesc (const eMotor_t motor);
const sMotor_t *Motor_Config_GetMotorDesc (const eMotor_t motor);

#ifdef ENABLE_PID_CONTROL
const sPID_t *Motor_Config_GetMotorPIDParams (const eMotor_t motor);
#endif /* ENABLE_PID_CONTROL */

#endif /* CONFIG_MOTOR_CONFIG_H_ */
