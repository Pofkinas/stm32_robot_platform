/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "action_measure.h"

#include <string.h>
#include "vl53l0xv2_api.h"
#include "cmd_api_helper.h"
#include "debug_api.h"
#include "heap_api.h"

#include "framework_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#ifdef DEBUG_ACTION_MEASURE
CREATE_MODULE_NAME (ACTION_MEASURE)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_ACTION_MEASURE */

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sMeasureData {
    const sMeasure_t *data;
    uint16_t distances_mm[eVl53l0x_Last];
    size_t stale_count;
    void *instance;
} sMeasureData_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 
static const osThreadAttr_t g_measure_thread_attributes = {
    .name = "Action_Measure",
    .stack_size = MEASURE_THREAD_STACK_SIZE,
    .priority = MEASURE_THREAD_PRIORITY
};

static const osMutexAttr_t g_action_mutex_attributes = {
    .name = "Action_Measure",
    .attr_bits = osMutexRecursive | osMutexPrioInherit,
    .cb_mem = NULL,
    .cb_size = 0
};

static const osEventFlagsAttr_t g_measure_event_flags_attributes = {
    .name = "Action_Measure",
    .attr_bits = 0,
    .cb_mem = NULL,
    .cb_size = 0
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static eActionState_t g_action_state = eActionState_Last;
static sMeasureData_t g_action_data = {0};

static osThreadId_t g_measure_thread = NULL;
static osMutexId_t g_action_mutex = NULL;
static osEventFlagsId_t g_measure_event_flags = NULL;
 
static void (*g_em_stop_callback)(const void *instance, eEmergencyType_t type, const char *response) = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static void Action_Measure_Thread (void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static void Action_Measure_Thread (void *arg) {
    uint32_t tick = 0;
    
    while (1) {
        uint32_t flags = osEventFlagsWait(g_measure_event_flags, MEASURE_EVENT_RUN | MEASURE_EVENT_STOP, osFlagsWaitAny, (g_action_state == eActionState_Running) ? 0U : osWaitForever);

        if (flags & MEASURE_EVENT_RUN) {
            tick = osKernelGetTickCount();
        }
        
        if (flags & MEASURE_EVENT_STOP) {
            continue;
        }

        if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
            continue;
        }

        size_t sensor_count = g_action_data.data->active_sensors_count;
        Sensors_t *active_sensors = g_action_data.data->active_sensors;

        osMutexRelease(g_action_mutex);

        uint16_t distance_mm[sensor_count];
        bool stale_data[sensor_count];
        bool all_sensors_ok = true;

        for (size_t sensor = 0; sensor < sensor_count; sensor++) {
            eVl53l0x_t active_sensor = active_sensors[sensor];
            stale_data[sensor] = false;
            
            if (!VL53L0X_API_GetDistance(active_sensor, &distance_mm[sensor], MEASURE_TIMEOUT_MS)) {
                stale_data[sensor] = true;
                all_sensors_ok = false;
            } 
        }

        if (all_sensors_ok) {
            g_action_data.stale_count = 0;
        } else {
            g_action_data.stale_count++;
        }

        if (g_action_data.stale_count >= STALE_MEASURE_COUNT_THRESHOLD) {
            if (g_em_stop_callback != NULL) {
                g_em_stop_callback(g_action_data.instance, eEmergencyType_TaskCritical, "Measurements stale\n");
            } else {
                TRACE_ERR("Thread: Invalid callback\n");
            }

            continue;
        }

        if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
            continue;
        }

        for (size_t sensor = 0; sensor < sensor_count; sensor++) {
            if (!stale_data[sensor]) {
                g_action_data.distances_mm[active_sensors[sensor]] = distance_mm[sensor];
            }
        }

        osMutexRelease(g_action_mutex);

        #ifdef DEBUG_ACTION_MEASURE_DATA
        char debug_msg[DEBUG_SENSOR_BUFFER_SIZE * eVl53l0x_Last] = {0};
        for (size_t sensor = 0; sensor < sensor_count; sensor++) {
            char distance[DEBUG_SENSOR_BUFFER_SIZE] = {0};
            snprintf(distance, DEBUG_SENSOR_BUFFER_SIZE, " [%d]: %umm%s;", active_sensors[sensor], distance_mm[sensor], stale_data[sensor] ? " (STALE)" : "");
            strcat(debug_msg, distance);
        }
        TRACE_INFO("Thread:%s\n", debug_msg);
        #endif /* DEBUG_ACTION_MEASURE_DATA */

        tick += MEASURE_UPDATE_RATE_MS;

        uint32_t now = osKernelGetTickCount();
        if ((int32_t)(now - tick) > 0) {
            tick = now + MEASURE_UPDATE_RATE_MS;
        }

        osDelayUntil(tick);
    }
}



