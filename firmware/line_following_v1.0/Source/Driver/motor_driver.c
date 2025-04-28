/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "motor_driver.h"
#include "pwm_driver.h"
#include "timer_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define STOP_SPEED 0

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
typedef struct sMotorConstDesc {
    eTimerDriver_t timer;
    ePwmDevice_t fwd_channel;
    ePwmDevice_t rev_channel;
} sMotorConstDesc_t;

typedef struct sMotorDynamic {
    bool is_enabled;
    uint16_t max_speed;
    uint16_t current_speed;
} sMotorDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sMotorConstDesc_t g_static_motor_lut[eMotorDriver_Last] = {
    [eMotorDriver_A] = {
        .timer = eTimerDriver_TIM3,
        .fwd_channel = ePwmDevice_MotorA_A2,
        .rev_channel = ePwmDevice_MotorA_A1
    },
    [eMotorDriver_B] = {
        .timer = eTimerDriver_TIM3,
        .fwd_channel = ePwmDevice_MotorB_A2,
        .rev_channel = ePwmDevice_MotorB_A1
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_motor_init = false;

/* clang-format off */
static sMotorDynamic_t g_dynamic_motor_lut[eMotorDriver_Last] = {
    [eMotorDriver_A] = {
        .is_enabled = false,
        .max_speed = 0,
        .current_speed = 0
    },
    [eMotorDriver_B] = {
        .is_enabled = false,
        .max_speed = 0,
        .current_speed = 0
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Motor_Driver_InitAllMotors (void) {
    if (g_is_all_motor_init) {
        return true;
    }

    for (eMotorDriver_t motor = eMotorDriver_First; motor < eMotorDriver_Last; motor++) {
        g_dynamic_motor_lut[motor].max_speed = Timer_Driver_GetResolution(g_static_motor_lut[motor].timer);
    }

    g_is_all_motor_init = true;

    return g_is_all_motor_init;
}

bool Motor_Driver_EnableMotor (const eMotorDriver_t motor) {
    if ((motor < eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
        return false;
    }

    if (g_dynamic_motor_lut[motor].is_enabled) {
        return true;
    }

    if (!PWM_Driver_Enable_Device(g_static_motor_lut[motor].fwd_channel)) {
        return false;
    }

    if (!PWM_Driver_Enable_Device(g_static_motor_lut[motor].rev_channel)) {
        return false;
    }

    g_dynamic_motor_lut[motor].is_enabled = true;

    return g_dynamic_motor_lut[motor].is_enabled;
}

bool Motor_Driver_DisableMotor (const eMotorDriver_t motor) {
    if ((motor < eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
        return false;
    }

    if (!g_dynamic_motor_lut[motor].is_enabled) {
        return true;
    }

    if (!PWM_Driver_Disable_Device(g_static_motor_lut[motor].fwd_channel)) {
        return false;
    }

    if (!PWM_Driver_Disable_Device(g_static_motor_lut[motor].rev_channel)) {
        return false;
    }

    g_dynamic_motor_lut[motor].is_enabled = false;

    return g_dynamic_motor_lut[motor].is_enabled;
}

bool Motor_Driver_SetSpeed (const eMotorDriver_t motor, const eMotorRotation_t rotation_dir, const size_t speed) {
    if ((motor < eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
        return false;
    }

    if ((rotation_dir < eMotorRotation_First) || (rotation_dir >= eMotorRotation_Last)) {
        return false;
    }

    if (speed > g_dynamic_motor_lut[motor].max_speed) {
        return false;
    }

    if (!g_dynamic_motor_lut[motor].is_enabled) {
        return false;
    }

    switch (rotation_dir) {
        case eMotorRotation_Forward: {
            if (!PWM_Driver_Change_Duty_Cycle(g_static_motor_lut[motor].rev_channel, STOP_SPEED)) {
                return false;
            }
            
            if (!PWM_Driver_Change_Duty_Cycle(g_static_motor_lut[motor].fwd_channel, speed)) {
                return false;
            }
        } break;
        case eMotorRotation_Backward: {
            if (!PWM_Driver_Change_Duty_Cycle(g_static_motor_lut[motor].fwd_channel, STOP_SPEED)) {
                return false;
            }
            
            if (!PWM_Driver_Change_Duty_Cycle(g_static_motor_lut[motor].rev_channel, speed)) {
                return false;
            }
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

bool Motor_Driver_GetMaxSpeed (const eMotorDriver_t motor, uint16_t *speed) {
    if ((motor < eMotorDriver_First) || (motor >= eMotorDriver_Last)) {
        return false;
    }

    if (speed == NULL) {
        return false;
    }

    *speed = g_dynamic_motor_lut[motor].max_speed;

    return true;
}
