/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "io_config.h"

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
const static sIoDesc_t g_static_io_desc_lut[eIo_Last] = {
    [eIo_StartStopButton] = {
        .gpio_pin = eGpio_StartButton,
        .active_state = eActiveState_Low,
        .triggered_flag = STARTSTOP_TRIGGERED_EVENT,
        .is_debounce_enable = true,
        .debounce_period = STARTSTOP_BUTTON_DEBOUNCE_PERIOD,
        .mutex_attributes = {.name = "StartStop_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .debounce_timer_attributes = {.name = "StartStop_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExti_StartButton
    },
    [eIo_Tcrt5000_Right] = {
        .gpio_pin = eGpio_Tcrt5000_Right,
        .active_state = eActiveState_Both,
        .triggered_flag = TCRT5000_RIGHT_TRIGGERED_EVENT,
        .is_debounce_enable = true,
        .debounce_period = TCRT5000_DEBOUNCE_PERIOD,
        .mutex_attributes = {.name = "Tcrt_Right_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .debounce_timer_attributes = {.name = "Tcrt_Right_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExti_Tcrt5000_Right
    },
    [eIo_Tcrt5000_Left] = {
        .gpio_pin = eGpio_Tcrt5000_Left,
        .active_state = eActiveState_Both,
        .triggered_flag = TCRT5000_LEFT_TRIGGERED_EVENT,
        .is_debounce_enable = true,
        .debounce_period = TCRT5000_DEBOUNCE_PERIOD,
        .mutex_attributes = {.name = "Tcrt_Left_Mutex", .attr_bits = osMutexRecursive | osMutexPrioInherit, .cb_mem = NULL, .cb_size = 0U},
        .debounce_timer_attributes = {.name = "Tcrt_Left_Debounce_Timer", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0},
        .is_exti = true,
        .exti_device = eExti_Tcrt5000_Left
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

const osThreadAttr_t g_io_thread_attributes = {
    .name = "IO_API",
    .stack_size = 128 * 8,
    .priority = (osPriority_t) osPriorityNormal
};

const osMessageQueueAttr_t g_io_message_queue_attributes = {
    .name = "IO_API",
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

bool IO_Config_IsCorrectIo (const eIo_t io_device) {
    return (io_device >= eIo_First) && (io_device < eIo_Last);
}

const sIoDesc_t *IO_Config_GetIoDesc (const eIo_t io_device) {
    if (!IO_Config_IsCorrectIo(io_device)) {
        return NULL;
    }

    return &g_static_io_desc_lut[io_device];
}
