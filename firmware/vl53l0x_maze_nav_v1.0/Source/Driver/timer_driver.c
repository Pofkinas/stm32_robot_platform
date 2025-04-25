/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "timer_driver.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_tim.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sTimerDesc {
    TIM_TypeDef *periph;
    uint16_t prescaler;
    uint32_t counter_mode;
    uint32_t auto_reload;
    uint32_t clock_division;
    void (*enable_clock_fp) (uint32_t);
    uint32_t clock;
    void (*clock_source_fp) (TIM_TypeDef *, uint32_t);
    uint32_t clock_source;
    bool enable_interupt;
    IRQn_Type nvic;
    void (*auto_relead_preload_fp) (TIM_TypeDef *);
    void (*master_slave_mode_fp) (TIM_TypeDef *);
    void (*set_slave_mode_fp) (TIM_TypeDef *, uint32_t);
    uint32_t slave_mode;
    void (*set_trigger) (TIM_TypeDef *, uint32_t);
    uint32_t triger_sync;
} sTimerDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sTimerDesc_t g_static_timer_lut[eTimerDriver_Last] = {
    [eTimerDriver_TIM3] = {
        .periph = TIM3,
        .prescaler = 0,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 256,
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
    },
    [eTimerDriver_TIM4] = {
        .periph = TIM4,
        .prescaler = 0,
        .counter_mode = LL_TIM_COUNTERMODE_UP,
        .auto_reload = 65535,
        .clock_division = LL_TIM_CLOCKDIVISION_DIV1,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .clock = LL_APB1_GRP1_PERIPH_TIM4,
        .clock_source_fp = LL_TIM_SetClockSource,
        .clock_source = LL_TIM_CLOCKSOURCE_INTERNAL,
        .enable_interupt = false,
        .auto_relead_preload_fp = LL_TIM_DisableARRPreload,
        .master_slave_mode_fp = LL_TIM_DisableMasterSlaveMode,
        .set_trigger = LL_TIM_SetTriggerOutput,
        .triger_sync = LL_TIM_TRGO_RESET
    },
    [eTimerDriver_TIM10] = {
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
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_all_timers_init = false;

/* clang-format off */
static bool g_is_counter_enable[eTimerDriver_Last] = {
    [eTimerDriver_TIM3] = false,
    [eTimerDriver_TIM4] = false,
    [eTimerDriver_TIM10] = false,
};
/* clang-format on */

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

bool Timer_Driver_InitAllTimers (void) {
    if (g_is_all_timers_init) {
        return true;
    }

    g_is_all_timers_init = true;

    LL_TIM_InitTypeDef timer_init_struct = {0};

    for (eTimerDriver_t timer = eTimerDriver_First; timer < eTimerDriver_Last; timer++) {
        g_static_timer_lut[timer].enable_clock_fp(g_static_timer_lut[timer].clock);

        if (g_static_timer_lut[timer].enable_interupt) {
            NVIC_SetPriority(g_static_timer_lut[timer].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 5, 0));
            NVIC_EnableIRQ(g_static_timer_lut[timer].nvic);
        }

        timer_init_struct.Prescaler = g_static_timer_lut[timer].prescaler;
        timer_init_struct.CounterMode = g_static_timer_lut[timer].counter_mode;
        timer_init_struct.Autoreload = g_static_timer_lut[timer].auto_reload;
        timer_init_struct.ClockDivision = g_static_timer_lut[timer].clock_division;
        
        if (LL_TIM_Init(g_static_timer_lut[timer].periph, &timer_init_struct) == ERROR) {
            g_is_all_timers_init = false;
        }
        
        if (g_static_timer_lut[timer].clock_source_fp != NULL) {
            g_static_timer_lut[timer].clock_source_fp(g_static_timer_lut[timer].periph, g_static_timer_lut[timer].clock_source);
        }

        if (g_static_timer_lut[timer].auto_relead_preload_fp != NULL) {
            g_static_timer_lut[timer].auto_relead_preload_fp(g_static_timer_lut[timer].periph);
        }

        if (g_static_timer_lut[timer].master_slave_mode_fp != NULL) {
            g_static_timer_lut[timer].master_slave_mode_fp(g_static_timer_lut[timer].periph);
        }

        if (g_static_timer_lut[timer].set_slave_mode_fp != NULL) {
            g_static_timer_lut[timer].set_slave_mode_fp(g_static_timer_lut[timer].periph, g_static_timer_lut[timer].slave_mode);
        }

        if (g_static_timer_lut[timer].set_trigger != NULL) {
            g_static_timer_lut[timer].set_trigger(g_static_timer_lut[timer].periph, g_static_timer_lut[timer].triger_sync);
        }
    }

    return g_is_all_timers_init;
}

bool Timer_Driver_Start (const eTimerDriver_t timer) {
    if ((timer < eTimerDriver_First) || (timer >= eTimerDriver_Last)) {
        return false;
    }

    if (!g_is_all_timers_init) {
        return false;
    }

    if (!g_is_counter_enable[timer]) {
        LL_TIM_EnableCounter(g_static_timer_lut[timer].periph);

        g_is_counter_enable[timer] = true;
    }

    return g_is_counter_enable[timer];
}

bool Timer_Driver_Stop (const eTimerDriver_t timer) {
    if ((timer < eTimerDriver_First) || (timer >= eTimerDriver_Last)) {
        return false;
    }

    if (!g_is_all_timers_init) {
        return false;
    }

    if (g_is_counter_enable[timer]) {
        LL_TIM_DisableCounter(g_static_timer_lut[timer].periph);

        g_is_counter_enable[timer] = false;
    }

    return g_is_counter_enable[timer];
}

uint16_t Timer_Driver_GetResolution (const eTimerDriver_t timer) {
    if ((timer < eTimerDriver_First) || (timer >= eTimerDriver_Last)) {
        return 0;
    }

    return LL_TIM_GetAutoReload(g_static_timer_lut[timer].periph);
}
