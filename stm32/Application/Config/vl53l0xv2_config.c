/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "vl53l0xv2_config.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

/* clang-format off */
static const sVl53l0xStaticDesc_t g_static_vl53l0x_lut[eVl53l0x_Last] = {
    [eVl53l0x_Front] = {
        .i2c = I2C_1,
        .i2c_address = VL53L0X_FRONT_I2C_ADDRESS,
        .crosstalk_compensation_en = 0,
        .crosstalk_distance = FIX1616_FROM_INT(DEFAULT_CROSSTALK_CALIB_DISTANCE_MM),
        .device_mode = VL53L0X_DEVICEMODE_SINGLE_RANGING,
        .has_xshut_pin = true,
        .xshut_pin = eGpio_Vl53l0_Xshut_Front,
        .range_profile = eVl53l0xRangeProfile_Custom
    },
    [eVl53l0x_Right] = {
        .i2c = I2C_1,
        .i2c_address = VL53L0X_RIGHT_I2C_ADDRESS,
        .crosstalk_compensation_en = 0,
        .crosstalk_distance = FIX1616_FROM_INT(DEFAULT_CROSSTALK_CALIB_DISTANCE_MM),
        .device_mode = VL53L0X_DEVICEMODE_SINGLE_RANGING,
        .has_xshut_pin = true,
        .xshut_pin = eGpio_Vl53l0_Xshut_Right,
        .range_profile = eVl53l0xRangeProfile_Custom
    },
    [eVl53l0x_Left] = {
        .i2c = I2C_1,
        .i2c_address = VL53L0X_LEFT_I2C_ADDRESS,
        .crosstalk_compensation_en = 0,
        .crosstalk_distance = FIX1616_FROM_INT(DEFAULT_CROSSTALK_CALIB_DISTANCE_MM),
        .device_mode = VL53L0X_DEVICEMODE_SINGLE_RANGING,
        .has_xshut_pin = true,
        .xshut_pin = eGpio_Vl53l0_Xshut_Left,
        .range_profile = eVl53l0xRangeProfile_Custom
    }    
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

/* clang-format off */
static sVl53l0xDynamicDesc_t g_dynamic_vl53l0x_lut[eVl53l0x_Last] = {
    [eVl53l0x_Front] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = I2C_1_CLOCK_SPEED / 1000},
        .state = eVl53l0xState_Off,
        .has_calib_data = false,
        .calib_SpadCount = 6,
        .calib_isApertureSpads = 0,
        .calib_VhvSettings = 29,
        .calib_PhaseCal = 1,
        .offset = 29000,
    },
    [eVl53l0x_Right] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = I2C_1_CLOCK_SPEED / 1000},
        .state = eVl53l0xState_Off,
        .has_calib_data = false,
        .calib_SpadCount = 3,
        .calib_isApertureSpads = 0,
        .calib_VhvSettings = 29,
        .calib_PhaseCal = 1,
        .offset = 29000,
    },
    [eVl53l0x_Left] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = I2C_1_CLOCK_SPEED / 1000},
        .state = eVl53l0xState_Off,
        .has_calib_data = false,
        .calib_SpadCount = 10,
        .calib_isApertureSpads = 1,
        .calib_VhvSettings = 30,
        .calib_PhaseCal = 1,
        .offset = 33000,
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool VL53L0XV2_Config_IsCorrectVl53l0x (const eVl53l0x_t vl53l0x_device) {
    return (vl53l0x_device >= eVl53l0x_First) && (vl53l0x_device < eVl53l0x_Last);
}

const sVl53l0xStaticDesc_t *VL53L0XV2_Config_GetVl53l0xStaticDesc (const eVl53l0x_t vl53l0x_device) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x_device)) {
        return NULL;
    }

    return &g_static_vl53l0x_lut[vl53l0x_device];
}

const sVl53l0xDynamicDesc_t *VL53L0XV2_Config_GetVl53l0xDynamicDesc (const eVl53l0x_t vl53l0x_device) {
    if (!VL53L0XV2_Config_IsCorrectVl53l0x(vl53l0x_device)) {
        return NULL;
    }

    return &g_dynamic_vl53l0x_lut[vl53l0x_device];
}
