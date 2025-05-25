/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <io_api.h>
#include <stdio.h>
#include "debug_api.h"
#include "exti_driver.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

//#define DEBUG_IO_API

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_TIMEOUT 5U
#define MUTEX_TIMEOUT 0U
#define IO_MESSAGE_CAPACITY 10

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eIoDeviceState {
    eIoDeviceState_First = 0,
    eIoDeviceState_Default = eIoDeviceState_First,
    eIoDeviceState_Init,
    eIoDeviceState_Debounce,
    eIoDeviceState_Last
} eIoDeviceState_t;

typedef enum eActiceState {
    eActiveState_First = 0,
    eActiveState_Low = eActiveState_First,
    eActiveState_High,
    eActiveState_Both,
    eActiveState_Last
} eActiveState_t;

typedef struct sIoDesc {
    eGpioPin_t gpio_pin;
    eActiveState_t active_state;
    uint32_t tiggered_flag;
    bool is_debounce_enable;
    size_t debounce_period;
    osMutexAttr_t mutex_attributes;
    osTimerAttr_t debouce_timer_attributes;
    bool is_exti;
    eExtiDriver_t exti_device;
} sIoDesc_t;

typedef struct sIoDynamic {
    eIo_t device;
    eIoDeviceState_t device_state;
    osMutexId_t mutex;
    osEventFlagsId_t callback_flag;
    osTimerId_t debouce_timer;
    bool io_value;
} sIoDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_IO_API
CREATE_MODULE_NAME (IO_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif

const static osThreadAttr_t g_io_thread_attributes = {
    .name = "IO_Thread",
    .stack_size = 128 * 4,
    .priority = (osPriority_t) osPriorityNormal
};

const static osMessageQueueAttr_t g_io_message_queue_attributes = {
    .name = "IO_API_MessageQueue", 
    .attr_bits = 0, 
    .cb_mem = NULL, 
    .cb_size = 0, 
    .mq_mem = NULL, 
    .mq_size = 0
};

/* clang-format off */
const static sIoDesc_t g_static_io_desc_lut[eIo_Last] = {
    [eIo_StartStopButton] = {
        .gpio_pin = eGpioPin_StartButton,
        .active_state = eActiveState_Low,
        .tiggered_flag = STARTSTOP_TRIGGERED_EVENT,
        .is_debounce_enable = true,
        .debounce_period = STARTSTOP_BUTTON_DEBOUNCE_PERIOD,
        .mutex_attributes = {.name = "StartStop_Button_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .debouce_timer_attributes = {.name = "StartStop_Button_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExtiDriver_StartButton
    },
    [eIo_Tcrt5000_Right] = {
        .gpio_pin = eGpioPin_Tcrt5000_Right,
        .active_state = eActiveState_Both,
        .tiggered_flag = TCRT5000_RIGHT_TRIGGERED_EVENT,
        .is_debounce_enable = true,
        .debounce_period = TCRT5000_DEBOUNCE_PERIOD,
        .mutex_attributes = {.name = "Tcrt5000_Right_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .debouce_timer_attributes = {.name = "Tcrt5000_Right_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExtiDriver_Tcrt5000_Right
    },
    [eIo_Tcrt5000_Left] = {
        .gpio_pin = eGpioPin_Tcrt5000_Left,
        .active_state = eActiveState_Both,
        .tiggered_flag = TCRT5000_LEFT_TRIGGERED_EVENT,
        .is_debounce_enable = true,
        .debounce_period = TCRT5000_DEBOUNCE_PERIOD,
        .mutex_attributes = {.name = "Tcrt5000_Left_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .debouce_timer_attributes = {.name = "Tcrt5000_Left_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExtiDriver_Tcrt5000_Left
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static osThreadId_t g_io_thread_id = NULL;
static osMessageQueueId_t g_io_message_queue_id = NULL;
static bool g_has_pooled_io = false;

/* clang-format off */
static sIoDynamic_t g_dynamic_io_lut[eIo_Last] = {
    [eIo_StartStopButton] = {
        .device_state = eIoDeviceState_Default,
        .mutex = NULL,
        .callback_flag = NULL,
        .debouce_timer = NULL,
        .io_value = false
    }, 
    [eIo_Tcrt5000_Right] = {
        .device_state = eIoDeviceState_Default,
        .mutex = NULL,
        .callback_flag = NULL,
        .debouce_timer = NULL,
        .io_value = false
    },
    [eIo_Tcrt5000_Left] = {
        .device_state = eIoDeviceState_Default,
        .mutex = NULL,
        .callback_flag = NULL,
        .debouce_timer = NULL,
        .io_value = false
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void IO_API_Thread (void *arg);
static void IO_API_DebounceTimerCallback (void *context);
static void IO_API_ExtiTriggered (void *context);
static void IO_API_StartDebounceTimer (const eIo_t device);
static bool IO_API_IsGpioStateCorrect (const eIo_t device);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void IO_API_Thread (void *arg) {
    eIo_t device;
    
    while(1) {
        if (osMessageQueueGet(g_io_message_queue_id, &device, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT) == osOK) {    
            if (g_static_io_desc_lut[device].is_debounce_enable) {
                IO_API_StartDebounceTimer(device);

                TRACE_INFO("IO [%d] Debounce triggered\n", device);
            }
        }

        if (!g_has_pooled_io) {
            continue;
        }

        for (device = eIo_First; device < eIo_Last; device++) {
            if (g_static_io_desc_lut[device].is_debounce_enable && (g_dynamic_io_lut[device].device_state == eIoDeviceState_Debounce)) {
                continue;
            }

            if (!IO_API_IsGpioStateCorrect(device)) {
                TRACE_WRN("GPIO state is incorrect [%d]\n", device);
                
                continue;
            }

            if (g_static_io_desc_lut[device].is_debounce_enable) {
                IO_API_StartDebounceTimer(device);
            } else {
                TRACE_INFO("IO [%d] triggered\n", device);

                osEventFlagsSet(g_dynamic_io_lut[device].callback_flag, g_static_io_desc_lut[device].tiggered_flag);
            }
        }

        osThreadYield();
    }
}

static void IO_API_ExtiTriggered (void *context) {
    sIoDynamic_t *device = (sIoDynamic_t*) context;

    if (!g_static_io_desc_lut[device->device].is_debounce_enable) {
        osEventFlagsSet(device->callback_flag, g_static_io_desc_lut[device->device].tiggered_flag);

        return;
    }

    Exti_Driver_Disable_IT(g_static_io_desc_lut[device->device].exti_device);
    osMessageQueuePut(g_io_message_queue_id, &device->device, MESSAGE_QUEUE_PRIORITY, 0);

    return;
}

static void IO_API_DebounceTimerCallback (void *context) {
    sIoDynamic_t *debounce_io = (sIoDynamic_t*) context;
    bool debounce_status = true;

    if (debounce_io->device_state != eIoDeviceState_Debounce) {
        TRACE_WRN("Debounce callback exited early, state [%d]\n", debounce_io->device_state);
        
        return;
    }

    if (!IO_API_IsGpioStateCorrect(debounce_io->device)) {
        TRACE_WRN("GPIO state is incorrect [%d]\n", debounce_io->device);
        
        debounce_status = false;
    }

    if (osMutexAcquire(debounce_io->mutex, MUTEX_TIMEOUT) != osOK) {
        debounce_status = false;
    }

    if (g_static_io_desc_lut[debounce_io->device].is_exti) {
        if (!Exti_Driver_ClearFlag(g_static_io_desc_lut[debounce_io->device].exti_device)) {
            debounce_status = false;
        }

        Exti_Driver_Enable_IT(g_static_io_desc_lut[debounce_io->device].exti_device);
    }

    debounce_io->device_state = eIoDeviceState_Init;

    if (!debounce_status) {  
        osMutexRelease(debounce_io->mutex);

        TRACE_WRN("Button [%d] debounce failed\n", debounce_io->device);

        return;
    }

    TRACE_INFO("Button [%d] triggered\n", debounce_io->device);

    osEventFlagsSet(debounce_io->callback_flag, g_static_io_desc_lut[debounce_io->device].tiggered_flag);

    osMutexRelease(debounce_io->mutex);

    return;
}

static void IO_API_StartDebounceTimer (const eIo_t device) {
    if (!IO_API_IsCorrectDevice(device)) {
        return;
    }

    if (g_dynamic_io_lut[device].device_state != eIoDeviceState_Init) {
        return;
    }

    if (osMutexAcquire(g_dynamic_io_lut[device].mutex, MUTEX_TIMEOUT) != osOK) {
        return;
    }

    g_dynamic_io_lut[device].device_state = eIoDeviceState_Debounce;

    osMutexRelease(g_dynamic_io_lut[device].mutex);

    osTimerStart(g_dynamic_io_lut[device].debouce_timer, g_static_io_desc_lut[device].debounce_period);

    return;
}

static bool IO_API_IsGpioStateCorrect (const eIo_t device) {
    if (!IO_API_IsCorrectDevice(device)) {
        return false;
    }

    if (g_dynamic_io_lut[device].device_state == eIoDeviceState_Default) {
        return false;
    }

    if (!GPIO_Driver_ReadPin(g_static_io_desc_lut[device].gpio_pin, &g_dynamic_io_lut[device].io_value)) {
        return false;
    }

    switch (g_static_io_desc_lut[device].active_state) {
        case eActiveState_Low: {
            if (g_dynamic_io_lut[device].io_value) {
                return false;
            }
        } break;
        case eActiveState_High: {
            if (!g_dynamic_io_lut[device].io_value) {
                return false;
            }
        } break;
        case eActiveState_Both: {
        } break;
        default: {
            return false;
        }
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool IO_API_Init (eIo_t device, osEventFlagsId_t event_flags_id) {
    if (!IO_API_IsCorrectDevice(device)) {
        return false;
    }

    if (g_dynamic_io_lut[device].device_state != eIoDeviceState_Default) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    if (!Exti_Driver_InitDevice(g_static_io_desc_lut[device].exti_device, &IO_API_ExtiTriggered, &g_dynamic_io_lut[device])) {
        return false;
    }

    if (g_io_thread_id == NULL) {
        g_io_thread_id = osThreadNew(IO_API_Thread, NULL, &g_io_thread_attributes);
    }

    if (g_io_message_queue_id == NULL) {
        g_io_message_queue_id = osMessageQueueNew(IO_MESSAGE_CAPACITY, sizeof(eIo_t), &g_io_message_queue_attributes);
    }

    if (g_static_io_desc_lut[device].is_debounce_enable) {
        g_dynamic_io_lut[device].debouce_timer = osTimerNew(IO_API_DebounceTimerCallback, osTimerOnce, &g_dynamic_io_lut[device], &g_static_io_desc_lut[device].debouce_timer_attributes);
    }

    if (g_dynamic_io_lut[device].mutex == NULL) {
        g_dynamic_io_lut[device].mutex = osMutexNew(&g_static_io_desc_lut[device].mutex_attributes);
    }

    g_dynamic_io_lut[device].callback_flag = event_flags_id;

    if (!g_static_io_desc_lut[device].is_exti) {
        g_has_pooled_io = true;
    }

    if (!Exti_Driver_Enable_IT(g_static_io_desc_lut[device].exti_device)) {
        return false;
    }

    g_dynamic_io_lut[device].device_state = eIoDeviceState_Init;
    g_dynamic_io_lut[device].device = device;

    return true;
}

bool IO_API_IsCorrectDevice (const eIo_t button) {
    return (button >= eIo_First) && (button < eIo_Last);
}

bool IO_API_ReadPinState (const eIo_t device, bool *pin_state) {
    if (!IO_API_IsCorrectDevice(device)) {
        return false;
    }

    if (g_dynamic_io_lut[device].device_state == eIoDeviceState_Default) {
        return false;
    }

    return (GPIO_Driver_ReadPin(g_static_io_desc_lut[device].gpio_pin, pin_state));
}
