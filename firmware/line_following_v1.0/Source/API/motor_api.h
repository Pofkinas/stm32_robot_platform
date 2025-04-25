#ifndef SOURCE_API_MOTOR_API_H_
#define SOURCE_API_MOTOR_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stddef.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define STOP_SPEED 0
#define DEFAULT_MOTOR_SPEED 60
#define MOTOR_SEARCH_SPEED 5

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eMotor {
    eMotor_First,
    eMotor_Right = eMotor_First,
    eMotor_Left,
    eMotor_Last
} eMotor_t;

typedef enum eMotorDirection {
    eMotorDirection_First,
    eMotorDirection_Forward = eMotorDirection_First,
    eMotorDirection_Backward,
    eMotorDirection_Right,
    eMotorDirection_Left,
    eMotorDirection_Last
} eMotorDirection_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_API_Init (void);
bool Motor_API_SetSpeed (const size_t speed, const eMotorDirection_t direction);
bool Motor_API_StopAllMotors (void);
bool Motor_API_IsCorrectDirection (const eMotorDirection_t direction);
bool Motor_API_IsCorrectSpeed (const size_t speed);
bool Motor_API_IsMotorEnabled (const eMotor_t motor);

#endif /* SOURCE_API_MOTOR_API_H_ */
