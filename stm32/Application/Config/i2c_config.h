#ifndef CONFIG_I2C_CONFIG_H_
#define CONFIG_I2C_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "cmsis_os2.h"
#include "platform_config.h"

#include I2C_DRIVER
#include BUS_DRIVER

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define I2C_WRITE 0
#define I2C_READ  1

#define GENERAL_CALL_ADDRESS 0x7F

#define MUTEX_TIMEOUT 0U
#define BUS_RESET_TIMEOUT 1U

#define I2C_1_OWN_ADDRESS1 0x00U
#define I2C_1_OWN_ADDRESS2 0x00U

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eI2c {
    eI2c_First = 0,
    eI2c_1 = eI2c_First,
    eI2c_Last
} eI2c_t;

typedef struct sI2cDesc {
    I2C_TypeDef *periph;
    uint32_t peripheral_mode;
    uint32_t clock_speed;
    uint32_t duty_cycle;
    uint32_t analog_filter;
    uint32_t digital_filter;
    uint32_t own_address1;
    uint32_t own_address2;
    uint32_t type_acknowledge;
    uint32_t own_addr_size;
    uint32_t clock;
    void (*enable_clock_fp) (uint32_t);
    bool is_enabled_it;
    IRQn_Type nvic;
} sI2cDesc_t;

typedef struct sI2cOsDesc {
    osEventFlagsAttr_t flag_attributes;
    osMutexAttr_t mutex_attributes;
} sI2cOsDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool I2C_Config_IsCorrectI2c (const eI2c_t i2c);
sI2cDesc_t const *I2C_Config_GetI2cDesc (const eI2c_t i2c);
sI2cOsDesc_t const *I2C_Config_GetI2cOsDesc (const eI2c_t i2c);

#endif /* CONFIG_I2C_CONFIG_H_ */
