#ifndef CONFIG_PWM_CONFIG_H_
#define CONFIG_PWM_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "platform_config.h"
#include "timer_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum ePwm {
    ePwm_First = 0,
    ePwm_MotorA = ePwm_First,
    ePwm_MotorB,
    ePwm_Last
} ePwm_t;

typedef struct sPwmOcDesc {
    TIM_TypeDef *periph;
    eTimer_t timer;
    uint32_t channel;
    uint32_t mode;
    uint32_t oc_state;
    uint32_t ocn_state;
    uint32_t compare_value;
    uint32_t oc_polarity;
    uint32_t ocn_polarity;
    uint32_t oc_idle;
    uint32_t ocn_idle;
    void (*fast_mode_fp) (TIM_TypeDef *, uint32_t);
    void (*compare_preload_fp) (TIM_TypeDef *, uint32_t);
    void (*compare_value_fp) (TIM_TypeDef *, uint32_t);
    bool is_dma_request_enabled;
    void (*dma_request_fp) (TIM_TypeDef *);
    uint32_t (*get_ccr_fp) (const TIM_TypeDef *);
} sPwmOcDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool PWM_Config_IsCorrectPwm (const ePwm_t pwm_channel);
const sPwmOcDesc_t *PWM_Config_GetPwmOcDesc (const ePwm_t pwm_channel);

#endif /* CONFIG_PWM_CONFIG_H_ */
