#ifndef APPLICATION_ACTION_ACTION_STATE_H_
#define APPLICATION_ACTION_ACTION_STATE_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eActionState {
    eActionState_First = 0,
    eActionState_Init = eActionState_First,
    eActionState_Running,
    eActionState_Pause,
    eActionState_Continue,
    eActionState_Completed,
    eActionState_Error,
    eActionState_Last
} eActionState_t;

typedef enum eEmergencyType {
    eEmergencyType_First = 0,
    eEmergencyType_TaskCritical = eEmergencyType_First,
    eEmergencyType_MissionCritical,
    eEmergencyType_AllCritical,
    eEmergencyType_Last
} eEmergencyType_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#endif /* APPLICATION_ACTION_ACTION_STATE_H_ */
