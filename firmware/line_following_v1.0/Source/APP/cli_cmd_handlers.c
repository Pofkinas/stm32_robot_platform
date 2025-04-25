/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "cli_cmd_handlers.h"
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "led_app.h"
#include "motor_app.h"
#include "tracker_app.h"
#include "cmd_api_helper.h"
#include "heap_api.h"
#include "led_api.h"
#include "motor_api.h"
#include "debug_api.h"
#include "error_messages.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

#define LED_SEPARATOR ","
#define LED_SEPARATOR_LENGHT (sizeof(LED_SEPARATOR) - 1)

#define PWM_SEPARATOR ","
#define PWM_SEPARATOR_LENGHT (sizeof(LED_SEPARATOR) - 1)

#define MOTOR_SEPARATOR ","
#define MOTOR_SEPARATOR_LENGHT (sizeof(MOTOR_SEPARATOR) - 1)

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

CREATE_MODULE_NAME (CLI_CMD_HANDLERS)

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool CLI_APP_Led_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eLedTask_t task);
static bool CLI_APP_Tracker_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eTrackerTask_t task);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool CLI_APP_Led_Handlers_Common (sMessage_t arguments, sMessage_t *response, const eLedTask_t task) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }
    
    eLedPin_t led;
    size_t led_value = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = task, .data = NULL};
    sLedCommon_t *task_data = Heap_API_Calloc(1, sizeof(sLedCommon_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led_pin = led;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

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

bool CLI_APP_Led_Handlers_Set (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Set;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Led_Handlers_Reset (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Reset;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Led_Handlers_Toggle (sMessage_t arguments, sMessage_t *response) {
    eLedTask_t task = eLedTask_Toggle;

    return CLI_APP_Led_Handlers_Common(arguments, response, task);
}

bool CLI_APP_Led_Handlers_Blink (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }
    
    eLedPin_t led;
    size_t led_value = 0;
    size_t blink_time = 0;
    size_t blink_frequency = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blink_time, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &blink_frequency, LED_SEPARATOR, LED_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectBlinkTime(blink_time)) {
        snprintf(response->data, response->size, "%d: Incorrect blink time\n", blink_time);

        return false;
    }

    if (!LED_API_IsCorrectBlinkFrequency(blink_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect blink frequency\n", blink_frequency);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Blink, .data = NULL};
    sLedBlink_t *task_data = Heap_API_Calloc(1, sizeof(sLedBlink_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led_pin = led;
    task_data->blink_time = blink_time;
    task_data->blink_frequency = blink_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Pwm_Led_Handlers_Set_Brightness (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    eLedPwmPin_t led;
    size_t led_value = 0;
    size_t duty_cycle = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, PWM_SEPARATOR, PWM_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (CMD_API_Helper_FindNextArgUInt(&arguments, &duty_cycle, PWM_SEPARATOR, PWM_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectPwmLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectDutyCycle(duty_cycle)) {
        snprintf(response->data, response->size, "%d: Incorrect duty cycle\n", led);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Set_Brightness, .data = NULL};
    sLedSetBrightness_t *task_data = Heap_API_Calloc(1, sizeof(sLedSetBrightness_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led_pin = led;
    task_data->duty_cycle = duty_cycle;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

bool CLI_APP_Pwm_Led_Handlers_Pulse (sMessage_t arguments, sMessage_t *response) {
    if (response == NULL) {
        TRACE_ERR("Invalid data pointer\n");

        return false;
    }

    if ((response->data == NULL)) {
        TRACE_ERR("Invalid response data pointer\n");

        return false;
    }

    eLedPwmPin_t led;
    size_t led_value = 0;
    size_t pulse_time = 0;
    size_t pulse_frequency = 0;

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &led_value, PWM_SEPARATOR, PWM_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }
    
    if (CMD_API_Helper_FindNextArgUInt(&arguments, &pulse_time, PWM_SEPARATOR, PWM_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (CMD_API_Helper_FindNextArgUInt(&arguments, &pulse_frequency, PWM_SEPARATOR, PWM_SEPARATOR_LENGHT, response) != eErrorCode_OSOK) {
        return false;
    }

    if (arguments.size != 0) {
        snprintf(response->data, response->size, "Too many arguments\n");

        return false;
    }

    led = led_value;

    if (!LED_API_IsCorrectPwmLed(led)) {
        snprintf(response->data, response->size, "%d: Incorrect led\n", led);

        return false;
    }

    if (!LED_API_IsCorrectPulseTime(pulse_time)) {
        snprintf(response->data, response->size, "%d: Incorrect pulse time\n", led);

        return false;
    }

    if (!LED_API_IsCorrectPulseFrequency(pulse_frequency)) {
        snprintf(response->data, response->size, "%d: Incorrect pulse frequency\n", led);

        return false;
    }

    sLedCommandDesc_t formated_task = {.task = eLedTask_Pulse, .data = NULL};
    sLedPulse_t *task_data = Heap_API_Calloc(1, sizeof(sLedPulse_t));

    if (task_data == NULL) {
        snprintf(response->data, response->size, "Failed Calloc\n");
        
        return false;
    }

    task_data->led_pin = led;
    task_data->pulse_time = pulse_time;
    task_data->pulse_frequency = pulse_frequency;
    formated_task.data = task_data;

    if (!LED_APP_Add_Task(&formated_task)) {
        snprintf(response->data, response->size, "Failed task add\n");
        
        Heap_API_Free(task_data);

        return false;
    }

    snprintf(response->data, response->size, "Operation successful\n");

    return true;
}

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
