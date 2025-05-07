#ifndef SOURCE_APP_MOTOR_APP_H_
#define SOURCE_APP_MOTOR_APP_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include "motor_api.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eMotorTask {
    eMotorTask_First,
    eMotorTask_Set = eMotorTask_First,
    eMotorTask_Stop,
    eMotorTask_Last
} eMotorTask_t;

typedef struct sMotorCommandDesc {
    eMotorTask_t task;
    void *data;
} sMotorCommandDesc_t;

typedef struct sMotorSet {
    uint8_t speed;
    eMotorDirection_t direction;
} sMotorSet_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Motor_APP_Init (void);
bool Motor_APP_Add_Task (sMotorCommandDesc_t *task_to_message_queue);

#endif /* SOURCE_APP_MOTOR_APP_H_ */
