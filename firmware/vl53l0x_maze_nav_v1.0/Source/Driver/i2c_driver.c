/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "i2c_driver.h"
#include "stm32f4xx_ll_bus.h"
#include "stm32f4xx_ll_i2c.h"
#include "stm32f4xx_ll_gpio.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

 #define RESET_SCL_PIN_PULSES 9

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

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
    eGpioPin_t scl_pin;
    eGpioPin_t sda_pin;
} sI2cDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
const static sI2cDesc_t g_static_i2c_lut[eI2cDriver_Last] = {
    [eI2cDriver_1] = {
        .periph = I2C1,
        .peripheral_mode = LL_I2C_MODE_I2C,
        .clock_speed = 100000,
        .duty_cycle = LL_I2C_DUTYCYCLE_2,
        .own_address1 = 0,
        .own_address2 = 0,
        .type_acknowledge = LL_I2C_ACK,
        .own_addr_size = LL_I2C_OWNADDRESS1_7BIT,
        .clock = LL_APB1_GRP1_PERIPH_I2C1,
        .enable_clock_fp = LL_APB1_GRP1_EnableClock,
        .is_enabled_it = true,
        .nvic = I2C1_EV_IRQn,
        .scl_pin = eGpioPin_I2c1_SCL,
        .sda_pin = eGpioPin_I2c1_SDA
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static bool g_is_i2c_init[eI2cDriver_Last] = {false};

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

void I2C1_EV_IRQHandler(void) {
    if (LL_I2C_IsActiveFlag_BERR(I2C1)) {
        NVIC_DisableIRQ(I2C1_EV_IRQn);

        LL_I2C_ClearFlag_BERR(I2C1);
        I2C_Driver_ResetLine(eI2cDriver_1);

        NVIC_EnableIRQ(I2C1_EV_IRQn);
    }

    return;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool I2C_Driver_Init (const eI2cDriver_t i2c) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }
    
    if (g_is_i2c_init[i2c]){ 
        return true;
    }

    LL_I2C_InitTypeDef i2c_init_struct = {0};

    i2c_init_struct.PeripheralMode = g_static_i2c_lut[i2c].peripheral_mode;
    i2c_init_struct.ClockSpeed = g_static_i2c_lut[i2c].clock_speed;
    i2c_init_struct.DutyCycle = g_static_i2c_lut[i2c].duty_cycle;
    i2c_init_struct.OwnAddress1 = g_static_i2c_lut[i2c].own_address1;
    i2c_init_struct.TypeAcknowledge = g_static_i2c_lut[i2c].type_acknowledge;
    i2c_init_struct.OwnAddrSize = g_static_i2c_lut[i2c].own_addr_size;

    g_static_i2c_lut[i2c].enable_clock_fp(g_static_i2c_lut[i2c].clock);

    if (LL_I2C_Init(g_static_i2c_lut[i2c].periph, &i2c_init_struct) == ERROR) {
        return false;
    }

    LL_I2C_Disable(g_static_i2c_lut[i2c].periph);

    LL_I2C_DisableOwnAddress2(g_static_i2c_lut[i2c].periph);
    LL_I2C_DisableGeneralCall(g_static_i2c_lut[i2c].periph);
    LL_I2C_EnableClockStretching(g_static_i2c_lut[i2c].periph);

    if (g_static_i2c_lut[i2c].is_enabled_it) {
        NVIC_SetPriority(g_static_i2c_lut[i2c].nvic, NVIC_EncodePriority(NVIC_GetPriorityGrouping(),5, 0));
        NVIC_EnableIRQ(g_static_i2c_lut[i2c].nvic);    
    }

    LL_I2C_Enable(g_static_i2c_lut[i2c].periph);

    g_is_i2c_init[i2c] = true;

    return g_is_i2c_init[i2c];
}

bool I2C_Driver_StartComms (const eI2cDriver_t i2c) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    LL_I2C_GenerateStartCondition(g_static_i2c_lut[i2c].periph);

    return true;
}

bool I2C_Driver_StopComms (const eI2cDriver_t i2c) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    LL_I2C_GenerateStopCondition(g_static_i2c_lut[i2c].periph);

    return true;
}

bool I2C_Driver_Acknowledge (const eI2cDriver_t i2c, const bool ack) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    if (ack) {
        LL_I2C_AcknowledgeNextData(g_static_i2c_lut[i2c].periph, LL_I2C_ACK);
    } else {
        LL_I2C_AcknowledgeNextData(g_static_i2c_lut[i2c].periph, LL_I2C_NACK);
    }

    return true;
}

bool I2C_Driver_SendDeviceAddress (const eI2cDriver_t i2c, const uint8_t address, const uint8_t rw_operation) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    if (rw_operation != I2C_WRITE && rw_operation != I2C_READ) {
        return false;
    }
    
    LL_I2C_TransmitData8(g_static_i2c_lut[i2c].periph, (uint8_t)((address << 1) | (rw_operation & 0x1)));

    return true;
}

bool I2C_Driver_SendMemAddress (const eI2cDriver_t i2c, const uint16_t mem_address, const uint8_t mem_address_size) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    if (mem_address_size != 1 && mem_address_size != 2) {
        return false;
    }

    switch (mem_address_size) {
        case 1: {
            LL_I2C_TransmitData8(g_static_i2c_lut[i2c].periph, (uint8_t) mem_address);
        } break;
        case 2: {
            LL_I2C_TransmitData8(g_static_i2c_lut[i2c].periph, (uint8_t) (mem_address >> 8));

            if (!I2C_Driver_CheckFlag(i2c, eI2cDriver_Flags_Txe)) {
                return false;
            }

            LL_I2C_TransmitData8(g_static_i2c_lut[i2c].periph, (uint8_t) (mem_address & 0xFF));
        } break;
        default: {
            return false;
        }
    }

    return true;
}

