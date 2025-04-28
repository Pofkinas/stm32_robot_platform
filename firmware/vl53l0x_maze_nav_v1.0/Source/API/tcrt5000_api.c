/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdio.h>
#include <tcrt5000_api.h>
#include "cmsis_os2.h"
#include "debug_api.h"
#include "exti_driver.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_TIMEOUT 0U
#define TCRT5000_MESSAGE_CAPACITY 5
#define TCRT5000_MUTEX_TIMEOUT 0U
#define DEBOUNCE_PERIOD 1U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sTcrt5000Desc {
    eExtiDriver_t exti_device;
    eGpioPin_t gpio_pin;
    bool active_state;
    osTimerAttr_t debouce_timer_attributes;
    osMutexAttr_t mutex_attributes;
} sTcrt5000Desc_t;

typedef struct sTcrt5000TimerArg {
    eTcrt5000_t device;
} sTcrt5000TimerArg_t;

typedef struct sTcrt5000Dynamic {
    bool debounce_pin_state;
    osTimerId_t debouce_timer;
    osMutexId_t mutex_id;
} sTcrt5000Dynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

CREATE_MODULE_NAME (TCRT5000_API)

const static osThreadAttr_t g_tcrt5000_thread_attributes = {
    .name = "Tcrt5000_Thread",
    .stack_size = 128 * 3,
    .priority = (osPriority_t) osPriorityNormal
};

const static osMessageQueueAttr_t g_tcrt5000_message_queue_attributes = {
    .name = "Tcrt5000_API_MessageQueue", 
    .attr_bits = 0, 
    .cb_mem = NULL, 
    .cb_size = 0, 
    .mq_mem = NULL, 
    .mq_size = 0
};

