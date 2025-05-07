#ifndef SOURCE_DRIVER_MOTOR_DRIVER_H_
#define SOURCE_DRIVER_MOTOR_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eMotorDriver {
    eMotorDriver_First,
    eMotorDriver_A = eMotorDriver_First,
    eMotorDriver_B,
    eMotorDriver_Last
} eMotorDriver_t;

typedef enum eMotorRotation {
    eMotorRotation_First,
    eMotorRotation_Forward = eMotorRotation_First,
    eMotorRotation_Backward,
    eMotorRotation_Last
} eMotorRotation_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_Driver_InitAllMotors (void);
bool Motor_Driver_EnableMotor (const eMotorDriver_t motor);
bool Motor_Driver_DisableMotor (const eMotorDriver_t motor);
bool Motor_Driver_SetSpeed (const eMotorDriver_t motor, const eMotorRotation_t rotation_dir, const size_t speed);
bool Motor_Driver_GetMaxSpeed (const eMotorDriver_t motor, uint16_t *speed);

#endif /* SOURCE_DRIVER_MOTOR_DRIVER_H_ */
