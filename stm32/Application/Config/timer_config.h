#ifndef CONFIG_TIMER_CONFIG_H_
#define CONFIG_TIMER_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "platform_config.h"

#include TIMER_DRIVER
#include BUS_DRIVER

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eTimer {
    eTimer_First = 0,
    eTimer_TIM10 = eTimer_First,
    eTimer_TIM3,
    eTimer_Last
} eTimer_t;

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
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Timer_Config_IsCorrectTimer (const eTimer_t timer);
const sTimerDesc_t *Timer_Config_GetTimerDesc (const eTimer_t timer);

#endif /* CONFIG_TIMER_CONFIG_H_ */
