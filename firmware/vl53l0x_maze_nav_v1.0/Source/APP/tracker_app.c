/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "tracker_app.h"
#include "cmsis_os2.h"
#include "heap_api.h"
#include "debug_api.h"
#include "button_api.h"
#include "tcrt5000_api.h"
#include "motor_api.h"
#include "led_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEBUG_TRACKER_APP

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_TIMEOUT 0U

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_TRACKER_APP
CREATE_MODULE_NAME (Tracker_APP)
#else
CREATE_MODULE_NAME_EMPTY
#endif

const static osThreadAttr_t g_tracker_thread_attributes = {
    .name = "Tracker_APP_Thread",
    .stack_size = 128 * 6,
    .priority = (osPriority_t) osPriorityNormal
};

const static osMessageQueueAttr_t g_tracker_message_queue_attributes = {
    .name = "Tracker_Command_MessageQueue", 
    .attr_bits = 0, 
    .cb_mem = NULL, 
    .cb_size = 0, 
    .mq_mem = NULL, 
    .mq_size = 0
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;

static eTrackerTask_t g_tracker_task = eTrackerTask_Collect;
static eTrackerTask_t g_previous_task = eTrackerTask_Stop;

static osThreadId_t g_tracker_thread_id = NULL;
static osMessageQueueId_t g_tracker_message_queue_id = NULL;

static sTcrt5000Data_t g_hw006_data[eTcrt5000_Last] = {0};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void Tracker_APP_Thread (void* arg);
static bool Tracker_APP_Process_Tcrt5000 (sTcrt5000Data_t data, eTrackerTask_t *task);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Tracker_APP_Thread (void* arg) {
    while(1) {
        osMessageQueueGet(g_tracker_message_queue_id, &g_tracker_task, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT);

        switch (g_tracker_task) {
            case eTrackerTask_Collect: {
                if (Button_API_IsTriggered(eButton_StartStop)) {
                    Button_API_ClearState(eButton_StartStop);
                    
                    if (g_previous_task == eTrackerTask_Stop) {
                        g_tracker_task = eTrackerTask_Start;

                        break;
                    } else {
                        g_tracker_task = eTrackerTask_Stop;

                        break;
                    }
                }

                if (Tcrt5000_API_GetData(g_hw006_data)) {
                    Tracker_APP_Process_Tcrt5000(*g_hw006_data, &g_tracker_task);

                    break;
                }
            } break;
            case eTrackerTask_Start: {
                TRACE_INFO("Current task: Start\n");

                if (!Tcrt5000_API_ReadPinState(eTcrt5000_Main, &g_hw006_data[eTcrt5000_Main].pin_state)) {
                    TRACE_ERR("Failed Tcrt5000 ReadPin\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!g_hw006_data[eTcrt5000_Main].pin_state) {
                    TRACE_WRN("No line detected at start\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!Motor_API_SetSpeed(STOP_SPEED, eMotorDirection_Forward)) {
                    TRACE_ERR("Failed Motor Set Speed\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!LED_API_TurnOn(eLedPin_OnboardLed)) {
                    TRACE_ERR("Failed Led Turn On\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!Tcrt5000_API_Enable()) {
                    TRACE_ERR("Failed Tcrt5000 Enable\n");

                    g_tracker_task = eTrackerTask_Stop;

                    break;
                }

                g_tracker_task = eTrackerTask_FallowLine;
            }
            case eTrackerTask_FallowLine: {
                TRACE_INFO("Current task: FallowLine\n");

                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Failed Motor Stop\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!Motor_API_SetSpeed(DEFAULT_MOTOR_SPEED, eMotorDirection_Forward)) {
                    TRACE_ERR("Failed Motor Set Speed\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                g_previous_task = eTrackerTask_FallowLine;
                g_tracker_task = eTrackerTask_Collect;
            } break;
            case eTrackerTask_SearchLine: {
                TRACE_INFO("Current task: SearchLine\n");

                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Failed Motor Stop\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!Motor_API_SetSpeed(MOTOR_SEARCH_SPEED, eMotorDirection_Left)) {
                    TRACE_ERR("Failed Motor Set Speed\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                // TODO: Make "Start Led Blinking"

                g_previous_task = eTrackerTask_SearchLine;
                g_tracker_task = eTrackerTask_Collect;
            } break;
            case eTrackerTask_Stop: {
                TRACE_INFO("Current task: Stop\n");

                if (!Tcrt5000_API_Disable()) {
                    TRACE_ERR("Failed Tcrt5000 Disable\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Failed Motor Stop\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (!LED_API_TurnOff(eLedPin_OnboardLed)) {
                    TRACE_ERR("Failed Led Turn On\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                g_previous_task = eTrackerTask_Stop;
                g_tracker_task = eTrackerTask_Collect;
            } break;
            default: {
            } break;
        }
    }

    osThreadYield();
}

bool Tracker_APP_Process_Tcrt5000 (sTcrt5000Data_t data, eTrackerTask_t *task) {
    if (task == NULL) {
        return false;
    }

    switch (data.device) {
        case eTcrt5000_Main: {
            if (!data.is_tiggered) {
                return false;
            }

            Tcrt5000_API_ClearTrigger(data.device);

            if (!data.pin_state && (g_previous_task != eTrackerTask_SearchLine)) {
                *task = eTrackerTask_SearchLine;

                return true;
            }
            
            if (data.pin_state && (g_previous_task != eTrackerTask_FallowLine)) {
                *task = eTrackerTask_FallowLine;

                return true;
            }
        } break;
        default: {
            return false;
        } break;
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Tracker_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (!Button_API_Init()) {
        return false;
    }

    if (!Tcrt5000_API_Init()) {
        return false;
    }
 
    if (!Motor_API_Init()) {
        return false;
    }

    if (!LED_API_Init()) {
        return false;
    }

    if (g_tracker_message_queue_id == NULL) {
        g_tracker_message_queue_id = osMessageQueueNew(TRACKER_COMMAND_MESSAGE_CAPACITY, sizeof(eTrackerTask_t), &g_tracker_message_queue_attributes);
    }

    if (g_tracker_thread_id == NULL) {
        g_tracker_thread_id = osThreadNew(Tracker_APP_Thread, NULL, &g_tracker_thread_attributes);
    }

    g_is_initialized = true;

    return g_is_initialized;
}

bool Tracker_APP_Add_Task (eTrackerTask_t task_to_message_queque) {
    if ((task_to_message_queque < eTrackerTask_First) || (task_to_message_queque >= eTrackerTask_Last)) {
        return false;
    }

    if (g_tracker_message_queue_id == NULL) {
        return false;
    }

    if (osMessageQueuePut(g_tracker_message_queue_id, &task_to_message_queque, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT) != osOK) {
        return false;
    }

    return true;
}
