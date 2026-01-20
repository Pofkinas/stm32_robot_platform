#ifndef APPLICATION_ACTION_ACTION_MEASURE_H_
#define APPLICATION_ACTION_ACTION_MEASURE_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stddef.h>
#include "cmsis_os2.h"
#include "vl53l0xv2_config.h"
#include "action_state.h"
#include "error_messages.h"
#include "message.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define MEASURE_UPDATE_RATE_MS 50
#define MEASURE_TIMEOUT_MS 55
#define STALE_MEASURE_COUNT_THRESHOLD 20

#define MEASURE_THREAD_STACK_SIZE (256 * 8)
#define MEASURE_THREAD_PRIORITY osPriorityNormal
#define MEASURE_MUTEX_TIMEOUT 0U

#define DEBUG_SENSOR_BUFFER_SIZE 32

#define MEASURE_EVENT_RUN 0x01
#define MEASURE_EVENT_STOP 0x02
#define MEASURE_EVENT_FLAG_NEW_DATA 0x04

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef eVl53l0x_t Sensors_t;

typedef struct sMeasure {
    Sensors_t *active_sensors;
    size_t active_sensors_count;
} sMeasure_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Action_Measure_Init (void (*em_stop)(const void *instance, eEmergencyType_t type, const char *response));
bool Action_Measure_Run (const void *data, void *instance, eActionState_t *state);
bool Action_Measure_Service (eActionState_t *state);
void Action_Measure_Stop (void);
eErrorCode_t Action_Measure_Parse (sMessage_t *arguments, void **data, sMessage_t *response);
bool Action_Measure_SetState (eActionState_t *state);

bool Action_Measure_IsCorrectSensor (const eVl53l0x_t sensor);
bool Action_Measure_IsCorrectSensorCount (const size_t sensor_count);
bool Action_Measure_IsCorrectSensorList (const Sensors_t *sensors, const size_t sensor_count);

osEventFlagsId_t *Action_Measure_GetEventFlag (void);
bool Action_Measure_GetDistance (const eVl53l0x_t sensor, uint16_t *distance_mm);

#endif /* APPLICATION_ACTION_ACTION_MEASURE_H_ */
