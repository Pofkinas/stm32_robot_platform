#ifndef SOURCE_API_IO_API_H_
#define SOURCE_API_IO_API_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include "cmsis_os2.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define STARTSTOP_BUTTON_DEBOUNCE_PERIOD 50U
#define TCRT5000_DEBOUNCE_PERIOD 15U // Used to debounce PWM EMS

#define STARTSTOP_TRIGGERED_EVENT 0x01U
#define TCRT5000_RIGHT_TRIGGERED_EVENT 0x01U
#define TCRT5000_LEFT_TRIGGERED_EVENT 0x02U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eIo {
    eIo_First = 0,
    eIo_StartStopButton = eIo_First,
    eIo_Tcrt5000_Right,
    eIo_Tcrt5000_Left,
    eIo_Last
} eIo_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool IO_API_Init (eIo_t device, osEventFlagsId_t event_flags_id);
bool IO_API_IsCorrectDevice (const eIo_t device);
bool IO_API_ReadPinState (const eIo_t device, bool *pin_state);

#endif /* SOURCE_API_IO_API_H_ */