bool I2C_Driver_SendByte (const eI2cDriver_t i2c, const uint8_t data) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    LL_I2C_TransmitData8(g_static_i2c_lut[i2c].periph, data);

    return true;
}

bool I2C_Driver_ReadByte (const eI2cDriver_t i2c, uint8_t *data) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    if (data == NULL) {
        return false;
    }

    *data = LL_I2C_ReceiveData8(g_static_i2c_lut[i2c].periph);

    return true;
}

bool I2C_Driver_ReadByteAck (const eI2cDriver_t i2c, uint8_t *data, const bool ack) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }

    if (data == NULL) {
        return false;
    }

    *data = LL_I2C_ReceiveData8(g_static_i2c_lut[i2c].periph);

    if (ack) {
        LL_I2C_AcknowledgeNextData(g_static_i2c_lut[i2c].periph, LL_I2C_ACK);
    } else {
        LL_I2C_AcknowledgeNextData(g_static_i2c_lut[i2c].periph, LL_I2C_NACK);
    }

    return true;
}

bool I2C_Driver_CheckFlag (const eI2cDriver_t i2c, const eI2cDriver_Flags_t flag) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return false;
    }

    if (!g_is_i2c_init[i2c]) {
        return false;
    }
    
    uint32_t (*flag_fp) (I2C_TypeDef *i2c) = NULL;
    
    switch (flag) {
        case eI2cDriver_Flags_Busy: {
            flag_fp = LL_I2C_IsActiveFlag_BUSY;
        } break;
        case eI2cDriver_Flags_Addr: {
            flag_fp = LL_I2C_IsActiveFlag_ADDR;
        } break;
        case eI2cDriver_Flags_Txe: {
            flag_fp = LL_I2C_IsActiveFlag_TXE;
        } break;
        case eI2cDriver_Flags_Rxne: {
            flag_fp = LL_I2C_IsActiveFlag_RXNE;
        } break;
        case eI2cDriver_Flags_Sb : {
            flag_fp = LL_I2C_IsActiveFlag_SB;
        } break;
        case eI2cDriver_Flags_Btf: {
            flag_fp = LL_I2C_IsActiveFlag_BTF;
        } break;
        case eI2cDriver_Flags_Af: {
            flag_fp = LL_I2C_IsActiveFlag_AF;
        } break;
        default: {
            return false;
        }
    }

    return flag_fp(g_static_i2c_lut[i2c].periph);
}

void I2C_Driver_ClearFlag (const eI2cDriver_t i2c, const eI2cDriver_Flags_t flag) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return;
    }

    if (!g_is_i2c_init[i2c]) {
        return;
    }
    
    void (*flag_fp) (I2C_TypeDef *i2c) = NULL;

    switch (flag) {
        case eI2cDriver_Flags_Addr: {
            flag_fp = LL_I2C_ClearFlag_ADDR;
        } break;
        case eI2cDriver_Flags_Af: {
            flag_fp = LL_I2C_ClearFlag_AF;
        } break;
        case eI2cDriver_Flags_Berr: {
            flag_fp = LL_I2C_ClearFlag_BERR;
        } break;
        default: {
            return;
        }
    }

    flag_fp(g_static_i2c_lut[i2c].periph);

    return;
}

void I2C_Driver_ResetLine (const eI2cDriver_t i2c) {
    if ((i2c < eI2cDriver_First) || (i2c >= eI2cDriver_Last)) {
        return;
    }

    if (!g_is_i2c_init[i2c]) {
        return;
    }

    GPIO_Driver_SetPinMode(g_static_i2c_lut[i2c].scl_pin, LL_GPIO_MODE_OUTPUT);
    GPIO_Driver_SetPinMode(g_static_i2c_lut[i2c].sda_pin, LL_GPIO_MODE_OUTPUT);

    for (size_t i = 0; i < RESET_SCL_PIN_PULSES; i++) {
        GPIO_Driver_WritePin(g_static_i2c_lut[i2c].scl_pin, true);
        GPIO_Driver_WritePin(g_static_i2c_lut[i2c].scl_pin, false);
    }

    GPIO_Driver_WritePin(g_static_i2c_lut[i2c].sda_pin, false);
    GPIO_Driver_WritePin(g_static_i2c_lut[i2c].scl_pin, true);
    GPIO_Driver_WritePin(g_static_i2c_lut[i2c].sda_pin, true);

    LL_I2C_Disable(g_static_i2c_lut[i2c].periph);
    LL_I2C_EnableReset(g_static_i2c_lut[i2c].periph);
    LL_I2C_DisableReset(g_static_i2c_lut[i2c].periph);

    g_is_i2c_init[i2c] = false;

    if (I2C_Driver_Init(i2c)) {
        g_is_i2c_init[i2c] = true;
    }

    GPIO_Driver_SetPinMode(g_static_i2c_lut[i2c].scl_pin, LL_GPIO_MODE_ALTERNATE);
    GPIO_Driver_SetPinMode(g_static_i2c_lut[i2c].sda_pin, LL_GPIO_MODE_ALTERNATE);

    LL_I2C_Enable(g_static_i2c_lut[i2c].periph);

    return;
}
