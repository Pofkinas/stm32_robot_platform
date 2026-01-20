#ifndef APPLICATION_MISSION_APP_H_
#define APPLICATION_MISSION_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "action_state.h"
#include "error_messages.h"
#include "message.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define MISSION_APP_THREAD_STACK_SIZE (256 * 8)
#define MISSION_APP_THREAD_PRIORITY osPriorityNormal

#define MISSION_APP_MESSAGE_QUEUE_CAPACITY 10
#define MISSION_APP_MESSAGE_QUEUE_PRIORITY 0U
#define MISSION_APP_MESSAGE_QUEUE_TIMEOUT 0U
#define MISSION_APP_MUTEX_TIMEOUT 0U

#define TASK_DELIMITER ";"

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eAction {
    eAction_First = 0,
    eAction_DriveDistance = eAction_First,
    eAction_Measure,
    eAction_WallFollow,
    eAction_Last
} eAction_t;

typedef enum eMissionState {
    eMissionState_First = 0,
    eMissionState_Pending = eMissionState_First,
    eMissionState_Running,
    eMissionState_Completed,
    eMissionState_Canceled,
    eMissionState_Error,
    eMissionState_Last
} eMissionState_t;

typedef struct sMission sMission_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Mission_APP_Init (void);

sMission_t *Mission_APP_CreateMission (void);
bool Mission_APP_AddMission (const sMission_t *mission);
void Mission_APP_CancelMission (const uint32_t mission_id);
void Mission_APP_EmergencyStop (const void *instance, eEmergencyType_t type, const char *response);

bool Mission_APP_AddTask (sMission_t *mission);
bool Mission_APP_AddAction (const sMission_t *mission, const eAction_t type, const void *data);

bool Mission_APP_IsCorrectAction (const eAction_t type);
eErrorCode_t Mission_APP_ParseActionData (const eAction_t type, sMessage_t *arguments, void **data, sMessage_t *response);

#endif /* APPLICATION_MISSION_APP_H_ */
