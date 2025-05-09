/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_api.h"
#include "cmsis_os2.h"
#include "gpio_driver.h"
#include "timer_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define BLINK_MUTEX_TIMEOUT 0U
#define PULSE_MUTEX_TIMEOUT 0U

#define MIN_DUTY_CYCLE 0
#define MAX_DUTY_CYCLE 65535

#define MIN_PULSE_FREQUENCY (MAX_DUTY_CYCLE / 1000)
#define PULSE_TIMER_FREQUENCY 1

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sLedControlDesc {
    bool is_inverted;
    osTimerAttr_t blink_timer_attributes;
    osMutexAttr_t blink_mutex_attributes;
} sLedControlDesc_t;

typedef struct sLedBlinkDesc {
    eLedPin_t led;
    osTimerId_t blink_timer;
    osMutexId_t blink_mutex;
    bool is_running;
    void (*timer_callback) (void *arg);
    uint16_t total_blinks;
    uint16_t blink_count;
} sLedBlinkDesc_t; 

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sLedControlDesc_t g_basic_led_control_static_lut[eLedPin_Last] = {
    [eLedPin_OnboardLed] = {
        .is_inverted = false,
        .blink_timer_attributes = {.name = "LED_API_Onboard_LED_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .blink_mutex_attributes = {.name = "LED_API_Onboard_LED_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void LED_API_Blink_Timer_Callback (void *arg);

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;
osTimerId_t g_blink_timer = NULL;
uint16_t g_blink_count = 0;

/* clang-format off */
static sLedBlinkDesc_t g_led_blink_lut[eLedPin_Last] = {
    [eLedPin_OnboardLed] = {
        .led = eLedPin_OnboardLed,
        .blink_timer = NULL,
        .blink_mutex = NULL,
        .is_running = false,
        .timer_callback = LED_API_Blink_Timer_Callback,
        .total_blinks = 0,
        .blink_count = 0,
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void LED_API_Blink_Timer_Callback (void *arg) {
    sLedBlinkDesc_t *led_blink_desc = (sLedBlinkDesc_t*) arg;

    if (!led_blink_desc->is_running) {
        if (osMutexAcquire(led_blink_desc->blink_mutex, BLINK_MUTEX_TIMEOUT) != osOK) {
            return;
        }
    }

    led_blink_desc->is_running = true;

    osMutexRelease(led_blink_desc->blink_mutex);

    LED_API_Toggle(led_blink_desc->led);

    led_blink_desc->blink_count++;

    if (led_blink_desc->blink_count >= led_blink_desc->total_blinks){
        osTimerStop(led_blink_desc->blink_timer);
        
        LED_API_TurnOff(led_blink_desc->led);
        
        led_blink_desc->is_running = false;
    }

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool LED_API_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    for (eLedPin_t led = eLedPin_First; led < eLedPin_Last; led++) {
        if (g_led_blink_lut[led].blink_timer == NULL) {
            g_led_blink_lut[led].blink_timer = osTimerNew(g_led_blink_lut[led].timer_callback, osTimerPeriodic, &g_led_blink_lut[led], &g_basic_led_control_static_lut[led].blink_timer_attributes);
        }

        if (g_led_blink_lut[led].blink_mutex == NULL) {
            g_led_blink_lut[led].blink_mutex = osMutexNew(&g_basic_led_control_static_lut[led].blink_mutex_attributes);
        }
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool LED_API_TurnOn (const eLedPin_t led_pin) {
    if (!g_is_initialized) {
        return false;
    }
    
    if (!LED_API_IsCorrectLed(led_pin)) {
        return false;
    }

    return GPIO_Driver_WritePin(led_pin, !g_basic_led_control_static_lut[led_pin].is_inverted);
}

bool LED_API_TurnOff (const eLedPin_t led_pin) {
    if (!g_is_initialized) {
        return false;
    }
    
    if (!LED_API_IsCorrectLed(led_pin)) {
        return false;
    }
    
    return GPIO_Driver_WritePin(led_pin, g_basic_led_control_static_lut[led_pin].is_inverted);
}

bool LED_API_Toggle (const eLedPin_t led_pin) {
    if (!g_is_initialized) {
        return false;
    }
    
    if (!LED_API_IsCorrectLed(led_pin)) {
        return false;
    }

    return GPIO_Driver_TogglePin(led_pin);
}

bool LED_API_Blink (const eLedPin_t led_pin, const uint8_t blink_time, const uint16_t blink_frequency) {
    if (!g_is_initialized) {
        return false;
    }

    if (!LED_API_IsCorrectLed(led_pin)) {
        return false;
    }

    if (!LED_API_IsCorrectBlinkTime(blink_time)) {
        return false;
    }

    if (!LED_API_IsCorrectBlinkFrequency(blink_frequency)) {
        return false;
    }

    if (g_led_blink_lut[led_pin].is_running) {
        return true;
    }

    if (osMutexAcquire(g_led_blink_lut[led_pin].blink_mutex, BLINK_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    g_led_blink_lut[led_pin].total_blinks = (blink_time * 1000 / blink_frequency) * 2;
    g_led_blink_lut[led_pin].blink_count = 0;

    osTimerStart(g_led_blink_lut[led_pin].blink_timer, (blink_frequency / 2));

    osMutexRelease(g_led_blink_lut[led_pin].blink_mutex);

    return true;
}

bool LED_API_IsCorrectLed (const eLedPin_t led_pin) {
    return (led_pin >= eLedPin_First) && (led_pin < eLedPin_Last);
}

bool LED_API_IsCorrectBlinkTime (const uint8_t blink_time) {
    return (blink_time <= MAX_BLINK_TIME) && (blink_time > 0);
}

bool LED_API_IsCorrectBlinkFrequency (const uint16_t blink_frequency) {
    return (blink_frequency <= MAX_BLINK_FREQUENCY) && (blink_frequency >= MIN_BLINK_FREQUENCY);
}
