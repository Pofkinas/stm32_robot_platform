
/***********************************************************************************************************************
 * @file 
 * @brief Custom CLI command handler file for the Pofkinas Development Framework (PDF).
 * 
 * This file is part of the Pofkinas Development Framework (PDF) and contains custom command handler implimentation for the CLI application.
 * 
 * @note Place this file in the Application/ folder of your project define.
 * 
 * @details
 * custom_cli_cmd_handler.c
 * 
 * Usage:
 * 1. Place this file in your Application/ folder of your project (e.g. ProjectName/Application/).
 * 2. Add PDF (Pofkinas Development Framework) to your project. Latest version can be found at: https://github.com/Pofkinas/pdf
 * 3. Include the used module headers.
 * 4. Define command seperator symbol (e.g. `,`) and seperator length (e.g. `sizeof(SEPARATOR) - 1`).
 * 5. Imlement the custom commands.
 * 6. Use `CMD_API_Helper_FindNextArgUInt` to parse the command arguments.
 * 7. Implement the custom commands handler definition header `custom_cli_cmd_handler.h`.
 ***********************************************************************************************************************/

/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "custom_cli_cmd_handlers.h"

#ifdef ENABLE_CUSTOM_CMD
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <cmsis_os2.h>
#include "mission_app.h"
#include "action_wall_follow.h"
#include "motor_app.h"
#include "odometry_api.h"
#include "cli_cmd_handlers.h"
#include "cmd_api_helper.h"
#include "debug_api.h"
#include "heap_api.h"
#include "float_parts.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define ENCODER_RPM_BUFFER 128

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_CUSTOM_CMD
CREATE_MODULE_NAME (CLI_CUSTOM_CMD)
#else
CREATE_MODULE_NAME_EMPTY
#endif /* DEBUG_CUSTOM_CMD */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static osTimerId_t rpm_timer;

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