/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Action_Measure_Init (void (*em_stop)(const void *instance, eEmergencyType_t type, const char *response)) {
    if (g_action_state != eActionState_Last) {
        return true;
    }

    if (em_stop == NULL) {
        g_action_state = eActionState_Error;
        
        return false;
    }

    if (!Heap_API_Init()) {
        g_action_state = eActionState_Error;

        return false;
    }

    if (!VL53L0X_API_InitAll()) {
        g_action_state = eActionState_Error;
        
        return false;
    }

    // for (eVl53l0x_t sensor = eVl53l0x_First; sensor < eVl53l0x_Last; sensor++) {
    //     if (!VL53L0X_API_TurnOff(sensor)) {
    //         g_action_state = eActionState_Error;
            
    //         return false;
    //     }
    // }

    g_action_mutex = osMutexNew(&g_action_mutex_attributes);

    if (g_action_mutex == NULL) {
        g_action_state = eActionState_Error;

        return false;
    }

    g_measure_event_flags = osEventFlagsNew(&g_measure_event_flags_attributes);

    if (g_measure_event_flags == NULL) {
        g_action_state = eActionState_Error;

        return false;
    }

    g_measure_thread = osThreadNew(Action_Measure_Thread, NULL, &g_measure_thread_attributes);

    if (g_measure_thread == NULL) {
        g_action_state = eActionState_Error;

        return false;
    }

    g_em_stop_callback = em_stop;
    g_action_state = eActionState_Init;

    return true;
}

bool Action_Measure_Run (const void *data, void *instance, eActionState_t *state) {
    if ((g_action_state != eActionState_Init) && (g_action_state != eActionState_Completed) && (g_action_state != eActionState_Error)) {
        return false;
    }
    
    if ((state == NULL) || (data == NULL) || (instance == NULL)) {
        g_action_state = eActionState_Error;

        return false;
    }

    const sMeasure_t *params = (const sMeasure_t *)data;

    if (params->active_sensors == NULL || !Action_Measure_IsCorrectSensorCount(params->active_sensors_count)) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (!Action_Measure_IsCorrectSensorList(params->active_sensors, params->active_sensors_count)) {
        g_action_state = eActionState_Error;
        *state = g_action_state;
        
        return false;
    }

    if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
        g_action_state = eActionState_Error;
        *state = g_action_state;

        return false;
    }

    memset(&g_action_data, 0, sizeof(sMeasureData_t));

    g_action_data.data = params;
    g_action_data.instance = instance;

    for (eVl53l0x_t sensor = eVl53l0x_First; sensor < (eVl53l0x_t)g_action_data.data->active_sensors_count; sensor++) {
        bool sensor_active = true;
        Sensors_t active_sensor = g_action_data.data->active_sensors[sensor];

        // if (!VL53L0X_API_TurnOn(active_sensor)) {
        //     sensor_active = false;
        // }
        
        if (!VL53L0X_API_StartMeasuring(active_sensor)) {
            sensor_active = false;
        }

        if (!sensor_active) {
            g_action_state = eActionState_Error;
            *state = g_action_state;

            osMutexRelease(g_action_mutex);

            return false;
        }
    }

    osEventFlagsSet(g_measure_event_flags, MEASURE_EVENT_RUN);

    g_action_state = eActionState_Running;
    *state = g_action_state;

    osMutexRelease(g_action_mutex);

    return true;
}

bool Action_Measure_Service (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    eActionState_t current_state = g_action_state;

    osMutexRelease(g_action_mutex);

    *state = current_state;

    return (current_state != eActionState_Error);
}

void Action_Measure_Stop (void) {
    if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Action_Measure_Stop: Failed to acquire mutex\n");
        
        return;
    }

    eActionState_t current_state = g_action_state;

    if (current_state == eActionState_Init) {
        osMutexRelease(g_action_mutex);
        
        return;
    }

    osMutexRelease(g_action_mutex);

    osEventFlagsSet(g_measure_event_flags, MEASURE_EVENT_STOP);

    bool sensor_disabled = true;

    for (eVl53l0x_t sensor = eVl53l0x_First; sensor < (eVl53l0x_t)g_action_data.data->active_sensors_count; sensor++) {        
        Sensors_t active_sensor = g_action_data.data->active_sensors[sensor];
        
        if (!VL53L0X_API_StopMeasuring(active_sensor)) {
            sensor_disabled = false;

            TRACE_ERR("Stop: Failed to stop measuring on sensor [%d]\n", active_sensor);
        }

        // if (!VL53L0X_API_TurnOff(active_sensor)) {
        //     sensor_disabled = false;

        //     TRACE_ERR("Stop: Failed to turn off sensor [%d]\n", active_sensor);
        // }

        if (!sensor_disabled) {
            current_state = eActionState_Error;
        }
    }

    if (!Heap_API_Free(g_action_data.data->active_sensors)) {
        g_action_state = eActionState_Error;
        
        return;
    }

    if (!Heap_API_Free((void *)g_action_data.data)) {
        g_action_state = eActionState_Error;
        
        return;
    }

    if (sensor_disabled) {
        current_state = eActionState_Init;

        TRACE_INFO("Stop: Stopped measuring\n");
    }

    if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
        TRACE_ERR("Stop: Failed to re-acquire mutex\n");
        
        return;
    }

    g_action_state = current_state;

    osMutexRelease(g_action_mutex);

    return;
}

