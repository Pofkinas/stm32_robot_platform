#ifndef APPLICATION_CONFIG_ODOMETRY_CONFIG_H_
#define APPLICATION_CONFIG_ODOMETRY_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include "motor_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define ODOMETRY_UPDATE_RATE_MS 100

#define ODOMETRY_DATA_STALE_THRESHOLD 5
#define ODOMETRY_DATA_FAULT_THRESHOLD 10

#define ODOMETRY_DATA_STALE_FLAG 0x01
#define ODOMETRY_DATA_FAULT_FLAG 0x02
#define ODOMETRY_NEW_DATA_READY_FLAG 0x04
#define ODOMETRY_DATA_READY_FLAG_TIMEOUT 500  // cli 500 ms timeout

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eEncoder_t {
    eEncoder_First = 0,
    eEncoder_Right_1 = eEncoder_First,
    eEncoder_Left_1,
    eEncoder_Last
} eEncoder_t;

typedef enum eEncoderSide {
    eEncoderSide_First = 0,
    eEncoderSide_Right = eEncoderSide_First,
    eEncoderSide_Left,
    eEncoderSide_Last
} eEncoderSide_t;

typedef struct sEncoderDesc {
    eMotor_t motor;
    eEncoderSide_t side;
    eMotorRotation_t positive_dir;
} sEncoderDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

extern const osTimerAttr_t g_odometry_timer_attributes;
extern const osMutexAttr_t g_odometry_mutex_attributes;

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool Odometry_Config_IsCorrectEncoder (const eEncoder_t encoder);
const sEncoderDesc_t *Odometry_Config_GetEncoderDesc (const eEncoder_t encoder);

#endif /* APPLICATION_CONFIG_ODOMETRY_CONFIG_H_ */
