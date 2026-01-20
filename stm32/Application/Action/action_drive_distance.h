#ifndef APPLICATION_ACTION_ACTION_DRIVE_DISTANCE_H_
#define APPLICATION_ACTION_ACTION_DRIVE_DISTANCE_H_
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

#define MIN_DRIVE_DISTANCE_MM 1U
#define MAX_DRIVE_DISTANCE_MM 10000U // 10 meters

#define MIN_SPEED_MMS 0U 
#define MAX_SPEED_MMS 10000U

#define MIN_HEADING_DEG 0.0f
#define MAX_HEADING_DEG 359.9f

#define EVENT_FLAG_TIMEOUT 0U
#define NEW_DATA_FLAG 0x80
#define DRIVE_DISTANCE_MUTEX_TIMEOUT 0U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef struct sDriveDistance {
    uint16_t distance_mm;
    uint8_t speed_mms; // Speed in mm/s
    float heading_deg; // Heading in degrees (0-359)
    uint16_t timeout_ms; // Action timeout (0 = no timeout)
} sDriveDistance_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Action_DriveDistance_Init (void (*em_stop)(const void *instance, eEmergencyType_t type, const char *response));
bool Action_DriveDistance_Run (const void *data, void *instance, eActionState_t *state);
bool Action_DriveDistance_Service (eActionState_t *state);
void Action_DriveDistance_Stop (void);
eErrorCode_t Action_DriveDistance_Parse (sMessage_t *arguments, void **data, sMessage_t *response);

bool Action_DriveDistance_SetState (eActionState_t *state);
bool Action_DriveDistance_UpdateData (sDriveDistance_t *data);

bool Action_DriveDistance_IsCorrectDistance (const uint16_t distance_mm);
bool Action_DriveDistance_IsCorrectSpeed (const uint8_t speed_mms);
bool Action_DriveDistance_IsCorrectHeading (const float heading_deg);

#endif /* APPLICATION_ACTION_ACTION_DRIVE_DISTANCE_H_ */
