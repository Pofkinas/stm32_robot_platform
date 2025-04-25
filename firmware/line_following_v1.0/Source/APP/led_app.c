/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "led_app.h"
#include <stddef.h>
#include "cmsis_os2.h"
#include "cli_app.h"
#include "debug_api.h"
#include "heap_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_TIMEOUT 0U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

CREATE_MODULE_NAME (LED_APP)

const static osThreadAttr_t g_led_thread_attributes = {
    .name = "LED_APP_Thread",
    .stack_size = 128 * 6,
    .priority = (osPriority_t) osPriorityNormal
};

const static osMessageQueueAttr_t g_led_message_queue_attributes = {
    .name = "Led_Command_MessageQueue", 
    .attr_bits = 0, 
    .cb_mem = NULL, 
    .cb_size = 0, 
    .mq_mem = NULL, 
    .mq_size = 0
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static sLedCommandDesc_t g_received_task = {.task = eLedTask_Last, .data = NULL};
static bool g_is_initialized = false;

static osThreadId_t g_led_thread_id = NULL;
static osMessageQueueId_t g_led_message_queue_id = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void LED_APP_Thread (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void LED_APP_Thread (void *arg) {
    while (1) {
        if (osMessageQueueGet(g_led_message_queue_id, &g_received_task, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT) != osOK) {
            continue;
        }

        if (g_received_task.data == NULL) {
            TRACE_ERR("No arguments\n");
        }
        switch (g_received_task.task) {
            case eLedTask_Set: {
                sLedCommon_t *arguments = (sLedCommon_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }
                
                if (!LED_API_IsCorrectLed(arguments->led_pin)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }
                
                if (!LED_API_TurnOn(arguments->led_pin)) {
                    TRACE_ERR("LED Turn On Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Set\n", arguments->led_pin);

                Heap_API_Free(arguments);    
            } break;
            case eLedTask_Reset: {
                sLedCommon_t *arguments = (sLedCommon_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectLed(arguments->led_pin)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_TurnOff(arguments->led_pin)) {
                    TRACE_ERR("LED Turn Off Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Reset\n", arguments->led_pin);

                Heap_API_Free(arguments);
            } break;
            case eLedTask_Toggle: {
                sLedCommon_t *arguments = (sLedCommon_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectLed(arguments->led_pin)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_Toggle(arguments->led_pin)) {
                    TRACE_ERR("LED Toggle Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Toggle\n", arguments->led_pin);

                Heap_API_Free(arguments);
            } break;
            case eLedTask_Blink: {
                sLedBlink_t *arguments = (sLedBlink_t*) g_received_task.data;

                if (arguments == NULL){
                    TRACE_ERR("No arguments\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectLed(arguments->led_pin)) {
                    TRACE_ERR("Invalid Led\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectBlinkTime(arguments->blink_time)) {
                    TRACE_ERR("Invalid blink time\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_IsCorrectBlinkFrequency(arguments->blink_frequency)) {
                    TRACE_ERR("Invalid blink frequency\n");

                    Heap_API_Free(arguments);

                    break;
                }

                if (!LED_API_Blink(arguments->led_pin, arguments->blink_time, arguments->blink_frequency)) {
                    TRACE_ERR("LED Blink Failed\n");

                    Heap_API_Free(arguments);

                    break;
                }

                TRACE_INFO("Led %d Blink %d s, @ %d Hz\n", arguments->led_pin, arguments->blink_time, arguments->blink_frequency);

                Heap_API_Free(arguments);
            } break;
            default: {
                TRACE_ERR("Task not found\n");
            } break;
        }
    }

    osThreadYield();
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool LED_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (!LED_API_Init()) {
        return false;
    }
    
    if (g_led_message_queue_id == NULL) {
        g_led_message_queue_id = osMessageQueueNew(CLI_COMMAND_MESSAGE_CAPACITY, sizeof(sLedCommandDesc_t), &g_led_message_queue_attributes);
    }

    if (g_led_thread_id == NULL) {
        g_led_thread_id = osThreadNew(LED_APP_Thread, NULL, &g_led_thread_attributes);
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool LED_APP_Add_Task (sLedCommandDesc_t *task_to_message_queue) {
    if (task_to_message_queue == NULL) {
        return false;
    }

    if (g_led_message_queue_id == NULL){
        return false;
    }

    if (osMessageQueuePut(g_led_message_queue_id, task_to_message_queue, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT) != osOK) {
        return false;
    }

    return true;
}
