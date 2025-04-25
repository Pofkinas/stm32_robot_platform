/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "button_api.h"
#include <stdio.h>
#include "cmsis_os2.h"
#include "exti_driver.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_TIMEOUT 0U
#define BUTTON_MESSAGE_CAPACITY 10
#define DEBOUNCE_PERIOD 30U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sButtonDesc {
    eButton_t button;
    eGpioPin_t gpio_pin;
    bool active_state;
    bool is_debounce_enable;
    osTimerAttr_t debouce_timer_attributes;
    bool is_exti;
    eExtiDriver_t exti_device;
} sButtonDesc_t;

typedef struct sButtonDynamic {
    osTimerId_t debouce_timer;
    bool is_tiggered;
    bool debouce_timer_state;
    bool button_state;
} sButtonDynamic_t;

typedef struct sButtonTimerArg {
    eButton_t button;
} sButtonTimerArg_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

const static osThreadAttr_t g_button_thread_attributes = {
    .name = "Button_Thread",
    .stack_size = 128 * 3,
    .priority = (osPriority_t) osPriorityNormal
};

const static osMessageQueueAttr_t g_button_message_queue_attributes = {
    .name = "Button_API_MessageQueue", 
    .attr_bits = 0, 
    .cb_mem = NULL, 
    .cb_size = 0, 
    .mq_mem = NULL, 
    .mq_size = 0
};

/* clang-format off */
const static sButtonDesc_t g_static_button_desc_lut[eButton_Last] = {
    [eButton_StartStop] = {
        .button = eButton_StartStop,
        .gpio_pin = eGpioPin_StartButton,
        .active_state = false,
        .is_debounce_enable = true,
        .debouce_timer_attributes = {.name = "Button_StartStop_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExtiDriver_StartButton
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_all_button_init = false;

static osThreadId_t g_button_thread_id = NULL;
static osMessageQueueId_t g_button_message_queue_id = NULL;

/* clang-format off */
static sButtonDynamic_t g_dynamic_button_lut[eButton_Last] = {
    [eButton_StartStop] = {
        .debouce_timer = NULL,
        .is_tiggered = false,
        .debouce_timer_state = false,
        .button_state = false
    }
};

static sButtonTimerArg_t g_static_button_timer_arg_lut[eButton_Last] = {
    [eButton_StartStop] = {
        .button = eButton_StartStop
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void Button_API_Thread (void *arg);
static void Button_API_DebounceTimerCallback (void *arg);
static void Button_API_ExtiTriggered (const eExtiDriver_t device);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Button_API_Thread (void *arg) {
    while(1) {
        for (eButton_t button = eButton_First; button < eButton_Last; button++) {
            if (g_static_button_desc_lut[button].is_exti) {
                if (osMessageQueueGet(g_button_message_queue_id, &button, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT) != osOK) {
                    continue;
                }

                if (g_static_button_desc_lut[button].is_debounce_enable) {
                    osTimerStart(g_dynamic_button_lut[button].debouce_timer, DEBOUNCE_PERIOD);
                }

                continue;
            }

            if (g_static_button_desc_lut[button].is_debounce_enable && g_dynamic_button_lut[button].debouce_timer_state) {
                continue;
            }

            if (!GPIO_Driver_ReadPin(g_static_button_desc_lut[button].gpio_pin, &g_dynamic_button_lut[button].button_state)) {
                continue;
            }
            
            if (g_dynamic_button_lut[button].button_state != g_static_button_desc_lut[button].active_state) {
                continue;
            }

            if (g_static_button_desc_lut[button].is_debounce_enable) {
                g_dynamic_button_lut[button].debouce_timer_state = true;

                osTimerStart(g_dynamic_button_lut[button].debouce_timer, DEBOUNCE_PERIOD);
            } else {
                g_dynamic_button_lut[button].is_tiggered = true;
            }
        }
    }

    osThreadYield();
}

static void Button_API_ExtiTriggered (const eExtiDriver_t device) {
    for (eButton_t button = eButton_First; button < eButton_Last; button++) {
        if (g_static_button_desc_lut[button].exti_device != device) {
            continue;
        }

        if (!g_static_button_desc_lut[button].is_debounce_enable) {
            g_dynamic_button_lut[button].is_tiggered = true;

            return;
        }
        
        Exti_Driver_Disable_IT(g_static_button_desc_lut[button].exti_device);
        osMessageQueuePut(g_button_message_queue_id, &g_static_button_desc_lut[button].button, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT);

        return;
    }
}

static void Button_API_DebounceTimerCallback (void *arg) {
    sButtonTimerArg_t *button_arg_lut = (sButtonTimerArg_t*) arg;

    if (g_static_button_desc_lut[button_arg_lut->button].is_exti) {
        Exti_Driver_Enable_IT(g_static_button_desc_lut[button_arg_lut->button].exti_device);
    }

    if (!GPIO_Driver_ReadPin(g_static_button_desc_lut[button_arg_lut->button].gpio_pin, &g_dynamic_button_lut[button_arg_lut->button].button_state)) {
        return;
    }

    if (g_dynamic_button_lut[button_arg_lut->button].button_state != g_static_button_desc_lut[button_arg_lut->button].active_state) {
        return;
    }

    g_dynamic_button_lut[button_arg_lut->button].is_tiggered = true;

    if (!g_static_button_desc_lut[button_arg_lut->button].is_exti) {
        g_dynamic_button_lut[button_arg_lut->button].debouce_timer_state = false;
    }

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Button_API_Init (void) {
    if (g_all_button_init) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    for (eButton_t button = eButton_First; button < eButton_Last; button++) {
        if (!Exti_Driver_InitDevice(g_static_button_desc_lut[button].exti_device, &Button_API_ExtiTriggered)) {
            return false;
        }

        if (g_static_button_desc_lut[button].is_debounce_enable) {
            g_dynamic_button_lut[button].debouce_timer = osTimerNew(Button_API_DebounceTimerCallback, osTimerOnce, &g_static_button_timer_arg_lut[button], &g_static_button_desc_lut[button].debouce_timer_attributes);
        }
    }

    if (g_button_thread_id == NULL) {
        g_button_thread_id = osThreadNew(Button_API_Thread, NULL, &g_button_thread_attributes);
    }

    if (g_button_message_queue_id == NULL) {
        g_button_message_queue_id = osMessageQueueNew(BUTTON_MESSAGE_CAPACITY, sizeof(eButton_t), &g_button_message_queue_attributes);
    }

    g_all_button_init = true;

    return g_all_button_init;
}

bool Button_API_IsTriggered (const eButton_t button) {
    if (!Button_API_IsCorrectButton(button)) {
        return false;
    }

    return g_dynamic_button_lut[button].is_tiggered;
}

bool Button_API_ClearState (const eButton_t button) {
    if (!Button_API_IsCorrectButton(button)) {
        return false;
    }

    g_dynamic_button_lut[button].is_tiggered = false;

    return true;
}

bool Button_API_IsCorrectButton (const eButton_t button) {
    return (button >= eButton_First) && (button < eButton_Last);
}
