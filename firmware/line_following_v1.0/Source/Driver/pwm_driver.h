#ifndef SOURCE_DRIVER_PWM_DRIVER_H_
#define SOURCE_DRIVER_PWM_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stddef.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum ePwmDevice {
    ePwmDevice_First = 0,
    ePwmDevice_MotorA_A1 = ePwmDevice_First,
    ePwmDevice_MotorA_A2,
    ePwmDevice_MotorB_A1,
    ePwmDevice_MotorB_A2,
    ePwmDevice_Last
} ePwmDevice_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool PWM_Driver_InitAllDevices (void);
bool PWM_Driver_Enable_Device (const ePwmDevice_t device);
bool PWM_Driver_Disable_Device (const ePwmDevice_t device);
bool PWM_Driver_Change_Duty_Cycle (const ePwmDevice_t device, const size_t value);

#endif /* SOURCE_DRIVER_PWM_DRIVER_H_ */
