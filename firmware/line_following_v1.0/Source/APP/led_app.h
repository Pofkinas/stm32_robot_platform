#ifndef SOURCE_APP_LED_APP_H_
#define SOURCE_APP_LED_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "led_api.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eLedTask {
    eLedTask_First = 0,
    eLedTask_Set = eLedTask_First,
    eLedTask_Reset,
    eLedTask_Toggle,
    eLedTask_Blink,
    eLedTask_Set_Brightness,
    eLedTask_Pulse,
    eLedTask_Last
} eLedTask_t;

typedef struct sLedCommandDesc {
    eLedTask_t task;
    void *data;
} sLedCommandDesc_t;

typedef struct sLedCommon {
    eLedPin_t led_pin;
} sLedCommon_t;

typedef struct sLedBlink {
    eLedPin_t led_pin;
    uint8_t blink_time;
    uint16_t blink_frequency;
} sLedBlink_t;

typedef struct sLedSetBrightness {
    eLedPwmPin_t led_pin;
    uint8_t duty_cycle;
} sLedSetBrightness_t;

typedef struct sLedPulse {
    eLedPwmPin_t led_pin;
    uint8_t pulse_time;
    uint16_t pulse_frequency;
} sLedPulse_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool LED_APP_Init (void);
bool LED_APP_Add_Task (sLedCommandDesc_t *task_to_message_queue);

#endif /* SOURCE_APP_LED_APP_H_ */
