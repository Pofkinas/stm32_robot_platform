#ifndef CONFIG_LED_CONFIG_H_
#define CONFIG_LED_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"

#ifdef ENABLE_LED
#include "gpio_config.h"
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
#include "pwm_config.h"
#endif /* ENABLE_PWM_LED */

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define PULSE_TIMER_FREQUENCY 1

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eLed {
    eLed_First = 0,

    #ifdef ENABLE_LED
    eLed_OnboardLed = eLed_First,
    #endif /* ENABLE_LED */

    eLed_Last
} eLed_t;

typedef enum eLedPwm {
    eLedPwm_First = 0,
    eLedPwm_Last
} eLedPwm_t;

#ifdef ENABLE_LED
typedef struct sLedDesc {
    eGpio_t led_pin;
    bool is_inverted;
    osTimerAttr_t blink_timer_attributes;
    osMutexAttr_t blink_mutex_attributes;
} sLedDesc_t;
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
typedef struct sLedPwmDesc {
    ePwm_t pwm_device;
    osTimerAttr_t pulse_timer_attributes;
    osMutexAttr_t pulse_mutex_attributes;
} sLedPwmDesc_t;
#endif /* ENABLE_PWM_LED */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern const osThreadAttr_t g_led_thread_attributes;
extern const osMessageQueueAttr_t g_led_message_queue_attributes;

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#ifdef ENABLE_LED
bool LED_Config_IsCorrectLed (const eLed_t led);
const sLedDesc_t *LED_Config_GetLedDesc (const eLed_t led);
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
bool LED_Config_IsCorrectPwmLed (const eLedPwm_t led);
const sLedPwmDesc_t *LED_Config_GetPwmLedDesc (const eLedPwm_t led);
#endif /* ENABLE_PWM_LED */

#endif /* CONFIG_LED_CONFIG_H_ */
