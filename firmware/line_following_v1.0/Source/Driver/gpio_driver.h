#ifndef SOURCE_DRIVER_GPIO_DRIVER_H_
#define SOURCE_DRIVER_GPIO_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eGpioPin {
    eGpioPin_First = 0,
    eGpioPin_OnboardLed = eGpioPin_First,
    eGpioPin_StartButton,
    eGpioPin_DebugTx,
    eGpioPin_DebugRx,
    eGpioPin_MotorA_A1,
    eGpioPin_MotorA_A2,
    eGpioPin_MotorB_A1,
    eGpioPin_MotorB_A2,
    eGpioPin_Tcrt5000,
    eGpioPin_I2c1_SCL,
    eGpioPin_I2c1_SDA,
    eGpioPin_Last
} eGpioPin_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool GPIO_Driver_InitAllPins (void);
bool GPIO_Driver_WritePin (const eGpioPin_t gpio_pin, const bool pin_state);
bool GPIO_Driver_ReadPin (const eGpioPin_t gpio_pin, bool *pin_state);
bool GPIO_Driver_TogglePin (const eGpioPin_t gpio_pin);
bool GPIO_Driver_SetPinMode (const eGpioPin_t gpio_pin, const uint32_t mode);

#endif /* SOURCE_DRIVER_GPIO_DRIVER_H_ */
