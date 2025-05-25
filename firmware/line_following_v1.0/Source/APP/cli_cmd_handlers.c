/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cli_cmd_handlers.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "motor_app.h"
#include "tracker_app.h"
#include "cmd_api_helper.h"
#include "heap_api.h"
#include "motor_api.h"
#include "debug_api.h"
#include "error_messages.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

#define DEBUG_CLI_APP

#define MOTOR_SEPARATOR ","
#define MOTOR_SEPARATOR_LENGHT (sizeof(MOTOR_SEPARATOR) - 1)

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_CLI_APP
CREATE_MODULE_NAME (CLI_CMD_HANDLERS)
#else
CREATE_MODULE_NAME_EMPTY
#endif

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool CLI_APP_Tracker_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eTrackerTask_t task);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool CLI_APP_Tracker_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eTrackerTask_t task) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    if (!Tracker_APP_Add_Task(task)) {
        snprintf(response->data, response->size, "Failed task add\n");

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool CLI_APP_Motors_Handlers_Stop (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Stop, .data = NULL};

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Motors_Handlers_Set (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    eMotorDirection_t direction;
    size_t speed = 0;
    size_t direction_value = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &speed, MOTOR_SEPARATOR, MOTOR_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &direction_value, MOTOR_SEPARATOR, MOTOR_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    direction = direction_value;

    if (!Motor_API_IsCorrectSpeed(speed)) {
        snprintf(response->data, response->size, "%d: Incorect speed\n", speed);

        return false;
    }

    if (!Motor_API_IsCorrectDirection(direction)) {
        snprintf(response->data, response->size, "%d: Incorect motors direction\n", direction);

        return false;
    }

    sMotorCommandDesc_t formated_task = {.task = eMotorTask_Set, .data = NULL};
    sMotorSet_t *task_data = Heap_API_Calloc(1, sizeof(sMotorSet_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->speed = speed;
    task_data->direction = direction;
    formated_task.data = task_data;

    if (!Motor_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Tracker_Handlers_Start (sMessage_t arguments, sMessage_t *response) {
    eTrackerTask_t task = eTrackerTask_Start;

    return CLI_APP_Tracker_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Tracker_Handlers_Stop (sMessage_t arguments, sMessage_t *response) {
    eTrackerTask_t task = eTrackerTask_Stop;

    return CLI_APP_Tracker_Handlers_Common(arguments, response, task);
}
