#ifndef SOURCE_APP_CLI_APP_H_
#define SOURCE_APP_CLI_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include "uart_baudrate.h"
#include "message.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define CLI_COMMAND_MESSAGE_CAPACITY 20

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eCliCommand {
    eCliCommand_First = 0,
    eCliCommand_Led_Set = eCliCommand_First,
    eCliCommand_Led_Reset,
    eCliCommand_Led_Toggle,
    eCliCommand_Led_Blink,
    eCliCommand_Pwm_Led_SetBrightness,
    eCliCommand_Pwm_Led_Pulse,
    eCliCommand_Motors_Set,
    eCliCommand_Motors_Stop,
    eCliCommand_StartTracker,
    eCliCommand_StopTracker,
    eCliCommand_Last
} eCliCommand_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool CLI_APP_Init (const eUartBaudrate_t baudrate);

#endif /* SOURCE_APP_CLI_APP_H_ */