/* clang-format off */
const static sTcrt5000Desc_t g_static_exti_tcrt5000_lut[eTcrt5000_Last] = {
    [eTcrt5000_Main] = {
        .exti_device = eExtiDriver_Tcrt5000,
        .gpio_pin = eGpioPin_Tcrt5000,
        .active_state = true,
        .debouce_timer_attributes = {.name = "Tcrt5000_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .mutex_attributes = {.name = "Tcrt5000_Main_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U}
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_tcrt5000_init = false;
static bool g_is_all_enable = false;

static osThreadId_t g_tcrt5000_thread_id = NULL;
static osMessageQueueId_t g_tcrt5000_message_queue_id = NULL;

/* clang-format off */
static sTcrt5000Data_t g_tcrt5000_data[eTcrt5000_Last] = {
    [eTcrt5000_Main] = {
        .device = eTcrt5000_Main,
        .is_tiggered = false,
        .pin_state = false
    }
};

static sTcrt5000Dynamic_t g_tcrt5000_dynamic_lut[eTcrt5000_Last] = {
    [eTcrt5000_Main] = {
        .debounce_pin_state = false,
        .debouce_timer = NULL,
        .mutex_id = NULL
    }
};

static sTcrt5000TimerArg_t g_static_tcrt5000_timer_arg_lut[eTcrt5000_Last] = {
    [eTcrt5000_Main] = {
        .device = eTcrt5000_Main
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void Tcrt5000_API_Thread (void *arg);
static void Tcrt5000_API_DebounceTimerCallback (void *arg);
static void Tcrt5000_API_ExtiTriggered (const eExtiDriver_t device);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Tcrt5000_API_Thread (void *arg) {
    eTcrt5000_t device_from_msg_queue = eTcrt5000_Last;
    
    while (1) {
        if (osMessageQueueGet(g_tcrt5000_message_queue_id, &device_from_msg_queue, MESSAGE_QUEUE_PRIORITY, osWaitForever) == osOK) {
            if (osMutexAcquire(g_tcrt5000_dynamic_lut[device_from_msg_queue].mutex_id, TCRT5000_MUTEX_TIMEOUT) != osOK) {
                continue;
            }

            GPIO_Driver_ReadPin(g_static_exti_tcrt5000_lut[device_from_msg_queue].gpio_pin, &g_tcrt5000_dynamic_lut[device_from_msg_queue].debounce_pin_state);
            osTimerStart(g_tcrt5000_dynamic_lut[device_from_msg_queue].debouce_timer, DEBOUNCE_PERIOD);

            osMutexRelease(g_tcrt5000_dynamic_lut[device_from_msg_queue].mutex_id);
        }
    }
    
    osThreadYield();
}

static void Tcrt5000_API_ExtiTriggered (const eExtiDriver_t exti_device) {
    for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
        if (g_static_exti_tcrt5000_lut[device].exti_device != exti_device) {
            continue;
        }

        Exti_Driver_Disable_IT(g_static_exti_tcrt5000_lut[device].exti_device);
        osMessageQueuePut(g_tcrt5000_message_queue_id, &device, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT);
    }

    return;
}

static void Tcrt5000_API_DebounceTimerCallback (void *arg) {
    sTcrt5000TimerArg_t *Tcrt5000_arg_lut = (sTcrt5000TimerArg_t*) arg;

    bool current_pin_state = false;

    if (!GPIO_Driver_ReadPin(g_static_exti_tcrt5000_lut[Tcrt5000_arg_lut->device].gpio_pin, &current_pin_state)) {
        Exti_Driver_Enable_IT(g_static_exti_tcrt5000_lut[Tcrt5000_arg_lut->device].exti_device);
        TRACE_ERR("Failed to read pin state\n");
        
        return;
    }

    if (g_tcrt5000_dynamic_lut[Tcrt5000_arg_lut->device].debounce_pin_state != current_pin_state) {
        Exti_Driver_Enable_IT(g_static_exti_tcrt5000_lut[Tcrt5000_arg_lut->device].exti_device);
        TRACE_ERR("Pin state changed\n");

        return;
    }

    g_tcrt5000_data[Tcrt5000_arg_lut->device].is_tiggered = true;
    g_tcrt5000_data[Tcrt5000_arg_lut->device].pin_state = current_pin_state;

    Exti_Driver_Enable_IT(g_static_exti_tcrt5000_lut[Tcrt5000_arg_lut->device].exti_device);

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Tcrt5000_API_Init (void) {
    if (g_is_all_tcrt5000_init) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
        if (!Exti_Driver_InitDevice(g_static_exti_tcrt5000_lut[device].exti_device, Tcrt5000_API_ExtiTriggered)) {
            return false;
        }

        Exti_Driver_Disable_IT(g_static_exti_tcrt5000_lut[device].exti_device);

        if (g_tcrt5000_dynamic_lut[device].debouce_timer == NULL) {
            g_tcrt5000_dynamic_lut[device].debouce_timer = osTimerNew(Tcrt5000_API_DebounceTimerCallback, osTimerOnce, &g_static_tcrt5000_timer_arg_lut[device], &g_static_exti_tcrt5000_lut[device].debouce_timer_attributes);
        }

        if (g_tcrt5000_dynamic_lut[device].mutex_id == NULL) {
            g_tcrt5000_dynamic_lut[device].mutex_id = osMutexNew(&g_static_exti_tcrt5000_lut[device].mutex_attributes);
        }
    }

    if (g_tcrt5000_thread_id == NULL) {
        g_tcrt5000_thread_id = osThreadNew(Tcrt5000_API_Thread, NULL, &g_tcrt5000_thread_attributes);
    }

    if (g_tcrt5000_message_queue_id == NULL) {
        g_tcrt5000_message_queue_id = osMessageQueueNew(TCRT5000_MESSAGE_CAPACITY, sizeof(eTcrt5000_t), &g_tcrt5000_message_queue_attributes);
    }

    g_is_all_enable = false;
    g_is_all_tcrt5000_init = true;

    return true;
}

bool Tcrt5000_API_Enable (void) {
    if (!g_is_all_tcrt5000_init) {
        return false;
    }

    if (g_is_all_enable) {
        return true;
    }

    for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
        if (!Exti_Driver_Enable_IT(g_static_exti_tcrt5000_lut[device].exti_device)) {
            return false;
        }
    }

    TRACE_INFO("Tcrt5000 Sensors Enabled\n");

    g_is_all_enable = true;

    return true;
}

bool Tcrt5000_API_Disable (void) {
    if (!g_is_all_tcrt5000_init) {
        return false;
    }

    if (!g_is_all_enable) {
        return true;
    }

    for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
        if (!Exti_Driver_Disable_IT(g_static_exti_tcrt5000_lut[device].exti_device)) {
            return false;
        }
    }

    TRACE_INFO("Tcrt5000 Sensors Disabled\n");

    g_is_all_enable = false;

    return true;
}

bool Tcrt5000_API_GetData (sTcrt5000Data_t *data) {
    if (data == NULL) {
        return false;
    }

    if (!g_is_all_enable) {
        return false;
    }

    for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
        data[device] = g_tcrt5000_data[device];
    }

    return true;
}

bool Tcrt5000_API_ReadPinState (const eTcrt5000_t device, bool *pin_state) {
    if (!Tcrt5000_API_IsCorrectTcrt5000(device)) {
        return false;
    }

    if (pin_state == NULL) {
        return false;
    }

    return GPIO_Driver_ReadPin(g_static_exti_tcrt5000_lut[device].gpio_pin, pin_state);
}

bool Tcrt5000_API_ClearTrigger (const eTcrt5000_t device) {
    if (!Tcrt5000_API_IsCorrectTcrt5000(device)) {
        return false;
    }

    g_tcrt5000_data[device].is_tiggered = false;

    return true;
}

bool Tcrt5000_API_IsCorrectTcrt5000 (const eTcrt5000_t device) {
    return ((device >= eTcrt5000_First) && (device < eTcrt5000_Last));
}
