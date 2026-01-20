#ifndef CONFIG_IO_CONFIG_H_
#define CONFIG_IO_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include "cmsis_os2.h"
#include "gpio_config.h"
#include "exti_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define STARTSTOP_TRIGGERED_EVENT 0x01U
#define STARTSTOP_BUTTON_DEBOUNCE_PERIOD 1U

#define TCRT5000_RIGHT_TRIGGERED_EVENT 0x01U
#define TCRT5000_LEFT_TRIGGERED_EVENT 0x02U
#define TCRT5000_DEBOUNCE_PERIOD 1U

#define IO_MESSAGE_QUEUE_PRIORITY 0U
#define IO_MESSAGE_QUEUE_TIMEOUT 5U
#define IO_MESSAGE_CAPACITY 10

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eIo {
    eIo_First = 0,
    eIo_StartStopButton = eIo_First,
    eIo_Tcrt5000_Right,
    eIo_Tcrt5000_Left,
    eIo_Last
} eIo_t;

typedef enum eActiveState {
    eActiveState_First = 0,
    eActiveState_Low = eActiveState_First,
    eActiveState_High,
    eActiveState_Both,
    eActiveState_Last
} eActiveState_t;

typedef struct sIoDesc {
    eGpio_t gpio_pin;
    eActiveState_t active_state;
    uint32_t triggered_flag;
    bool is_debounce_enable;
    uint16_t debounce_period;
    osMutexAttr_t mutex_attributes;
    osTimerAttr_t debounce_timer_attributes;
    bool is_exti;
    eExti_t exti_device;
} sIoDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern const osThreadAttr_t g_io_thread_attributes;
extern const osMessageQueueAttr_t g_io_message_queue_attributes;

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool IO_Config_IsCorrectIo (const eIo_t io_device);
const sIoDesc_t *IO_Config_GetIoDesc (const eIo_t io_device);

#endif /* CONFIG_IO_CONFIG_H_ */
