/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "tracker_app.h"
#include "tracker_app.h"
#include "cmsis_os2.h"
#include "heap_api.h"
#include "debug_api.h"
#include "io_api.h"
#include "motor_api.h"
#include "led_api.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEBUG_TRACKER_APP

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_TIMEOUT 0U

#define BUTTON_FLAG_TIMEOUT 0U
#define TCRT5000_FLAG_TIMEOUT 1U

#define CHANGE_DIRECTION_DELAY 10

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eTcrt5000 {
    eTcrt5000_First = 0,
    eTcrt5000_Right = eTcrt5000_First,
    eTcrt5000_Left,
    eTcrt5000_Last
} eTcrt5000_t;

typedef struct sTcrt5000Data {
    eIo_t device;
    uint32_t event_flag;
    bool is_tiggered;
    bool pin_state;
} sTcrt5000Data_t;

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

const static osEventFlagsAttr_t g_start_button_event_attributes = {
    .name = "Start_Button_Event",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

const static osEventFlagsAttr_t g_tcrt5000_event_attributes = {
    .name = "Tcrt5000_Event",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_initialized = false;

static eTrackerTask_t g_tracker_task = eTrackerTask_Off;
static eMotorDirection_t g_course = eMotorDirection_First;
static eMotorDirection_t g_last_course = eMotorDirection_First;

static osThreadId_t g_tracker_thread_id = NULL;
static osMessageQueueId_t g_tracker_message_queue_id = NULL;

static osEventFlagsId_t g_start_button_event = NULL;
static osEventFlagsId_t g_tcrt5000_event = NULL;

/* clang-format off */
static sTcrt5000Data_t g_tcrt5000_data[eTcrt5000_Last] = {
    [eTcrt5000_Right] = {
        .device = eIo_Tcrt5000_Right,
        .event_flag = TCRT5000_RIGHT_TRIGGERED_EVENT,
        .is_tiggered = false,
        .pin_state = false
    },
    [eTcrt5000_Left] = {
        .device = eIo_Tcrt5000_Left,
        .event_flag = TCRT5000_LEFT_TRIGGERED_EVENT,
        .is_tiggered = false,
        .pin_state = false
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void Tracker_APP_Thread (void* arg);
static eMotorDirection_t Tracker_APP_Process_Tcrt5000 (void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void Tracker_APP_Thread (void* arg) {
    while(1) {
        osMessageQueueGet(g_tracker_message_queue_id, &g_tracker_task, MESSAGE_QUEUE_PRIORITY, MESSAGE_QUEUE_TIMEOUT);

        if (g_tracker_task != eTrackerTask_Init) {
            if (osEventFlagsWait(g_start_button_event, STARTSTOP_TRIGGERED_EVENT, osFlagsWaitAny, BUTTON_FLAG_TIMEOUT) == STARTSTOP_TRIGGERED_EVENT) {
                TRACE_INFO("Stop button\n");
                
                g_tracker_task = eTrackerTask_Stop;
            }
        }

        switch (g_tracker_task) {
            case eTrackerTask_Init: {
                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Failed to stop all motors\n");

                    break;
                }
                
                if (osEventFlagsWait(g_start_button_event, STARTSTOP_TRIGGERED_EVENT, osFlagsWaitAny, osWaitForever) != STARTSTOP_TRIGGERED_EVENT) {
                    TRACE_ERR("Failed to to receive start button flag\n");
                    
                    break;
                }

                g_tracker_task = eTrackerTask_Collect;
            }
            case eTrackerTask_Start: {
                TRACE_INFO("Current task: Start\n");

                if (!IO_API_ReadPinState(g_tcrt5000_data[eTcrt5000_First].device, &g_tcrt5000_data[eTcrt5000_First].pin_state)) {
                    TRACE_ERR("Failed Tcrt5000 ReadPin\n");

                    g_tracker_task = eTrackerTask_Init;

                    break;
                }

                // if (g_tcrt5000_data[eTcrt5000_First].pin_state) {
                //     TRACE_WRN("No line detected at start\n");

                //     g_tracker_task = eTrackerTask_Init;

                //     break;
                // }

                if (!LED_API_TurnOn(eLedPin_OnboardLed)) {
                    TRACE_ERR("Failed Led Turn On\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                 if (!Motor_API_SetSpeed(DEFAULT_MOTOR_SPEED, eMotorDirection_Forward)) {
                     TRACE_ERR("Failed Motor Set Speed\n");

                     g_tracker_task = eTrackerTask_Init;

                     break;
                 }

                g_last_course = eMotorDirection_Forward;
                g_tracker_task = eTrackerTask_Collect;
            }
            case eTrackerTask_Collect: {
                bool data_received = false;
                
                for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
                    if (g_tcrt5000_data[device].is_tiggered) {
                        data_received = true;

                        continue;
                    }

                    if (osEventFlagsWait(g_tcrt5000_event, g_tcrt5000_data[device].event_flag, osFlagsWaitAny, TCRT5000_FLAG_TIMEOUT) == g_tcrt5000_data[device].event_flag) {
                        if (!IO_API_ReadPinState(g_tcrt5000_data[device].device, &g_tcrt5000_data[device].pin_state)) {
                            TRACE_ERR("Failed Tcrt5000 ReadPin\n");

                            g_tracker_task = eTrackerTask_Init;

                            break;
                        }

                        g_tcrt5000_data[device].is_tiggered = true;
                    }
                }

                if (data_received) {
                    g_course = Tracker_APP_Process_Tcrt5000();
                    g_tracker_task = eTrackerTask_UpdateCourse;
                }
            } break;
            case eTrackerTask_UpdateCourse: {
                TRACE_INFO("Current task: Update course [%d]\n", g_course);

                if ((g_course < eMotorDirection_First) || (g_course >= eMotorDirection_Last)) {
                    TRACE_ERR("Invalid course direction\n");

                    g_tracker_task = eTrackerTask_Collect;

                    break;
                }

                if (((g_course == eMotorDirection_Forward) || (g_course == eMotorDirection_Reverse)) && ((g_last_course == eMotorDirection_Forward) || (g_last_course == eMotorDirection_Reverse))) {
                    if (!Motor_API_StopAllMotors()) {
                        TRACE_ERR("Failed Motor Stop\n");

                        g_tracker_task = eTrackerTask_Collect;

                        break;
                    }

                    osDelay(CHANGE_DIRECTION_DELAY);
                }

                 if (!Motor_API_SetSpeed(DEFAULT_MOTOR_SPEED, g_course)) {
                     TRACE_ERR("Failed Motor Set Speed\n");

                     g_tracker_task = eTrackerTask_Collect;

                     break;
                 }

                g_tracker_task = eTrackerTask_Collect;
            } break;
            case eTrackerTask_Stop: {
                TRACE_INFO("Current task: Stop\n");

                if (!Motor_API_StopAllMotors()) {
                    TRACE_ERR("Failed Motor Stop\n");

                    g_tracker_task = eTrackerTask_Init;

                    break;
                }

                if (!LED_API_TurnOff(eLedPin_OnboardLed)) {
                    TRACE_ERR("Failed Led Turn On\n");

                    g_tracker_task = eTrackerTask_Init;

                    break;
                }

                g_tracker_task = eTrackerTask_Init;
            } break;
            default: {
            } break;
        }
    }

    osThreadYield();
}

static eMotorDirection_t Tracker_APP_Process_Tcrt5000 (void) { 
    if ((g_tcrt5000_data[eTcrt5000_Right].is_tiggered && g_tcrt5000_data[eTcrt5000_Left].is_tiggered)) {
        g_tcrt5000_data[eTcrt5000_Right].is_tiggered = false;
        g_tcrt5000_data[eTcrt5000_Left].is_tiggered = false;
        
        if (g_tcrt5000_data[eTcrt5000_Right].pin_state && g_tcrt5000_data[eTcrt5000_Left].pin_state) {
            g_last_course = g_course;
            
            return eMotorDirection_Forward;
        } else {
            g_last_course = g_course;

            return eMotorDirection_Reverse;
        }
    }

    if (g_tcrt5000_data[eTcrt5000_Right].is_tiggered) {
        g_tcrt5000_data[eTcrt5000_Right].is_tiggered = false;

        if (g_tcrt5000_data[eTcrt5000_Right].pin_state) {
            return eMotorDirection_Forward;
        } else {
            return eMotorDirection_LeftSoft;
        }
    }

    if (g_tcrt5000_data[eTcrt5000_Left].is_tiggered) {
        g_tcrt5000_data[eTcrt5000_Left].is_tiggered = false;

        if (g_tcrt5000_data[eTcrt5000_Left].pin_state) {
            return eMotorDirection_Forward;
        } else {
            return eMotorDirection_RightSoft;
        }
    }

    return eMotorDirection_Last;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Tracker_APP_Init (void) {
    if (g_is_initialized) {
        return true;
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

    if (g_start_button_event == NULL) {
        g_start_button_event = osEventFlagsNew(&g_start_button_event_attributes);
    }

    if (g_tcrt5000_event == NULL) {
        g_tcrt5000_event = osEventFlagsNew(&g_tcrt5000_event_attributes);
    }

    if (!IO_API_Init(eIo_StartStopButton, g_start_button_event)) {
        TRACE_ERR("Failed to initialize StartStopButton\n");

        return false;
    }

    for (eTcrt5000_t device = eTcrt5000_First; device < eTcrt5000_Last; device++) {
        if (!IO_API_Init(g_tcrt5000_data[device].device, g_tcrt5000_event)) {
            TRACE_ERR("Failed to initialize Tcrt5000 %d\n", device);

            return false;
        }
    }

    if (!Motor_API_EnableAllMotors()) {
        TRACE_ERR("Failed to enable all motors\n");

        return false;
    }

    g_is_initialized = true;

    g_tracker_task = eTrackerTask_Init;

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
