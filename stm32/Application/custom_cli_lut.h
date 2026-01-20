#ifndef APPLICATION_CUSTOM_CLI_LUT_H_
#define APPLICATION_CUSTOM_CLI_LUT_H_

/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command lookup table header file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command lookup table definition for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * project_cli_lut.h
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Use the `ENABLE_CUSTOM_CLI` macro in `platform_config.h` to include custom cli lut in your project.
 * 4. Define your custom commands in `eCliCustomCmd` enum.
 * 5. Implement the custom commands definitions in `custom_cli_lut.c`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#ifdef ENABLE_CUSTOM_CMD
#include "cmd_api.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eCliCustomCmd {
    eCliCustomCmd_First = 0,
    eCliCustomCmd_RequestRpm,
    eCliCustomCmd_ReadRpm,
    eCliCustomCmd_Set_MotorOdom,
    eCliCustomCmd_Stop_MotorOdom,
    eCliCustomCmd_ResetOdom,
    eCliCustomCmd_AddMission,
    eCliCustomCmd_CancelMission,
    eCliCustomCmd_EmergencyStop,
    eCliCustomCmd_SetPID,
    eCliCustomCmd_Last
} eCliCustomCmd;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern sCmdDesc_t g_custom_cmd_lut[eCliCustomCmd_Last];

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

#endif /* ENABLE_CUSTOM_CMD */
#endif /* APPLICATION_CUSTOM_CLI_LUT_H_ */
