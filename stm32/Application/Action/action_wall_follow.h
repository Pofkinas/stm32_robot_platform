#ifndef APPLICATION_ACTION_ACTION_WALL_FOLLOW_H_
#define APPLICATION_ACTION_ACTION_WALL_FOLLOW_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "action_state.h"
#include "math_utils.h"
#include "error_messages.h"
#include "message.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define WALL_FOLLOW_MUTEX_TIMEOUT 0U

#define DATA_SAMPLES 3

// Distance thresholds (in mm)
#define MIN_SAFE_DISTANCE_MM 30
#define FRONT_WALL_THRESHOLD_MM 100
#define FRONT_WALL_SLOW_DOWN_TRESHOLD_MM 250
#define SIDE_WALL_THRESHOLD_MM 200
#define MAX_SIDE_DISTANCE_MM 300

#define FRONT_TO_SIDE_DIFFERENCE 40

// Speed limits (0-100%)
#define MIN_WALL_FOLLOW_SPEED 0
#define MAX_WALL_FOLLOW_SPEED 100
#define SLOW_DOWN_REDUCTION_COEF 0.8f // (0.0 - 1.0) Reduction of base speed in slow-down
#define SPIN_SPEED_REDUCTION_COEF 0.65f // (0.0 - 1.0) Reduction of base speed in spin
#define MAX_SPEED_CORRECTION_COEF 1.0f // Maximum speed correction proportioal to max speed

// PID controller gains
// #define KP 0.0846f // Proportional
#define KP 0.0258f // Proportional
#define KI 0.0009f // Integral
// #define KD 0.0089f // Derivative
#define KD 0.0039f // Derivative
#define INTEGRAL_LIMIT 15.0f // Anti-windup limit

#define ERROR_DEADBAND_MM 2

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eWallFollowStrategy {
    eWallFollowStrategy_First = 0,
    eWallFollowStrategy_Center = eWallFollowStrategy_First,
    eWallFollowStrategy_Right,
    eWallFollowStrategy_Left,
    eWallFollowStrategy_Last
} eWallFollowStrategy_t;

typedef struct sWallFollow {
    float speed; // Base speed 0.0-100.0%
    eWallFollowStrategy_t strategy;
    int16_t target_offset_mm; // Target offset from center (for center mode) or target distance from wall (for single-wall mode)
    uint32_t timeout_ms; // Action timeout (0 = no timeout)
} sWallFollow_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Action_WallFollow_Init (void (*em_stop)(const void *instance, eEmergencyType_t type, const char *response));
bool Action_WallFollow_Run (const void *data, void *instance, eActionState_t *state);
bool Action_WallFollow_Service (eActionState_t *state);
void Action_WallFollow_Stop (void);
eErrorCode_t Action_WallFollow_Parse (sMessage_t *arguments, void **data, sMessage_t *response);

bool Action_WallFollow_IsCorrectSpeed (const uint8_t speed);
bool Action_WallFollow_IsCorrectStrategy (const eWallFollowStrategy_t strategy);
bool Action_WallFollow_SetPID (const sPID_t *pid_params);

#endif /* APPLICATION_ACTION_ACTION_WALL_FOLLOW_H_ */
