/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "mission_app.h"

#include <stddef.h>
#include "cmsis_os2.h"
#include "action_drive_distance.h"
#include "action_measure.h"
#include "action_wall_follow.h"
#include "esp_comm_app.h"
#include "led_api.h"
#include "heap_api.h"
#include "debug_api.h"

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sActionDesc {
    bool (*action_run)(const void *data, void *instance, eActionState_t *state);
    bool (*action_service)(eActionState_t *state);
    void (*action_stop)(void);
    eErrorCode_t (*action_parse)(sMessage_t *arguments, void **data, sMessage_t *response);
} sActionDesc_t;

typedef struct sActionSequence {
    eAction_t action;
    const sActionDesc_t *fp;
    const void *data;
    eActionState_t state;
    struct sActionSequence *next;
} sActionSequence_t;

typedef struct sTaskSequence {
    sActionSequence_t *action_head;
    sActionSequence_t *action_tail;
    struct sTaskSequence *next;
} sTaskSequence_t;

struct sMission {
    uint32_t id;
    sTaskSequence_t *task_head;
    sTaskSequence_t *task_tail;
    sTaskSequence_t *current_task;
    struct sMission *next;
    eMissionState_t state;
};

typedef struct sMissionSequence {
    sMission_t *mission_head;
    sMission_t *mission_tail;
    sMission_t *current;
} sMissionSequence_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 
#ifdef DEBUG_MISSION_APP
CREATE_MODULE_NAME (MISSION_APP)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_MISSION_APP */

static const osThreadAttr_t g_mission_thread_attributes = {
    .name = "Mission_APP",
    .stack_size = MISSION_APP_THREAD_STACK_SIZE,
    .priority = MISSION_APP_THREAD_PRIORITY
};

static const osMutexAttr_t g_mission_mutex_attributes = {
    .name = "Mission_APP",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = NULL,
    .cb_size = 0
};

static const osMessageQueueAttr_t g_mission_message_queue_attributes = {
    .name = "Mission_APP",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0,
    .mq_mem = NULL,
    .mq_size = 0
};

