
/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command lookup table file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command lookup table implimentation for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * project_cli_lut.c
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Use the `INCLUDE_PROJECT_CLI` macro in `platform_config.h to include custom cli lut in your project.
 * 4. Include the custom commands defined in `project_cli_cmd_handlers.h`.
 * 5. Define your custom commands definitions in `g_framework_cli_lut` sCmdDesc_t.
 * 6. Provide the command handler function for each command in `g_framework_cli_lut`.
 * 7. Implement your custom commands definition header `project_cli_lut.h`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "custom_cli_lut.h"

#ifdef ENABLE_CUSTOM_CMD
#include "custom_cli_cmd_handlers.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define DEFINE_CMD(command_string) .command = command_string, .command_length = sizeof(command_string) - 1

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/* clang-format off */
sCmdDesc_t g_custom_cmd_lut[eCliCustomCmd_Last] = {
    [eCliCustomCmd_RequestRpm] = {
        DEFINE_CMD("request_rpm"),
        .handler = Custom_CLI_APP_Handlers_RequestRpm
        /* e. g. request_rpm */
    },
    [eCliCustomCmd_ReadRpm] = {
        DEFINE_CMD("read_rpm:"),
        .handler = Custom_CLI_APP_Handlers_ReadRpm
        /* e. g. read_rpm:<eEncoder_t> */
    },
    [eCliCustomCmd_Set_MotorOdom] = {
        DEFINE_CMD("modom_set:"),
        .handler = Custom_CLI_APP_Handlers_Set_MotorOdom
        /* e. g. modom_set:<speed %>, <eMotorDirection_t>, <eMotorControl_t> */
    },
    [eCliCustomCmd_Stop_MotorOdom] = {
        DEFINE_CMD("modom_stop"),
        .handler = Custom_CLI_APP_Handlers_Stop_MotorOdom
        /* e. g. modom_stop */
    },
    [eCliCustomCmd_ResetOdom] = {
        DEFINE_CMD("reset_odom"),
        .handler = Custom_CLI_APP_Handlers_ResetOdom
        /* e. g. reset_odom */
    },
    [eCliCustomCmd_AddMission] = {
        DEFINE_CMD("mission:"),
        .handler = Custom_CLI_APP_Handlers_AddMission
        /* e. g. mission: (task 1) <eAction_t>, <params>, <eAction_t>, <params>; (task 2) <eAction_t>, <params> */
    },
    [eCliCustomCmd_CancelMission] = {
        DEFINE_CMD("cancel:"),
        .handler = Custom_CLI_APP_Handlers_CancelMission
        /* e. g. cancel:<mission_id> */
    },
    [eCliCustomCmd_EmergencyStop] = {
        DEFINE_CMD("stop"),
        .handler = Custom_CLI_APP_Handlers_EmergencyStop
        /* e. g. stop */
    },
    [eCliCustomCmd_SetPID] = {
        DEFINE_CMD("set_pid:"),
        .handler = Custom_CLI_APP_Handlers_SetPID
        /* e. g. set_pid:<kp>,<ki>,<kd>,<integral_limit> */
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

#endif /* ENABLE_CUSTOM_CMD */
