/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_config.h"

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
#ifdef ENABLE_LED
const static sLedDesc_t g_led_static_lut[eLed_Last] = {
    [eLed_OnboardLed] = {
        .led_pin = eGpio_OnboardLed,
        .is_inverted = false,
        .blink_timer_attributes = {.name = "Onboard_LED_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .blink_mutex_attributes = {.name = "Onboard_LED_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
    }
};
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
const static sLedPwmDesc_t g_pwm_led_static_lut[eLedPwm_Last] = {
};
#endif /* ENABLE_PWM_LED */
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

const osThreadAttr_t g_led_thread_attributes = {
    .name = "LED_APP",
    .stack_size = 128 * 6,
    .priority = (osPriority_t) osPriorityNormal
};

const osMessageQueueAttr_t g_led_message_queue_attributes = {
    .name = "LED_APP",
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

#ifdef ENABLE_LED
bool LED_Config_IsCorrectLed (const eLed_t led) {
    return (led >= eLed_First) && (led < eLed_Last);
}

const sLedDesc_t *LED_Config_GetLedDesc (const eLed_t led) {
    if (!LED_Config_IsCorrectLed(led)) {
        return NULL;
    }

    return &g_led_static_lut[led];
}
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
bool LED_Config_IsCorrectPwmLed (const eLedPwm_t led) {
    return (led >= eLedPwm_First) && (led < eLedPwm_Last);
}

const sLedPwmDesc_t *LED_Config_GetPwmLedDesc (const eLedPwm_t led) {
    if (!LED_Config_IsCorrectPwmLed(led)) {
        return NULL;
    }

    return &g_pwm_led_static_lut[led];
}
#endif /* ENABLE_PWM_LED */