/* clang-format off */
static const sActionDesc_t g_static_action_desc_lut[eAction_Last] = {
    [eAction_DriveDistance] = {
        .action_run = Action_DriveDistance_Run,
        .action_service = Action_DriveDistance_Service,
        .action_stop = Action_DriveDistance_Stop,
        .action_parse = Action_DriveDistance_Parse
    },
    [eAction_Measure] = {
        .action_run = Action_Measure_Run,
        .action_service = Action_Measure_Service,
        .action_stop = Action_Measure_Stop,
        .action_parse = Action_Measure_Parse
    },
    [eAction_WallFollow] = {
        .action_run = Action_WallFollow_Run,
        .action_service = Action_WallFollow_Service,
        .action_stop = Action_WallFollow_Stop,
        .action_parse = Action_WallFollow_Parse
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
static bool g_is_initialized = false;

static uint32_t g_mission_id_counter = 0;
static sMissionSequence_t g_mission_sequence = {0};

static osThreadId_t g_mission_thread = NULL;
static osMutexId_t g_mission_mutex = NULL;
static osMessageQueueId_t g_mission_message_queue = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void Mission_APP_Thread (void *arg);
static bool Mission_APP_QueueMission (sMissionSequence_t *queue, sMission_t *mission);
static bool Mission_APP_DeleteMission (sMissionSequence_t *queue, sMission_t *mission);
static void Mission_APP_StopAction (const sActionSequence_t *action);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void Mission_APP_Thread (void *arg) {
    sMission_t *received_mission = NULL;
    sMission_t *current_mission = g_mission_sequence.current;
    sTaskSequence_t *current_task;

    uint32_t tick = osKernelGetTickCount();

    while (1) {
        if (osMessageQueueGet(g_mission_message_queue, &received_mission, MISSION_APP_MESSAGE_QUEUE_PRIORITY, (current_mission == NULL) ? osWaitForever : MISSION_APP_MESSAGE_QUEUE_TIMEOUT) == osOK) {
            if (received_mission == NULL) {
                TRACE_ERR("Thread: Received NULL\n");
                
                continue;
            }

            if (!Mission_APP_QueueMission(&g_mission_sequence, received_mission)) {
                TRACE_ERR("Thread: Failed to queue received mission\n");
                
                continue;
            }
        }

        if (osMutexAcquire(g_mission_mutex, MISSION_APP_MUTEX_TIMEOUT) != osOK) {
            TRACE_WRN("Thread: Failed to acquire mission mutex\n");
            
            continue;
        }

        current_mission = g_mission_sequence.current;

        osMutexRelease(g_mission_mutex);

        if (current_mission == NULL) {
            continue;
        }

        current_task = current_mission->current_task;

        switch (current_mission->state) {
            case eMissionState_Pending: {
                if (current_task == NULL) {
                    current_mission->state = eMissionState_Error;
                    
                    TRACE_ERR("Thread: No current task for mission [%d]\n", current_mission->id);
                    
                    continue;
                }

                bool is_success = true;
                sActionSequence_t *sequence = current_task->action_head;

                while (sequence != NULL) {
                    bool result = sequence->fp->action_run(sequence->data, current_mission, &sequence->state);

                    if (!result || (sequence->state != eActionState_Running)) {
                        is_success = false;
                        current_mission->state = eMissionState_Error;

                        TRACE_ERR("Thread: Failed to start task for mission [%d]\n", current_mission->id);
                        
                        break;
                    }

                    sequence = sequence->next;
                }

                if (!is_success) {
                    break;
                }

                LED_API_TurnOn(eLed_OnboardLed);
                current_mission->state = eMissionState_Running;
            }
            case eMissionState_Running: {
                if (current_task == NULL) {
                    current_mission->state = eMissionState_Error;
                    
                    TRACE_ERR("Thread: No current task for mission [%d]\n", current_mission->id);
                    
                    continue;
                }

                sActionSequence_t *sequence = current_task->action_head;
                bool is_all_completed = true;

                while (sequence != NULL) {
                    bool result = sequence->fp->action_service(&sequence->state);

                    if (sequence->state == eActionState_Pause) {
                        sequence = sequence->next;
                        
                        continue;
                    }

                    if (!result) {
                        current_mission->state = eMissionState_Error;
                        is_all_completed = false;
                        
                        TRACE_ERR("Thread: Failed to service task for mission [%d]\n", current_mission->id);
                        
                        break;
                    }

                    if ((sequence->state == eActionState_Error) || (sequence->state == eActionState_Init)) {
                        current_mission->state = eMissionState_Error;
                        is_all_completed = false;
                        
                        break;
                    }

                    if (sequence->state == eActionState_Completed) {
                        Mission_APP_StopAction(sequence);
                    } else {
                        is_all_completed = false;
                    }

                    sequence = sequence->next;
                }

                if (!is_all_completed) {
                    break;
                }

                sequence = current_task->action_head;
                
                while (sequence != NULL) {
                    sequence->fp->action_stop();
                    sequence = sequence->next;
                }

                current_mission->current_task = current_task->next;

                if (current_mission->current_task != NULL) {
                    break;
                }

                current_mission->state = eMissionState_Completed;
            } break;
            case eMissionState_Completed: {
                TRACE_INFO("Thread: Mission [%d] completed\n", current_mission->id);

                if (!Mission_APP_DeleteMission(&g_mission_sequence, current_mission)) {
                    TRACE_ERR("Thread: Failed to delete completed mission [%d]\n", current_mission->id);

                    osThreadTerminate(g_mission_thread);
                }
                
                if (g_mission_sequence.current != NULL) {
                    break;
                }

                LED_API_TurnOff(eLed_OnboardLed);
            } break;
            case eMissionState_Canceled: {
                TRACE_WRN("Thread: Mission [%d] was canceled\n", current_mission->id);

                if (!Mission_APP_DeleteMission(&g_mission_sequence, current_mission)) {
                    TRACE_ERR("Thread: Failed to delete canceled mission [%d]\n", current_mission->id);

                    osThreadTerminate(g_mission_thread);
                }
                
                if (g_mission_sequence.current != NULL) {
                    break;
                }

                LED_API_TurnOff(eLed_OnboardLed);
            } break;
            default: {
                TRACE_ERR("Thread: Mission [%d] in bad state [%d]\n", current_mission->id, current_mission->state);

                if (!Mission_APP_DeleteMission(&g_mission_sequence, current_mission)) {
                    TRACE_ERR("Thread: Failed to delete canceled mission [%d]\n", current_mission->id);

                    osThreadTerminate(g_mission_thread);
                }

                if (g_mission_sequence.current != NULL) {
                    break;
                }

                LED_API_TurnOff(eLed_OnboardLed);
            }
        }

        tick += MISSION_APP_UPDATE_RATE;

        uint32_t now = osKernelGetTickCount();
        if ((int32_t)(now - tick) > 0) {
            tick = now + MISSION_APP_UPDATE_RATE;
        }

        osDelayUntil(tick);
    }
}

static bool Mission_APP_QueueMission (sMissionSequence_t *queue, sMission_t *mission) {
    if (queue == NULL) {
        return false;
    }

    if (mission == NULL) {
        return false;
    }

    if (queue->mission_head == NULL) {
        queue->mission_head = mission;
        queue->current = mission;
    } else {
        if (queue->mission_tail == NULL) {
            return false;    
        }

        queue->mission_tail->next = mission;
    }

    queue->mission_tail = mission;
    mission->state = eMissionState_Pending;

    return true;
}

static bool Mission_APP_DeleteMission (sMissionSequence_t *queue, sMission_t *mission) {
    if (queue == NULL) {
        return false;
    }
    
    if (mission == NULL) {
        return false;
    }

    sTaskSequence_t *current_task = mission->task_head;
    sTaskSequence_t *next_task;

    while (current_task != NULL) {
        next_task = current_task->next;
        
        sActionSequence_t *current_action = current_task->action_head;
        sActionSequence_t *next_action;

        while (current_action != NULL) {
            next_action = current_action->next;

            Mission_APP_StopAction(current_action);

            if (current_action->data != NULL) {
                if (!Heap_API_Free((void *)current_action->data)) {
                    return false;
                }
            }

            if (!Heap_API_Free(current_action)) {
                return false;
            }

            current_action = next_action;
        }

        if (!Heap_API_Free(current_task)) {
            return false;
        }

        current_task = next_task;
    }

    sMission_t *prev_mission = NULL;
    sMission_t *iter = queue->mission_head;

    while (iter != NULL && iter != mission) {
        prev_mission = iter;
        iter = iter->next;
    }

    if (iter == NULL) {
        return false;
    }

    if (prev_mission == NULL) {
        queue->mission_head = mission->next;
    } else {
        prev_mission->next = mission->next;
    }

    if (mission == queue->mission_tail) {
        queue->mission_tail = prev_mission;
    }

    if (queue->current == mission) {
        queue->current = mission->next;
    }

    if (!Heap_API_Free(mission)) {
        return false;
    }

    return true;
}

static void Mission_APP_StopAction (const sActionSequence_t *action) {
    if ((action == NULL) || (action->fp == NULL) || (action->fp->action_stop == NULL)) {
        return;
    }

    action->fp->action_stop();

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Mission_APP_Init (void) {
    if (g_is_initialized) {
        return true;
    }

    if (!Heap_API_Init()) {
        TRACE_ERR("Init: Failed to initialize Heap API\n");
        
        return false;
    }

    if (!LED_API_Init()) {
        TRACE_ERR("Init: Failed to initialize LED API\n");
        
        return false;
    }

    if (!ESP_Comm_APP_Init()) {
        TRACE_ERR("Init: Failed to initialize ESP Communication\n");
        
        return false;
    }

    if (!Action_DriveDistance_Init(&Mission_APP_EmergencyStop)) {
        TRACE_ERR("Init: Failed to initialize Drive Distance Action\n");
        
        return false;
    }

    if (!Action_Measure_Init(&Mission_APP_EmergencyStop)) {
        TRACE_ERR("Init: Failed to initialize Measure Action\n");
        
        return false;
    }

    if (!Action_WallFollow_Init(&Mission_APP_EmergencyStop)) {
        TRACE_ERR("Init: Failed to initialize Wall Follow Action\n");
        
        return false;
    }

    g_mission_thread = osThreadNew(Mission_APP_Thread, NULL, &g_mission_thread_attributes);

    if (g_mission_thread == NULL) {
        TRACE_ERR("Init: Failed to create Mission thread\n");
        
        return false;
    }

    g_mission_mutex = osMutexNew(&g_mission_mutex_attributes);

    if (g_mission_mutex == NULL) {
        TRACE_ERR("Init: Failed to create mutex\n");
        
        return false;
    }

    g_mission_message_queue = osMessageQueueNew(MISSION_APP_MESSAGE_QUEUE_CAPACITY, sizeof(sMission_t *), &g_mission_message_queue_attributes);

    if (g_mission_message_queue == NULL) {
        TRACE_ERR("Init: Failed to create message queue\n");
        
        return false;
    }

    g_is_initialized = true;
    
    return g_is_initialized;
}

sMission_t *Mission_APP_CreateMission (void) {
    if (!g_is_initialized) {
        return NULL;
    }

    sMission_t *new_mission = Heap_API_Malloc(sizeof(sMission_t));

    if (new_mission == NULL) {
        TRACE_ERR("CreateMission: Failed to allocate memory for new mission\n");
        
        return NULL;
    }

    new_mission->id = g_mission_id_counter++;
    new_mission->task_head = NULL;
    new_mission->task_tail = NULL;
    new_mission->current_task = NULL;
    new_mission->next = NULL;
    new_mission->state = eMissionState_Last;

    return new_mission;
}

bool Mission_APP_AddMission (const sMission_t *mission) {
    if (!g_is_initialized) {
        return false;
    }
    
    if (mission == NULL) {
        TRACE_ERR("AddMission: NULL mission pointer\n");
        
        return false;
    }

    if (g_mission_message_queue == NULL){
        TRACE_ERR("AddMission: Message queue ID is NULL\n");
        
        return false;
    }

    if (osMessageQueuePut(g_mission_message_queue, &mission, MISSION_APP_MESSAGE_QUEUE_PRIORITY, MISSION_APP_MESSAGE_QUEUE_TIMEOUT) != osOK) {
        TRACE_ERR("AddMission: Failed to put mission in message queue\n");
        
        return false;
    }

    TRACE_INFO("AddMission: Mission [%d] added to queue\n", mission->id);

    return true;
}

void Mission_APP_CancelMission (const uint32_t mission_id) {
    if (!g_is_initialized) {
        return;
    }

    if (osMutexAcquire(g_mission_mutex, MISSION_APP_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("CancelMission: Failed to acquire mission mutex\n");
        
        return;
    }

    sMission_t *current_mission = g_mission_sequence.mission_head;

    while (current_mission != NULL) {
        if (current_mission->id == mission_id) {
            current_mission->state = eMissionState_Canceled;

            break;
        }

        current_mission = current_mission->next;
    }

    osMutexRelease(g_mission_mutex);
            
    return;
}

void Mission_APP_EmergencyStop (const void *instance, eEmergencyType_t type, const char *response) {
    if (!g_is_initialized) {
        return;
    }

    if (type < eEmergencyType_First || type >= eEmergencyType_Last) {
        TRACE_ERR("EmergencyStop: Invalid emergency type [%d]\n", type);
        return;
    }

    sMission_t *mission = NULL;
    sTaskSequence_t *task = NULL;

    if (osMutexAcquire(g_mission_mutex, MISSION_APP_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("EmergencyStop: Failed to acquire mission mutex\n");
        return;
    }

    if (instance == NULL) {
        mission = g_mission_sequence.current;

        TRACE_WRN("EmergencyStop: No instance provided; using current mission\n");
    } else {
        mission = (sMission_t *)instance;
    }

    if (mission == NULL) {
        TRACE_ERR("EmergencyStop: Invalid mission instance\n");
        
        osMutexRelease(g_mission_mutex);
        
        return;
    }

    switch (type) {
        case eEmergencyType_TaskCritical: {
            task = mission->current_task;
        } break;
        case eEmergencyType_MissionCritical: {
            task = mission->task_head;
        } break;
        case eEmergencyType_AllCritical: {
            mission = g_mission_sequence.mission_head;

            if (mission == NULL) {
                TRACE_ERR("EmergencyStop: No missions to stop\n");
                
                osMutexRelease(g_mission_mutex);
                
                return;
            }

            task = mission->task_head;
        } break;
        default: {
            TRACE_ERR("EmergencyStop: Unexpected emergency type [%d]\n", type);
            
            osMutexRelease(g_mission_mutex);
            
            return;
        }
    }

    osMutexRelease(g_mission_mutex);

    if (task == NULL) {
        TRACE_ERR("EmergencyStop: No current task in mission [%d]\n", mission->id);

        return;
    }

    while (mission != NULL) {
        while (task != NULL) {
            sActionSequence_t *action = task->action_head;
            
            while (action != NULL) {
                if ((action->state == eActionState_Running) || (action->state == eActionState_Pause)) {
                    Mission_APP_StopAction(action);
                }

                action = action->next;
            }

            if (type == eEmergencyType_TaskCritical) {
                break;
            }

            task = task->next;
        }

        if (type == eEmergencyType_MissionCritical) {
            break;
        }

        mission = mission->next;
    }

    if (osMutexAcquire(g_mission_mutex, MISSION_APP_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("EmergencyStop: Failed to re-acquire mutex to update mission state\n");
        return;
    }

    if (type == eEmergencyType_AllCritical) {
        mission = g_mission_sequence.mission_head;
        
        while (mission != NULL) {
            mission->state = eMissionState_Error;
            mission = mission->next;
        }
    } else {
        g_mission_sequence.current->state = eMissionState_Error;
    }

    osMutexRelease(g_mission_mutex);

    TRACE_ERR("EmergencyStop: %s", (response == NULL) ? "Emergency stop triggered" : response);

    return;
}

bool Mission_APP_AddTask (sMission_t *mission) {
    if (!g_is_initialized) {
        return false;
    }
    
    if (mission == NULL) {
        TRACE_ERR("AddTask: NULL mission pointer\n");
        
        return false;
    }
    
    sTaskSequence_t *new_task = Heap_API_Malloc(sizeof(sTaskSequence_t));

    if (new_task == NULL) {
        TRACE_ERR("AddTask: Failed to allocate memory for new task\n");
        
        return false;
    }

    new_task->action_head = NULL;
    new_task->action_tail = NULL;
    new_task->next = NULL;

    if (mission->task_head == NULL) {
        mission->task_head = new_task;
        mission->current_task = new_task;
    } else {
        if (mission->task_tail == NULL) {
            TRACE_ERR("AddTask: Mission tail is NULL while head is not NULL\n");
            
            return false;
        }

        mission->task_tail->next = new_task;
    }

    mission->task_tail = new_task;

    return true;
}

// TODO: Check current actions to prevent duplicate actions in a task
bool Mission_APP_AddAction (const sMission_t *mission, const eAction_t type, const void *data) {
    if (!g_is_initialized) {
        return false;
    }

    if (mission == NULL) {
        TRACE_ERR("AddAction: NULL mission pointer\n");
        
        return false;
    }
    
    if (!Mission_APP_IsCorrectAction(type)) {
        TRACE_ERR("AddAction: Invalid action type %u\n", type);
        
        return false;
    }

    if (mission->task_tail == NULL) {
        TRACE_ERR("AddAction: No task to add action to\n");
        
        return false;
    }
    
    sTaskSequence_t *current_task = mission->task_tail;
    sActionSequence_t *new_action = Heap_API_Malloc(sizeof(sActionSequence_t));

    if (new_action == NULL) {
        TRACE_ERR("AddAction: Failed to allocate memory for new action\n");
        
        return false;
    }

    new_action->action = type;
    new_action->fp = &g_static_action_desc_lut[type];
    new_action->data = data;
    new_action->state = eActionState_Init;
    new_action->next = NULL;

    if (current_task->action_head == NULL) {
        current_task->action_head = new_action;
    } else {
        if (current_task->action_tail == NULL) {
            TRACE_ERR("AddAction: Task tail is NULL while head is not NULL\n");
            
            return false;
        }

        current_task->action_tail->next = new_action;
    }
    
    current_task->action_tail = new_action;

    return true;
}

bool Mission_APP_IsCorrectAction (const eAction_t type) {
    return (type >= eAction_First && type < eAction_Last);
}

eErrorCode_t Mission_APP_ParseActionData (const eAction_t type, sMessage_t *arguments, void **data, sMessage_t *response) {
    if (!g_is_initialized) {
        return eErrorCode_BADSTATE;
    }

    if (!Mission_APP_IsCorrectAction(type)) {
        return eErrorCode_INVAL;
    }
    
    if ((arguments == NULL) || (data == NULL) || (response == NULL)) {
        return eErrorCode_NULLPTR;
    }

    return g_static_action_desc_lut[type].action_parse(arguments, data, response);
}
