/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "pwm_config.h"

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
const static sPwmOcDesc_t g_static_oc_pwm_lut[ePwm_Last] = {
    [ePwm_MotorA] = {
        .periph = TIM3,
        .timer = eTimer_TIM3,
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
        .compare_value_fp = LL_TIM_OC_SetCompareCH3,
        .is_dma_request_enabled = false,
    },
    [ePwm_MotorB] = {
        .periph = TIM3,
        .timer = eTimer_TIM3,
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
        .compare_value_fp = LL_TIM_OC_SetCompareCH4,
        .is_dma_request_enabled = false,
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

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

bool PWM_Config_IsCorrectPwm (const ePwm_t pwm_channel) {
    return (pwm_channel >= ePwm_First) && (pwm_channel < ePwm_Last);
}

const sPwmOcDesc_t *PWM_Config_GetPwmOcDesc (const ePwm_t pwm_channel) {
    if ((pwm_channel < ePwm_First) || (pwm_channel >= ePwm_Last)) {
        return NULL;
    }

    return &g_static_oc_pwm_lut[pwm_channel];
}
