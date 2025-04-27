#ifndef SOURCE_DRIVER_I2C_DRIVER_H_
#define SOURCE_DRIVER_I2C_DRIVER_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include <stdbool.h>
#include <stdint.h>

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define I2C_WRITE 0
#define I2C_READ  1

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

/* clang-format off */
typedef enum eI2cDriver {
    eI2cDriver_First = 0,
    eI2cDriver_1 = eI2cDriver_First,
    eI2cDriver_Last
} eI2cDriver_t;

typedef enum eI2cDriver_Flags {
    eI2cDriver_Flags_First = 0,
    eI2cDriver_Flags_Busy = eI2cDriver_Flags_First,
    eI2cDriver_Flags_Addr,
    eI2cDriver_Flags_Txe,
    eI2cDriver_Flags_Rxne,
    eI2cDriver_Flags_Sb,
    eI2cDriver_Flags_Btf,
    eI2cDriver_Flags_Af,
    eI2cDriver_Flags_Berr,
    eI2cDriver_Flags_Last
} eI2cDriver_Flags_t;
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool I2C_Driver_Init (const eI2cDriver_t i2c);
bool I2C_Driver_StartComms (const eI2cDriver_t i2c);
bool I2C_Driver_StopComms (const eI2cDriver_t i2c);
bool I2C_Driver_Acknowledge (const eI2cDriver_t i2c, const bool ack);
bool I2C_Driver_SendDeviceAddress (const eI2cDriver_t i2c, const uint8_t address, const uint8_t rw_operation);
bool I2C_Driver_SendMemAddress (const eI2cDriver_t i2c, const uint16_t mem_address, const uint8_t mem_address_size);
bool I2C_Driver_SendByte (const eI2cDriver_t i2c, const uint8_t data);
bool I2C_Driver_ReadByte (const eI2cDriver_t i2c, uint8_t *data);
bool I2C_Driver_ReadByteAck (const eI2cDriver_t i2c, uint8_t *data, const bool ack);
bool I2C_Driver_CheckFlag (const eI2cDriver_t i2c, const eI2cDriver_Flags_t flag);
void I2C_Driver_ClearFlag (const eI2cDriver_t i2c, const eI2cDriver_Flags_t flag);
void I2C_Driver_ResetLine (const eI2cDriver_t i2c);

#endif /* SOURCE_DRIVER_I2C_DRIVER_H_ */
