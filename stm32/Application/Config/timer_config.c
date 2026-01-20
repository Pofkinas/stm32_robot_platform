/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "timer_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sTimerDesc_t g_static_timer_lut[eTimer_Last] = {
    [eTimer_TIM10] = {
        .periph = TIM10,
        .prescaler = 999,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 65535,
        .clock_division = LL_TIM_CLOCKDIVISION_DIV1,
        .clock = LL_APB2_GRP1_PERIPH_TIM10,
        .clock_source_fp = NULL,
        .enable_clock_fp = LL_APB2_GRP1_EnableClock,
        .nvic = TIM1_UP_TIM10_IRQn,
        .enable_interupt = true,
        .auto_relead_preload_fp = LL_TIM_DisableARRPreload,
        .master_slave_mode_fp = NULL
    },
    [eTimer_TIM3] = {
        .periph = TIM3,
        .prescaler = 0,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 999,
        .clock_division = LL_TIM_CLOCKDIVISION_DIV1,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .clock = LL_APB1_GRP1_PERIPH_TIM3,
        .clock_source_fp = LL_TIM_SetClockSource,
        .clock_source = LL_TIM_CLOCKSOURCE_INTERNAL,
        .enable_interupt = false,
        .auto_relead_preload_fp = LL_TIM_DisableARRPreload,
        .master_slave_mode_fp = LL_TIM_DisableMasterSlaveMode,
        .set_trigger = LL_TIM_SetTriggerOutput,
        .triger_sync = LL_TIM_TRGO_RESET
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool Timer_Config_IsCorrectTimer (const eTimer_t timer) {
    return (timer >= eTimer_First) && (timer < eTimer_Last);
}

const sTimerDesc_t *Timer_Config_GetTimerDesc (const eTimer_t timer) {
    if (!Timer_Config_IsCorrectTimer(timer)) {
        return NULL;
    }

    return &g_static_timer_lut[timer];
}
