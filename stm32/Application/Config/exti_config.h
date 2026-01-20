#ifndef CONFIG_EXTI_CONFIG_H_
#define CONFIG_EXTI_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "platform_config.h"
#include "gpio_config.h"

#include EXTI_DRIVER
#include SYSTEM

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eExti {
    eExti_First = 0,
    eExti_StartButton = eExti_First,
    eExti_Tcrt5000_Right,
    eExti_Tcrt5000_Left,
    eExti_Last
} eExti_t;

typedef struct sExtiDesc {
    eGpio_t pin;
    uint32_t system_port;
    uint32_t system_line;
    uint32_t line_0_31;
    FunctionalState command;
    uint8_t mode;
    uint8_t trigger;
    IRQn_Type nvic;
} sExtiDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Exti_Config_IsCorrectExti (const eExti_t exti);
const sExtiDesc_t *Exti_Config_GetExtiDesc (const eExti_t exti);

#endif /* CONFIG_EXTI_CONFIG_H_ */
