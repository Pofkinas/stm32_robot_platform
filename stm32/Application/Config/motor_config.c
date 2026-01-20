/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sMotorDriverDesc_t g_static_motor_driver_lut[eMotor_Last] = {
    [eMotor_A] = {
        .timer = eTimer_TIM3,
        .pwm_1 = ePwm_MotorA,
        .in1 = eGpio_MotorA_In1,
        .in2 = eGpio_MotorA_In2
    },
    [eMotor_B] = {
        .timer = eTimer_TIM3,
        .pwm_1 = ePwm_MotorB,
        .in1 = eGpio_MotorB_In1,
        .in2 = eGpio_MotorB_In2
    }
};

const static sMotor_t g_static_motor_lut[eMotor_Last] = {
    [eMotor_A] = {
        .mutex_attributes = {.name = "Motor_A_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .timer_attributes = {.name = "Motor_A_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0U},
        .motor_speed_offset = MOTOR_A_SPEED_OFFSET,
        .position = eMotorPosition_Right,
        .rotation = {
            [eMotorDirection_Forward] = eMotorRotation_CW,
            [eMotorDirection_Reverse] = eMotorRotation_CCW,
            [eMotorDirection_Right] =  eMotorRotation_CCW,
            [eMotorDirection_RightSoft] = eMotorRotation_CW,
            [eMotorDirection_Left] = eMotorRotation_CW,
            [eMotorDirection_LeftSoft] = eMotorRotation_CW,
            [eMotorDirection_Brake] = eMotorRotation_Brake,
            [eMotorDirection_BrakeLock] = eMotorRotation_Brake,
            [eMotorDirection_Stop] = eMotorRotation_Stop
        }
    },
    [eMotor_B] = {
        .mutex_attributes = {.name = "Motor_B_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .timer_attributes = {.name = "Motor_B_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0U},
        .motor_speed_offset = MOTOR_B_SPEED_OFFSET,
        .position = eMotorPosition_Left,
        .rotation = {
            [eMotorDirection_Forward] = eMotorRotation_CW,
            [eMotorDirection_Reverse] = eMotorRotation_CCW,
            [eMotorDirection_Right] =  eMotorRotation_CCW,
            [eMotorDirection_RightSoft] = eMotorRotation_CW,
            [eMotorDirection_Left] = eMotorRotation_CW,
            [eMotorDirection_LeftSoft] = eMotorRotation_CW,
            [eMotorDirection_Brake] = eMotorRotation_Brake,
            [eMotorDirection_BrakeLock] = eMotorRotation_Brake,
            [eMotorDirection_Stop] = eMotorRotation_Stop
        }
    }
};

#ifdef ENABLE_PID_CONTROL
static sPID_t g_motor_pid_params[eMotor_Last] = {
    [eMotor_A] = {
        .kp = 1.5,
        .ki = 0.05,
        .kd = 0.01,
        .integral_limit = 50,
    },
    [eMotor_B] = {
        .kp = 1.5,
        .ki = 0.05,
        .kd = 0.01,
        .integral_limit = 50,
    }
};
#endif /* ENABLE_PID_CONTROL */

/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

const osThreadAttr_t g_motor_thread_attributes = {
    .name = "Motor_APP",
    .stack_size = 128 * 5,
    .priority = (osPriority_t) osPriorityNormal
};

const osMessageQueueAttr_t g_motor_message_queue_attributes = {
    .name = "Motor_APP",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
    .mq_mem = NULL,
    .mq_size = 0
};

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Motor_Config_IsCorrectMotor (const eMotor_t motor) {
    return (motor >= eMotor_First) && (motor < eMotor_Last);
}

bool Motor_Config_IsCorrectDirection (const eMotorDirection_t direction) {
    return (direction >= eMotorDirection_First) && (direction < eMotorDirection_Last);
}

const sMotorDriverDesc_t *Motor_Config_GetMotorDriverDesc (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return NULL;
    }

    return &g_static_motor_driver_lut[motor];
}

const sMotor_t *Motor_Config_GetMotorDesc (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return NULL;
    }

    return &g_static_motor_lut[motor];
}

#ifdef ENABLE_PID_CONTROL
const sPID_t *Motor_Config_GetMotorPIDParams (const eMotor_t motor) {
    if (!Motor_Config_IsCorrectMotor(motor)) {
        return NULL;
    }

    return &g_motor_pid_params[motor];
}
#endif /* ENABLE_PID_CONTROL */
