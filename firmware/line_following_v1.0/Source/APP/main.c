/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "main.h"
#include "cmsis_os.h"
#include "FreeRTOSConfig.h"
#include "stm32f4xx_ll_rcc.h"
#include "stm32f4xx_ll_system.h"
#include "stm32f4xx_ll_utils.h"
#include "stm32f4xx_ll_pwr.h"
#include "stm32f4xx_ll_usart.h"
#include "usart.h"
#include "cli_app.h"
#include "led_app.h"
#include "motor_app.h"
#include "tracker_app.h"
#include "debug_api.h"
#include "timer_driver.h"
#include "uart_baudrate.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

CREATE_MODULE_NAME (MAIN)

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void SystemClock_Config (void);

volatile unsigned long ulHighFrequencyTimerTicks;

void configureTimerForRunTimeStats (void);
void TIM1_UP_TIM10_IRQnHandler (void);

unsigned long getRunTimeCounterValue (void);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void SystemClock_Config (void) {
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);
    while(LL_FLASH_GetLatency()!= LL_FLASH_LATENCY_3) {}
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_HSI_SetCalibTrimming(16);
    LL_RCC_HSI_Enable();

    while(LL_RCC_HSI_IsReady() != 1) {}
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 100, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_Enable();

    while(LL_RCC_PLL_IsReady() != 1) {}
    while (LL_PWR_IsActiveFlag_VOS() == 0) {}
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_2);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {}
    LL_SetSystemCoreClock(100000000);

    if (HAL_InitTick (TICK_INT_PRIORITY) != HAL_OK) {
        __disable_irq();
        while (1) {}
    }
}

void configureTimerForRunTimeStats (void) {
    ulHighFrequencyTimerTicks = 0;
    LL_TIM_EnableIT_UPDATE(TIM10);
    LL_TIM_EnableCounter(TIM10);
}

unsigned long getRunTimeCounterValue (void) {
    return ulHighFrequencyTimerTicks;
}

void TIM1_UP_TIM10_IRQHandler (void) {
    if (LL_TIM_IsActiveFlag_UPDATE(TIM10)) {
        ulHighFrequencyTimerTicks++;
        LL_TIM_ClearFlag_UPDATE(TIM10);
    }
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

int main (void) {
    LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

    NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

    NVIC_SetPriority(PendSV_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));
    NVIC_SetPriority(SysTick_IRQn, NVIC_EncodePriority(NVIC_GetPriorityGrouping(), 15, 0));

    SystemClock_Config();

    osKernelInitialize();

    // Init TIM10 for debbuging stack size
    Timer_Driver_InitAllTimers();
    Timer_Driver_Start(eTimerDriver_TIM10);

    LED_APP_Init();
    Motor_APP_Init();
    Tracker_APP_Init();
    CLI_APP_Init(eUartBaudrate_115200);

    TRACE_INFO("Start OK\n");

    osKernelStart();

    while (1) {}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        HAL_IncTick();
    }
}
