#ifndef SOURCE_DRIVER_EXTI_DRIVER_H_
#define SOURCE_DRIVER_EXTI_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eExtiDriver {
    eExtiDriver_First = 0,
    eExtiDriver_StartButton = eExtiDriver_First,
    eExtiDriver_Tcrt5000,
    eExtiDriver_Last
} eExtiDriver_t;
/* clang-format on */

typedef void (*exti_callback_t) (const eExtiDriver_t exti_device);

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Exti_Driver_InitDevice (eExtiDriver_t exti_device, exti_callback_t exti_callback);
bool Exti_Driver_Disable_IT (const eExtiDriver_t exti_device);
bool Exti_Driver_Enable_IT (const eExtiDriver_t exti_device);

#endif /* SOURCE_DRIVER_EXTI_DRIVER_H_ */
