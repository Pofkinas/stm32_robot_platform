/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "exti_driver.h"
#include <stdint.h>
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_exti.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sExtiDesc {
    eGpioPin_t pin;
    uint32_t system_port;
    uint32_t system_line;
    uint32_t line_0_31;
    FunctionalState command;
    uint8_t mode;
    uint8_t trigger;
    IRQn_Type nvic;
} sExtiDesc_t;

typedef struct sExtiDynamic {
    bool is_init;
    exti_callback_t callback;
} sExtiDynamic_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sExtiDesc_t g_static_exti_lut[eExtiDriver_Last] = {
    [eExtiDriver_StartButton] = {
        .pin = eGpioPin_StartButton,
        .system_port = LL_SYSCFG_EXTI_PORTC,
        .system_line = LL_SYSCFG_EXTI_LINE0,
        .line_0_31 = LL_EXTI_LINE_0,
        .command = ENABLE,
        .mode = LL_EXTI_MODE_IT,
        .trigger = LL_EXTI_TRIGGER_FALLING,
        .nvic = EXTI0_IRQn,
    },
    [eExtiDriver_Tcrt5000] = {
        .pin = eGpioPin_Tcrt5000,
        .system_port = LL_SYSCFG_EXTI_PORTC,
        .system_line = LL_SYSCFG_EXTI_LINE1,
        .line_0_31 = LL_EXTI_LINE_1,
        .command = ENABLE,
        .mode = LL_EXTI_MODE_IT,
        .trigger = LL_EXTI_TRIGGER_RISING_FALLING,
        .nvic = EXTI1_IRQn,
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/* clang-format off */
static sExtiDynamic_t g_dynamic_exti_lut[eExtiDriver_Last] = {
    [eExtiDriver_StartButton] = {
        .is_init = false,
        .callback = NULL
    },
    [eExtiDriver_Tcrt5000] = {
        .is_init = false,
        .callback = NULL
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void EXTIx_IRQHandler (const IRQn_Type interupt);
void EXTI0_IRQHandler(void);
void EXTI1_IRQHandler(void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void EXTIx_IRQHandler (const IRQn_Type interupt) {
    for (eExtiDriver_t exti_device = eExtiDriver_First; exti_device < eExtiDriver_Last; exti_device++) {
        if (g_static_exti_lut[exti_device].nvic != interupt) {
            continue;
        }

        if (LL_EXTI_IsActiveFlag_0_31(g_static_exti_lut[exti_device].line_0_31)) {
            LL_EXTI_ClearFlag_0_31(g_static_exti_lut[exti_device].line_0_31);

            g_dynamic_exti_lut[exti_device].callback(exti_device);
        }
    }
}

void EXTI0_IRQHandler(void) {
    EXTIx_IRQHandler(EXTI0_IRQn);
}

void EXTI1_IRQHandler(void) {
    EXTIx_IRQHandler(EXTI1_IRQn);
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Exti_Driver_InitDevice (eExtiDriver_t exti_device, exti_callback_t exti_callback) {
    if (g_dynamic_exti_lut[exti_device].is_init) {
        return true;
    }

    if ((exti_device < eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    LL_EXTI_InitTypeDef exti_init_struct = {0};

    LL_SYSCFG_SetEXTISource(g_static_exti_lut[exti_device].system_port, g_static_exti_lut[exti_device].system_line);

    exti_init_struct.Line_0_31 = g_static_exti_lut[exti_device].line_0_31;
    exti_init_struct.LineCommand = g_static_exti_lut[exti_device].command;
    exti_init_struct.Mode = g_static_exti_lut[exti_device].mode;
    exti_init_struct.Trigger = g_static_exti_lut[exti_device].trigger;

    if (LL_EXTI_Init(&exti_init_struct) == ERROR) {
        return false;
    }

    NVIC_SetPriority(g_static_exti_lut[exti_device].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));

    NVIC_EnableIRQ(g_static_exti_lut[exti_device].nvic);

    g_dynamic_exti_lut[exti_device].is_init = true;
    g_dynamic_exti_lut[exti_device].callback = exti_callback;

    return true;
}

bool Exti_Driver_Disable_IT (const eExtiDriver_t exti_device) {
    if ((exti_device < eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    LL_EXTI_DisableIT_0_31(g_static_exti_lut[exti_device].line_0_31);

    return true;
}

bool Exti_Driver_Enable_IT (const eExtiDriver_t exti_device) {
    if ((exti_device < eExtiDriver_First) || (exti_device >= eExtiDriver_Last)) {
        return false;
    }

    LL_EXTI_EnableIT_0_31(g_static_exti_lut[exti_device].line_0_31);

    return true;
}
