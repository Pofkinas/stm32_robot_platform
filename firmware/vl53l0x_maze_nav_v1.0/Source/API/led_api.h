#ifndef __SOMETHING__H__
#define __SOMETHING__H__

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define MAX_BLINK_TIME 59
#define MIN_BLINK_FREQUENCY 2
#define MAX_BLINK_FREQUENCY 100
#define MAX_PULSING_TIME 59

#define MAX_PULSE_FREQUENCY 500

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eLedPin {
    eLedPin_First = 0,
    eLedPin_OnboardLed = eLedPin_First,
    eLedPin_Last
} eLedPin_t;

typedef enum eLedPwmPin {
    eLedPwmPin_First = 0,
    // eLedPwmPin_Test = eLedPwmPin_First,
    eLedPwmPin_Last
} eLedPwmPin_t;
/* clang-format off */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool LED_API_Init (void);
bool LED_API_TurnOn (const eLedPin_t led_pin);
bool LED_API_TurnOff (const eLedPin_t led_pin);
bool LED_API_Toggle (const eLedPin_t led_pin);
bool LED_API_Blink (const eLedPin_t led_pin, const uint8_t blink_time, const uint16_t blink_frequency);
bool LED_API_Set_Brightness (const eLedPwmPin_t led_pin, const uint8_t brightness) ;
bool LED_API_Pulse (const eLedPwmPin_t led_pin, const uint8_t pulsing_time, const uint16_t pulse_frequency);
bool LED_API_IsCorrectLed (const eLedPin_t led_pin);
bool LED_API_IsCorrectPwmLed (const eLedPwmPin_t led_pin);
bool LED_API_IsCorrectBlinkTime (const uint8_t blink_time);
bool LED_API_IsCorrectBlinkFrequency (const uint16_t blink_frequency);
bool LED_API_IsCorrectDutyCycle (const uint8_t duty_cycle);
bool LED_API_IsCorrectPulseTime (const uint8_t pulse_time);
bool LED_API_IsCorrectPulseFrequency (const uint16_t pulse_frequency);

#endif /* __SOMETHING__H__ */
