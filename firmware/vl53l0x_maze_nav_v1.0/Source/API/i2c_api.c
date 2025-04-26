/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "i2c_api.h"
#include "cmsis_os.h"
#include "i2c_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

#define SYSTEM_TIMER_DIVIDER 1000000 // timeout in microseconds

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef struct sI2cStaticDesc {
    eI2cDriver_t i2c_driver;
} sI2cStaticDesc_t;

typedef struct sI2cDynamicDesc {
    bool is_init;
} sI2cDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
static const sI2cStaticDesc_t g_static_i2c_lut[eI2c_Last] = {
    [eI2c_1] = {
        .i2c_driver = eI2cDriver_1
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static uint32_t g_timeout_multiplier = 0;

/* clang-format off */
static sI2cDynamicDesc_t g_dynamic_i2c[eI2cDriver_Last] = {
    [eI2cDriver_1] = {
        .is_init = false,
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/
 
static bool I2C_API_IsActiveFlag (const eI2c_t i2c, const eI2cDriver_Flags_t flag, const bool active_state, const uint32_t ticks, const uint32_t timeout);
static bool I2C_API_StartComms (const eI2c_t i2c, const uint8_t device_address, uint32_t *ticks, const uint32_t timeout, const bool is_restart, const uint8_t rw_operation);
static bool I2C_API_ReadByte (const eI2cDriver_t i2c, uint8_t *data, const uint32_t ticks, const uint32_t timeout);
static bool I2C_API_ReadMultiBytes (const eI2cDriver_t i2c, uint8_t *data, const size_t bytes_to_read, const uint32_t ticks, const uint32_t timeout);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

static bool I2C_API_IsActiveFlag (const eI2c_t i2c, const eI2cDriver_Flags_t flag, const bool active_state, const uint32_t ticks, const uint32_t timeout) {
    while (I2C_Driver_CheckFlag(g_static_i2c_lut[i2c].i2c_driver, flag) != active_state) {
        if ((osKernelGetSysTimerCount() - ticks) > timeout) {
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);
            
            if (I2C_Driver_CheckFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Berr)) {
                I2C_Driver_ClearFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Berr);

                I2C_Driver_ResetLine(g_static_i2c_lut[i2c].i2c_driver);
            }

            if (I2C_Driver_CheckFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Busy)) {
                I2C_Driver_ResetLine(g_static_i2c_lut[i2c].i2c_driver);
            }

            return false;
        }
    }

    return true;
}

static bool I2C_API_StartComms (const eI2c_t i2c, const uint8_t device_address, uint32_t *ticks, const uint32_t timeout, const bool is_restart, const uint8_t rw_operation) {
    if (ticks == NULL) {
        return false;
    }
    
    if (*ticks == 0) {
        *ticks = osKernelGetSysTimerCount();
    }
    
    if (!is_restart) {
        if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Busy, false, *ticks, timeout)) {
            return false;
        }
    }

    I2C_Driver_StartComms(g_static_i2c_lut[i2c].i2c_driver);

    if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Sb, true, *ticks, timeout)) {
        return false;
    }

    if (!I2C_Driver_SendDeviceAddress(g_static_i2c_lut[i2c].i2c_driver, device_address, rw_operation)) {
        I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

        return false;
    }

    if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Addr, true, *ticks, timeout)) {
        return false;
    }

    return true;
}

static bool I2C_API_ReadByte (const eI2cDriver_t i2c, uint8_t *data, const uint32_t ticks, const uint32_t timeout) {
    if (data == NULL) {
        return false;
    }
    
    if (!I2C_Driver_Acknowledge(g_static_i2c_lut[i2c].i2c_driver, false)) {
        I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

        return false;
    }

    I2C_Driver_ClearFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Addr);

    if (!I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver)) {
        I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

        return false;
    }

    if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Rxne, true, ticks, timeout)) {
        return false;
    }

    if (!I2C_Driver_ReadByte(g_static_i2c_lut[i2c].i2c_driver, data)) {
        I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

        return false;
    }

    return true;
}

