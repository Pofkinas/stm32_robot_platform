#ifndef APPLICATION_CUSTOM_CLI_CMD_HANDLERS_H_
#define APPLICATION_CUSTOM_CLI_CMD_HANDLERS_H_

/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command handler header file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command handler definition for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * custom_cli_cmd_handler.h
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Define your custom commands handlers.
 * 4. Implement the custom commands handlers in `custom_cli_cmd_handler.c`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_CUSTOM_CMD
#include <stdbool.h>
#include "message.h"
#include "error_messages.h"

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

eErrorCode_t Custom_CLI_APP_Handlers_ReadRpm (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_RequestRpm (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_Set_MotorOdom (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_Stop_MotorOdom (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_ResetOdom (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_AddMission (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_CancelMission (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_EmergencyStop (sMessage_t arguments, sMessage_t *response);
eErrorCode_t Custom_CLI_APP_Handlers_SetPID (sMessage_t arguments, sMessage_t *response);

#endif /* ENABLE_CUSTOM_CMD */
#endif /* APPLICATION_CUSTOM_CLI_CMD_HANDLERS_H_ */