// static eErrorCode_t Custom_CLI_APP_Handlers_TrackerCommon (sMessage_t arguments, sMessage_t *response, const eTrackerTask_t task);
static void rpm_timer_callback(void *arg);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static void rpm_timer_callback(void *arg) {
    osEventFlagsId_t odom_event_flag = Odometry_API_GetEventFlag();

    if ((odom_event_flag != NULL) && (osEventFlagsGet(odom_event_flag) == ODOMETRY_NEW_DATA_READY_FLAG)) {
        char encoder_rpm[ENCODER_RPM_BUFFER];
        
        snprintf(encoder_rpm, ENCODER_RPM_BUFFER, "RPM:\n");
        
        for (eEncoder_t encoder = eEncoder_First; encoder < eEncoder_Last; encoder++) {
            int16_t rpm = 0;
            if (Odometry_API_GetRPM(encoder, &rpm)) {
                snprintf(encoder_rpm + strlen(encoder_rpm), ENCODER_RPM_BUFFER - strlen(encoder_rpm), "id[%d]: %d\n", encoder, rpm);
            }
        }
        TRACE_INFO(encoder_rpm);
    } else {
        TRACE_INFO("RPM data Timeout\n");
    }

    osTimerDelete(rpm_timer);
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

eErrorCode_t Custom_CLI_APP_Handlers_RequestRpm (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    osEventFlagsId_t odom_event_flag = Odometry_API_GetEventFlag();

    if (odom_event_flag == NULL) {
        snprintf(response->data, response->size, "Failed to get odom event flag\n");

        return eErrorCode_FAILED;
    }

    if (!Odometry_API_RequestRPM()) {
        snprintf(response->data, response->size, "Failed to request RPM\n");

        return eErrorCode_FAILED;
    }

    osTimerAttr_t rpm_timer_attr = {.name = "RPM_Get", .attr_bits = 0, .cb_mem = NULL, .cb_size = 0};

    rpm_timer = osTimerNew(rpm_timer_callback, osTimerOnce, NULL, &rpm_timer_attr);

    if (rpm_timer == NULL) {
        snprintf(response->data, response->size, "Failed to create timer\n");
        
        return eErrorCode_FAILED;
    }

    osTimerStart(rpm_timer, ODOMETRY_DATA_READY_FLAG_TIMEOUT);

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_ReadRpm (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    size_t encoder_value = 0;
    int16_t rpm = 0;
    eEncoder_t encoder;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &encoder_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    encoder = encoder_value;

    if (!Odometry_API_GetRPM(encoder, &rpm)) {
        snprintf(response->data, response->size, "Failed to read RPM\n");

        return eErrorCode_FAILED;
    }

    TRACE_INFO("Encoder [%d]: %d RPM\n", encoder, rpm);

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_Set_MotorOdom (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (CLI_APP_Motors_Handlers_Set(arguments, response) != eErrorCode_OK) {
        return eErrorCode_FAILED;
    }

    if (!Odometry_API_Start()) {
        snprintf(response->data, response->size, "Failed to start odometry\n");

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_Stop_MotorOdom (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Stop, .data = NULL};

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed motor task add\n");

        return eErrorCode_FAILED;
    }

    if (!Odometry_API_Stop()) {
        snprintf(response->data, response->size, "Failed to stop odometry\n");

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_ResetOdom (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    Odometry_API_Reset();

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_AddMission (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    sMission_t *mission = Mission_APP_CreateMission();

    if (mission == NULL) {
        snprintf(response->data, response->size, "Failed to create mission\n");

        return eErrorCode_FAILED;
    }

    eAction_t type;
    eErrorCode_t error = eErrorCode_OK;

    size_t type_value = 0;
    void *data;

    while (arguments.size > 0) {
        if (!Mission_APP_AddTask(mission)) {
            Heap_API_Free(mission);
            snprintf(response->data, response->size, "Failed to add mission task\n");

            return eErrorCode_FAILED;
        }

        char *token;
        sMessage_t actions = {0};
        size_t separator_lenght = sizeof(TASK_DELIMITER) - 1;

        error = CMD_API_Helper_ParseToken(&token, &arguments, TASK_DELIMITER, response);

        if (error != eErrorCode_OK) {
            Heap_API_Free(mission);
            
            return error;
        }

        if (token == NULL) {
            actions.data = arguments.data;
            actions.size = arguments.size;
        } else {
            actions.data = arguments.data;
            actions.size = (size_t)(token - arguments.data);
        }

        while (actions.size > 0) {
            error = CMD_API_Helper_FindNextArgUInt(&actions, &type_value, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

            if (error != eErrorCode_OK) {
                Heap_API_Free(mission);
                
                return error;
            }

            type = type_value;

            error = Mission_APP_ParseActionData(type, &actions, &data, response);

            if (error != eErrorCode_OK) {
                Heap_API_Free(mission);

                return error;
            }

            if (!Mission_APP_AddAction(mission, type, data)) {
                Heap_API_Free(mission);
                Heap_API_Free(data);
                
                snprintf(response->data, response->size, "Failed to add action to mission\n");

                return eErrorCode_FAILED;
            }
        }

        if (token == NULL) {
           arguments.size = 0;

           break;
        }

        arguments.size -= (token - arguments.data + separator_lenght);
        arguments.data = token + separator_lenght;
    }

    if (!Mission_APP_AddMission(mission)) {
        snprintf(response->data, response->size, "Failed to add mission\n");

        return eErrorCode_FAILED;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_CancelMission (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    size_t mission_id = 0;
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgUInt(&arguments, &mission_id, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    Mission_APP_CancelMission((uint32_t)mission_id);

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_EmergencyStop (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    Mission_APP_EmergencyStop(NULL, eEmergencyType_AllCritical, "CLI\n");

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

eErrorCode_t Custom_CLI_APP_Handlers_SetPID (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return eErrorCode_NULLPTR;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return eErrorCode_NULLPTR;
    }

    sPID_t pid_params = {0};
    eErrorCode_t error = eErrorCode_OK;

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.kp, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.ki, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.kd, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    error = CMD_API_Helper_FindNextArgFloat(&arguments, &pid_params.integral_limit, CMD_SEPARATOR, CMD_SEPARATOR_LENGTH, response);

    if (error != eErrorCode_OK) {
        return error;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return eErrorCode_ARGMANY;
    }

    if (!Action_WallFollow_SetPID(&pid_params)) {
        snprintf(response->data, response->size, "Failed to set PID parameters\n");

        return eErrorCode_FAILED;
    }

    TRACE_INFO("Set PID: Kp: %ld.%04u, Ki: %ld.%04u, Kd: %ld.%04u, I limit: %ld.%04u\n", FLOAT_INTEGER_PART(pid_params.kp), FLOAT_FRACTIONAL_PART(pid_params.kp, 4), FLOAT_INTEGER_PART(pid_params.ki), FLOAT_FRACTIONAL_PART(pid_params.ki, 4), FLOAT_INTEGER_PART(pid_params.kd), FLOAT_FRACTIONAL_PART(pid_params.kd, 4), FLOAT_INTEGER_PART(pid_params.integral_limit), FLOAT_FRACTIONAL_PART(pid_params.integral_limit, 4));

    snprintf(response->data, response->size, "Operation successful\n");

    return eErrorCode_OK;
}

#endif /* ENABLE_CUSTOM_CMD */