static bool I2C_API_ReadMultiBytes (const eI2cDriver_t i2c, uint8_t *data, const size_t bytes_to_read, const uint32_t ticks, const uint32_t timeout) {
    if (data == NULL) {
        return false;
    }

    size_t remaining_bytes = bytes_to_read;

    if (!I2C_Driver_Acknowledge(g_static_i2c_lut[i2c].i2c_driver, true)) {
        I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

        return false;
    }

    while (remaining_bytes > 0) {
        if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Rxne, true, ticks, timeout)) {
            return false;
        }

        if (!I2C_Driver_ReadByte(g_static_i2c_lut[i2c].i2c_driver, &data[bytes_to_read - remaining_bytes])) {
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

            return false;
        }

        if (!I2C_Driver_Acknowledge(g_static_i2c_lut[i2c].i2c_driver, true)) {
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

            return false;
        }

        if (remaining_bytes == 2) {
            if (!I2C_Driver_Acknowledge(g_static_i2c_lut[i2c].i2c_driver, false)) {
                I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

                return false;
            }

            if (!I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver)) {
                return false;
            }
        }

        remaining_bytes--;
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool I2C_API_Init (const eI2c_t i2c) {
    if ((i2c < eI2c_First) || (i2c >= eI2c_Last)) {
        return false;
    }

    if (g_dynamic_i2c[i2c].is_init) {
        return true;
    }

    if (!I2C_Driver_Init(i2c)) {
        return false;
    }

    g_timeout_multiplier = osKernelGetSysTimerFreq() / SYSTEM_TIMER_DIVIDER;

    g_dynamic_i2c[i2c].is_init = true;

    return true;
}

bool I2C_API_Write (const eI2c_t i2c, const uint8_t device_address, uint8_t *data, const size_t data_size, const uint16_t mem_address, const uint8_t mem_address_size, uint32_t timeout) {
    if (!g_dynamic_i2c[i2c].is_init) {
        return false;
    }

    if (timeout == 0) {
        return false;
    }

    uint32_t ticks = 0;
    timeout = timeout * g_timeout_multiplier;

    if (!I2C_API_StartComms(i2c, device_address, &ticks, timeout, false, I2C_WRITE)) {
        return false;
    }

    I2C_Driver_ClearFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Addr);

    if (data == NULL && data_size == 0) {
        return I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);
    }

    if (mem_address_size != 0) {
        if (!I2C_Driver_SendMemAddress(i2c, mem_address, mem_address_size)) {
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);
            
            return false;
        }
    }

    for (size_t i = 0; i < data_size; i++) {
        if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Txe, true, ticks, timeout)) {
            return false;
        }

        if (!I2C_Driver_SendByte(g_static_i2c_lut[i2c].i2c_driver, data[i])) {
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);
            
            return false;
        }

        if (I2C_Driver_CheckFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Af)) {
            I2C_Driver_ClearFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Af);
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);
            
            return false;
        }
    }

    if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Btf, true, ticks, timeout)) {
        return false;
    }

    return I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);
}

bool I2C_API_Read (const eI2c_t i2c, const uint8_t device_address, uint8_t *data, const size_t bytes_to_read, const uint16_t mem_address, const uint8_t mem_address_size, uint32_t timeout) {
    if (!g_dynamic_i2c[i2c].is_init) {
        return false;
    }

    if (data == NULL || bytes_to_read == 0) {
        return false;
    }

    if (timeout == 0) {
        return false;
    }

    uint32_t ticks = 0;
    timeout = timeout * g_timeout_multiplier;

    if (mem_address_size != 0) {
        if (!I2C_API_StartComms(i2c, device_address, &ticks, timeout, false, I2C_WRITE)) {
            return false;
        }

        I2C_Driver_ClearFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Addr);

        if (!I2C_Driver_SendMemAddress(i2c, mem_address, mem_address_size)) {
            I2C_Driver_StopComms(g_static_i2c_lut[i2c].i2c_driver);

            return false;
        }

        if (!I2C_API_IsActiveFlag(i2c, eI2cDriver_Flags_Btf, true, ticks, timeout)) {
            return false;
        }
    }

    if (!I2C_API_StartComms(i2c, device_address, &ticks, timeout, true, I2C_READ)) {
        return false;
    }

    if (bytes_to_read == 1) {
        return I2C_API_ReadByte(i2c, data, ticks, timeout);
    }

    I2C_Driver_ClearFlag(g_static_i2c_lut[i2c].i2c_driver, eI2cDriver_Flags_Addr);

    return I2C_API_ReadMultiBytes(i2c, data, bytes_to_read, ticks, timeout);
}