eErrorCode_t Action_Measure_Parse (sMessage_t *arguments, void **data, sMessage_t *response) {
    if ((arguments == NULL) || (data == NULL) || (response == NULL)) {
        return eErrorCode_NULLPTR;
    }

    char sensor_config = 0;
    Sensors_t temp_sensors[eVl53l0x_Last];
    size_t sensor_count = 0;

    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgChar(arguments, &sensor_config, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);
    
    if (error != eErrorCode_OK) {
        return error;
    }

    for (size_t sensor_bit = 0; sensor_bit < (size_t)eVl53l0x_Last; sensor_bit++) {
        if (sensor_config & (1 << sensor_bit)) {
            temp_sensors[sensor_count] = (eVl53l0x_t)sensor_bit;

            sensor_count++;
        }
    }

    sMeasure_t *params = Heap_API_Malloc(sizeof(sMeasure_t));

    if (params == NULL) {
        return eErrorCode_NOMEM;
    }

    Sensors_t *sensors = Heap_API_Malloc(sizeof(Sensors_t) * sensor_count);

    if (sensors == NULL) {
        Heap_API_Free(params);
        return eErrorCode_NOMEM;
    }

    memcpy(sensors, temp_sensors, sizeof(Sensors_t) * sensor_count);
    
    params->active_sensors = sensors;
    params->active_sensors_count = sensor_count;

    *data = params;

    return eErrorCode_OK;
}

bool Action_Measure_SetState (eActionState_t *state) {
    if (state == NULL) {
        return false;
    }

    if ((*state <= eActionState_First) || (*state >= eActionState_Last)) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    bool is_success = true;

    switch (*state) {
        case eActionState_Pause: {
            if (g_action_state == eActionState_Running) {
                osEventFlagsSet(g_measure_event_flags, MEASURE_EVENT_STOP);
                g_action_state = eActionState_Pause;
            } else {
                is_success = false;
            }
        } break;
        case eActionState_Continue: {
            if (g_action_state == eActionState_Pause) {
                osEventFlagsSet(g_measure_event_flags, MEASURE_EVENT_RUN);
                g_action_state = eActionState_Running;
            } else {
                is_success = false;
            }
        } break;
        case eActionState_Completed: {
            g_action_state = eActionState_Completed;
        } break;
        default: {
            is_success = false;
        } break;
    }

    *state = g_action_state;
    osMutexRelease(g_action_mutex);

    return is_success;
}

bool Action_Measure_IsCorrectSensor (const eVl53l0x_t sensor) {
    return (sensor >= eVl53l0x_First && sensor < eVl53l0x_Last);
}

bool Action_Measure_IsCorrectSensorCount (const size_t sensor_count) {
    return (sensor_count > eVl53l0x_First && sensor_count <= (eVl53l0x_Last - eVl53l0x_First));
}

bool Action_Measure_IsCorrectSensorList (const Sensors_t *sensors, const size_t sensor_count) {
    if (sensors == NULL || sensor_count == 0) {
        return false;
    }

    bool sensor_found[sensor_count];
    memset(sensor_found, 0, sizeof(sensor_found));

    for (eVl53l0x_t sensor = eVl53l0x_First; sensor < (eVl53l0x_t)sensor_count; sensor++) {
        if (!Action_Measure_IsCorrectSensor(sensors[sensor])) {
            return false;
        }

        if (sensor_found[sensors[sensor]]) {
            return false;
        }

        sensor_found[sensors[sensor]] = true;
    }

    return true;
}

osEventFlagsId_t *Action_Measure_GetEventFlag (void) {
    return &g_measure_event_flags;
}

bool Action_Measure_GetDistance (const eVl53l0x_t sensor, uint16_t *distance_mm) {
    if (!Action_Measure_IsCorrectSensor(sensor) || distance_mm == NULL) {
        return false;
    }

    if (osMutexAcquire(g_action_mutex, MEASURE_MUTEX_TIMEOUT) != osOK) {
        return false;
    }

    *distance_mm = g_action_data.distances_mm[sensor];

    osMutexRelease(g_action_mutex);

    return true;
}
