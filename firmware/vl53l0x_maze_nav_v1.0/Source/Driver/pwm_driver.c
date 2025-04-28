/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "pwm_driver.h"
#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx_ll_tim.h"
#include "timer_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sPwmOcChannelDesc {
    TIM_TypeDef *periph;
    eTimerDriver_t timer;
    uint32_t channel;
    uint32_t mode;
    uint32_t oc_state;
    uint32_t ocn_state;
    uint32_t compare_value;
    uint32_t oc_polarity;
    uint32_t ocn_polarity;
    uint32_t oc_idle;
    uint32_t ocn_idle;
    void (*fast_mode_fp) (TIM_TypeDef *, uint32_t);
    void (*compare_preload_fp) (TIM_TypeDef *, uint32_t);
    void (*compare_value_fp) (TIM_TypeDef *, uint32_t);
} sPwmOcChannelDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sPwmOcChannelDesc_t g_static_pwm_lut[ePwmDevice_Last] = {
    [ePwmDevice_MotorA_A1] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH1,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH1
    },
    [ePwmDevice_MotorA_A2] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH2,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH2
    },
    [ePwmDevice_MotorB_A1] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH3,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH3
    },
    [ePwmDevice_MotorB_A2] = {
        .periph = TIM3,
        .timer = eTimerDriver_TIM3,
        .channel = LL_TIM_CHANNEL_CH4,
        .mode = LL_TIM_OCMODE_PWM1,
        .oc_state = LL_TIM_OCSTATE_DISABLE,
        .ocn_state = LL_TIM_OCSTATE_DISABLE,
        .compare_value = 0,
        .oc_polarity = LL_TIM_OCPOLARITY_HIGH,
        .ocn_polarity = LL_TIM_OCPOLARITY_HIGH,
        .oc_idle = LL_TIM_OCIDLESTATE_LOW,
        .ocn_idle = LL_TIM_OCIDLESTATE_LOW,
        .fast_mode_fp = LL_TIM_OC_DisableFast,
        .compare_preload_fp = LL_TIM_OC_EnablePreload,
        .compare_value_fp = LL_TIM_OC_SetCompareCH4
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_device_init = false;

/* clang-format off */
static bool g_is_device_enabled[ePwmDevice_Last] = {
    [ePwmDevice_MotorA_A1] = false,
    [ePwmDevice_MotorA_A2] = false,
    [ePwmDevice_MotorB_A1] = false,
    [ePwmDevice_MotorB_A2] = false
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

bool PWM_Driver_InitAllDevices (void) {
    if (g_is_all_device_init) {
        return true;
    }

    g_is_all_device_init = true;
    
    LL_TIM_OC_InitTypeDef channel_oc_init_struct = {0};

    for (ePwmDevice_t device = ePwmDevice_First; device < ePwmDevice_Last; device++) {
        channel_oc_init_struct.OCMode = g_static_pwm_lut[device].mode;
        channel_oc_init_struct.OCState = g_static_pwm_lut[device].oc_state;
        channel_oc_init_struct.OCNState = g_static_pwm_lut[device].ocn_state;
        channel_oc_init_struct.CompareValue = g_static_pwm_lut[device].compare_value;
        channel_oc_init_struct.OCPolarity = g_static_pwm_lut[device].oc_polarity;
        channel_oc_init_struct.OCNPolarity = g_static_pwm_lut[device].ocn_polarity;
        channel_oc_init_struct.OCIdleState = g_static_pwm_lut[device].oc_idle;
        channel_oc_init_struct.OCNIdleState = g_static_pwm_lut[device].ocn_idle;
    
        if (LL_TIM_OC_Init(g_static_pwm_lut[device].periph, g_static_pwm_lut[device].channel, &channel_oc_init_struct) == ERROR) {
            g_is_all_device_init = false;
        }
    
        if (g_static_pwm_lut[device].fast_mode_fp != NULL) {
            g_static_pwm_lut[device].fast_mode_fp(g_static_pwm_lut[device].periph, g_static_pwm_lut[device].channel);
        }
    
        if (g_static_pwm_lut[device].compare_preload_fp != NULL) {
            g_static_pwm_lut[device].compare_preload_fp(g_static_pwm_lut[device].periph, g_static_pwm_lut[device].channel);
        }
    }

    return g_is_all_device_init;
}

bool PWM_Driver_Enable_Device (const ePwmDevice_t device) {
    if ((device < ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (g_is_device_enabled[device]) {
        return true;
    }

    if (!Timer_Driver_Start(g_static_pwm_lut[device].timer)) {
        return false;
    }

    LL_TIM_CC_EnableChannel(g_static_pwm_lut[device].periph, g_static_pwm_lut[device].channel);

    g_is_device_enabled[device] = true;

    return g_is_device_enabled[device];
}

bool PWM_Driver_Disable_Device (const ePwmDevice_t device) {
    if ((device < ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (!g_is_device_enabled[device]) {
        return true;
    }

    if (!Timer_Driver_Stop(g_static_pwm_lut[device].timer)) {
        return false;
    }

    LL_TIM_CC_DisableChannel(g_static_pwm_lut[device].periph, g_static_pwm_lut[device].channel);

    g_is_device_enabled[device] = false;

    return g_is_device_enabled[device];
}

bool PWM_Driver_Change_Duty_Cycle (const ePwmDevice_t device, const size_t value) {
    if ((device < ePwmDevice_First) || (device >= ePwmDevice_Last)) {
        return false;
    }

    if (!g_is_all_device_init) {
        return false;
    }

    if (!g_is_device_enabled[device]) {
        return false;
    }

    if (value > UINT16_MAX) {
        return false;
    }

    if (!LL_TIM_IsEnabledCounter(g_static_pwm_lut[device].periph)) {
        return false;
    }

    g_static_pwm_lut[device].compare_value_fp(g_static_pwm_lut[device].periph, value);

    return true;
}
