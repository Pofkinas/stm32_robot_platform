/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

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
#include "debug_api.h"
#include "heap_api.h"
#include "timer_driver.h"
#include "baudrate.h"

#include "mission_app.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_MAIN
CREATE_MODULE_NAME (MAIN)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_MAIN */

static const osThreadAttr_t g_init_thread_attributes = {
    .name = "Main_Init",
    .stack_size = (256 * 8),
    .priority = osPriorityRealtime
};

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

volatile unsigned long ulHighFrequencyTimerTicks;

static osThreadId_t g_init_thread = NULL;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static void SystemClock_Config (void);
void configureTimerForRunTimeStats (void);
void TIM1_UP_TIM10_IRQHandler (void);
unsigned long getRunTimeCounterValue (void);

static void Main_Init_Thread (void *arg);

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
    LL_SetSystemCoreClock(SYSTEM_CLOCK_HZ);

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

// Workaround for VL53L0X Init that requires Kernel to be running
static void Main_Init_Thread (void *arg) {
    bool init_success = true;

    Timer_Driver_InitAllTimers();
    Timer_Driver_Start(eTimer_TIM10);

    // if (!CLI_APP_Init(UART_2_BAUDRATE)) {
    //     init_success = false;
    // }

    if (!Heap_API_Init()) {
        while (1) {}
    }

    if (!Debug_API_Init(UART_1_BAUDRATE)) {
        while (1) {}
    }

    if (!LED_APP_Init()) {
        TRACE_ERR("LED_APP_Init failed\n");

        init_success = false;
    }

    if (!Motor_APP_Init()) {
        TRACE_ERR("Motor_APP_Init failed\n");

        init_success = false;
    }

    if (!Mission_APP_Init()) {
        TRACE_ERR("Mission_APP_Init failed\n");

        init_success = false;
    }

    if (!init_success) {
        while (1) {}
    }

    TRACE_INFO("Start OK\n");

    osThreadTerminate(g_init_thread);
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

    g_init_thread = osThreadNew(Main_Init_Thread, NULL, &g_init_thread_attributes);

    if (g_init_thread == NULL) {
        while (1) {}
    }

    osKernelStart();

    while (1) {}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        HAL_IncTick();
    }
}
