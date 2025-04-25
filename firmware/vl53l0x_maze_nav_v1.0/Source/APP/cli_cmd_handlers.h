#ifndef SOURCE_APP_CLI_CMD_HANDLERS_H_
#define SOURCE_APP_CLI_CMD_HANDLERS_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include "message.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool CLI_APP_Led_Handlers_Set (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Led_Handlers_Reset (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Led_Handlers_Toggle (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Pwm_Led_Handlers_Set_Brightness (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Pwm_Led_Handlers_Pulse (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Motors_Handlers_Stop (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Motors_Handlers_Set (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Tracker_Handlers_Start (sMessage_t arguments, sMessage_t *response);
bool CLI_APP_Tracker_Handlers_Stop (sMessage_t arguments, sMessage_t *response);

#endif /* SOURCE_APP_CLI_APP_HANDLERS_H_ */
