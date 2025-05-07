/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "vl53l0xv2_api.h"
#include "cmsis_os2.h"
#include "vl53l0x_api.h"
#include "i2c_api.h"
#include "debug_api.h"
#include "gpio_driver.h"

/**********************************************************************************************************************
 * Private definitions and macros
 *********************************************************************************************************************/

//#define DEBUG_VL53L0X_API

#define VL53L0X_DEFAULT_ADDRESS 0x29
#define SYSTEM_TIMER_DIVIDER 1000 // timeout in miliseconds

/**********************************************************************************************************************
 * Private typedef
 *********************************************************************************************************************/

typedef enum eVl53l0xRangeProfile {
    eVl53l0xRangeProfile_First = 0,
    eVl53l0xRangeProfile_Default = eVl53l0xRangeProfile_First,
    eVl53l0xRangeProfile_HighAccuracy,
    eVl53l0xRangeProfile_LongRange,
    eVl53l0xRangeProfile_HighSpeed,
    eVl53l0xRangeProfile_Last
} eVl53l0xRangeProfile_t;


typedef struct sVl53l0xStaticDesc {
    eI2c_t i2c;
    uint8_t i2c_address;
    int32_t offset;
    uint8_t crosstalk_talk_compensation_en;
    uint32_t crosstalk_value;
    VL53L0X_DeviceModes device_mode; 
    eGpioPin_t xshut_pin;
    eVl53l0xRangeProfile_t range_profile;
} sVl53l0xStaticDesc_t;

typedef struct sVl53l0xDynamicDesc {
    VL53L0X_Dev_t device;
    bool is_init;
    bool is_enabled;
    bool is_calib_default_data;
    uint32_t calib_SpadCount;
	uint8_t calib_isApertureSpads;
    uint8_t calib_VhvSettings;
	uint8_t calib_PhaseCal;
} sVl53l0xDynamicDesc_t;

/**********************************************************************************************************************
 * Private constants
 *********************************************************************************************************************/

#ifdef DEBUG_VL53L0X_API 
CREATE_MODULE_NAME (VL53L0XV2_API)
#else
CREATE_MODULE_NAME_EMPTY
#endif

/* clang-format off */
static const sVl53l0xStaticDesc_t g_static_vl53l0x_lut[eVl53l0x_Last] = {
    [eVl53l0x_1] = {
        .i2c = eI2c_1,
        .i2c_address = 0x29,
        .offset = 0,
        .crosstalk_talk_compensation_en = 0,
        .crosstalk_value = 0.0f * 65536,
        .device_mode = VL53L0X_DEVICEMODE_CONTINUOUS_RANGING,
        .xshut_pin = eGpioPin_vl53l0_Xshut_1,
        .range_profile = eVl53l0xRangeProfile_Default
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

static uint32_t g_timeout_multiplier = 0;

/* clang-format off */
static sVl53l0xDynamicDesc_t g_dynamic_vl53l0x[eVl53l0x_Last] = {
    [eVl53l0x_1] = {
        .device = {.I2cDevAddr = VL53L0X_DEFAULT_ADDRESS, .comms_type = I2C, .comms_speed_khz = 100},
        .is_init = false,
        .is_enabled = false,
        .is_calib_default_data = false,
        .calib_SpadCount = 0,
        .calib_isApertureSpads = 0,
        .calib_VhvSettings = 0,
        .calib_PhaseCal = 0
    }
};
/* clang-format on */

/**********************************************************************************************************************
 * Exported variables and references
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of private functions
 *********************************************************************************************************************/

static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile);

/**********************************************************************************************************************
 * Definitions of private functions
 *********************************************************************************************************************/
 
static bool VL53L0X_API_SetRangeProfile (const eVl53l0x_t vl53l0x, const eVl53l0xRangeProfile_t profile) {
    if ((profile < eVl53l0xRangeProfile_First) || (profile >= eVl53l0xRangeProfile_Last)) {
        return false;
    }

    uint8_t signal_rate_multiplyer = 0;
    uint8_t sigma_multiplyer = 0;
    uint32_t measurement_time = 0;

    switch (profile) {
        case eVl53l0xRangeProfile_Default: {
            signal_rate_multiplyer = 0.25;
            sigma_multiplyer = 18;
            measurement_time = 33000;
            return true;
        }
        case eVl53l0xRangeProfile_HighAccuracy: {
            signal_rate_multiplyer = 0.25;
            sigma_multiplyer = 18;
            measurement_time = 200000;
        } break;
        case eVl53l0xRangeProfile_LongRange: {
            signal_rate_multiplyer = 0.1;
            sigma_multiplyer = 60;
            measurement_time = 33000;
        } break;
        case eVl53l0xRangeProfile_HighSpeed: {
            signal_rate_multiplyer = 0.25;
            sigma_multiplyer = 32;
            measurement_time = 20000;
        } break;
        default: {
            return false;
        }
    }

    if (VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, (FixPoint1616_t)(signal_rate_multiplyer * 65536)) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (VL53L0X_SetLimitCheckValue(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, (FixPoint1616_t)(sigma_multiplyer * 65536)) != VL53L0X_ERROR_NONE) {
        return false;
    }
    if (VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&g_dynamic_vl53l0x[vl53l0x].device, measurement_time) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (profile == eVl53l0xRangeProfile_LongRange) {
        if (VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18) != VL53L0X_ERROR_NONE) {
            return false;
        }
        if (VL53L0X_SetVcselPulsePeriod(&g_dynamic_vl53l0x[vl53l0x].device, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14) != VL53L0X_ERROR_NONE) {
            return false;
        }
    }

    return true;
}

/**********************************************************************************************************************
 * Definitions of exported functions
 *********************************************************************************************************************/

bool VL53L0X_API_Init (const eVl53l0x_t vl53l0x) {
    if ((vl53l0x < eVl53l0x_First) || (vl53l0x >= eVl53l0x_Last)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].is_init) {
        return true;
    }

    if (!GPIO_Driver_InitAllPins()) {
        return false;
    }

    if (!I2C_API_Init(g_static_vl53l0x_lut[vl53l0x].i2c)) {
        return false;
    }

    if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false)) {
        return false;
    }

    osDelay(2);

    if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, true)) {
        return false;
    }

    osDelay(5);

    if (VL53L0X_DataInit(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (VL53L0X_StaticInit(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        return false;
    }

//    if (VL53L0X_SetDeviceAddress(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].i2c_address) != VL53L0X_ERROR_NONE) {
//        return false;
//    }
//
//    g_dynamic_vl53l0x[vl53l0x].device.I2cDevAddr = g_static_vl53l0x_lut[vl53l0x].i2c_address;

    if (!g_dynamic_vl53l0x[vl53l0x].is_calib_default_data) {
        if (VL53L0X_PerformRefSpadManagement(&g_dynamic_vl53l0x[vl53l0x].device, &g_dynamic_vl53l0x[vl53l0x].calib_SpadCount, &g_dynamic_vl53l0x[vl53l0x].calib_isApertureSpads) != VL53L0X_ERROR_NONE) {
            return false;
        }

        if (VL53L0X_PerformRefCalibration(&g_dynamic_vl53l0x[vl53l0x].device, &g_dynamic_vl53l0x[vl53l0x].calib_VhvSettings, &g_dynamic_vl53l0x[vl53l0x].calib_PhaseCal) != VL53L0X_ERROR_NONE) {
            return false;
        }

        g_dynamic_vl53l0x[vl53l0x].is_calib_default_data = true;
    }


    if (VL53L0X_SetReferenceSpads(&g_dynamic_vl53l0x[vl53l0x].device, g_dynamic_vl53l0x[vl53l0x].calib_SpadCount, g_dynamic_vl53l0x[vl53l0x].calib_isApertureSpads) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (VL53L0X_SetRefCalibration(&g_dynamic_vl53l0x[vl53l0x].device, g_dynamic_vl53l0x[vl53l0x].calib_VhvSettings, g_dynamic_vl53l0x[vl53l0x].calib_PhaseCal) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (VL53L0X_SetOffsetCalibrationDataMicroMeter(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].offset) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) {
        if (VL53L0X_SetXTalkCompensationRateMegaCps(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_value) != VL53L0X_ERROR_NONE) {
            return false;
        }
    
        if (VL53L0X_SetXTalkCompensationEnable(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].crosstalk_talk_compensation_en) != VL53L0X_ERROR_NONE) {
            return false;
        }
    }

    if (VL53L0X_SetDeviceMode(&g_dynamic_vl53l0x[vl53l0x].device, g_static_vl53l0x_lut[vl53l0x].device_mode) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (!VL53L0X_API_SetRangeProfile(vl53l0x, g_static_vl53l0x_lut[vl53l0x].range_profile)) {
        return false;
    }

    g_timeout_multiplier = osKernelGetSysTimerFreq() / SYSTEM_TIMER_DIVIDER;

    g_dynamic_vl53l0x[vl53l0x].is_init = true;

    return true;
}

bool VL53L0X_API_Enable (const eVl53l0x_t vl53l0x) {
    if ((vl53l0x < eVl53l0x_First) || (vl53l0x >= eVl53l0x_Last)) {
        return false;
    }

    if (!VL53L0X_API_Init(vl53l0x)) {
        return false;
    }

    if (g_dynamic_vl53l0x[vl53l0x].is_enabled) {
        return true;
    }

    if (VL53L0X_StartMeasurement(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_StartMeasurement failed\n");
        
        return false;
    }

    g_dynamic_vl53l0x[vl53l0x].is_enabled = true;

    return true;
}

bool VL53L0X_API_Disable (const eVl53l0x_t vl53l0x) {
    if ((vl53l0x < eVl53l0x_First) || (vl53l0x >= eVl53l0x_Last)) {
        return false;
    }

    if (!g_dynamic_vl53l0x[vl53l0x].is_init) {
        return false;
    }

    uint32_t stop_status = 0;

    if (VL53L0X_StopMeasurement(&g_dynamic_vl53l0x[vl53l0x].device) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_StopMeasurement failed\n");
        
        return false;
    }

    if (VL53L0X_GetStopCompletedStatus(&g_dynamic_vl53l0x[vl53l0x].device, &stop_status) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_GetStopCompletedStatus failed\n");
        
        return false;
    }

    if (stop_status != 0) {
        return false;
    }

    if (!GPIO_Driver_WritePin(g_static_vl53l0x_lut[vl53l0x].xshut_pin, false)) {
        return false;
    }

    return true;
}

bool VL53L0X_API_GetDistance (const eVl53l0x_t vl53l0x, uint16_t *distance, size_t timeout) {
    if ((vl53l0x < eVl53l0x_First) || (vl53l0x >= eVl53l0x_Last)) {
        return false;
    }

    if (distance == NULL) {
        return false;
    }

    if (!g_dynamic_vl53l0x[vl53l0x].is_enabled) {
        return false;
    }

    uint8_t data_status = 0;
    uint32_t ticks = 0;
    VL53L0X_RangingMeasurementData_t ranging_data = {0};

    timeout = timeout * g_timeout_multiplier;
    ticks = osKernelGetSysTimerCount();

    while (VL53L0X_GetMeasurementDataReady(&g_dynamic_vl53l0x[vl53l0x].device, &data_status) != VL53L0X_ERROR_NONE) {
        if ((osKernelGetSysTimerCount() - ticks) > timeout) {
            TRACE_ERR("VL53L0X_GetMeasurementDataReady timeout\n");
            
            return false;
        }
    }

    if (data_status == 0) {
        return false;
    }

    if (VL53L0X_GetRangingMeasurementData(&g_dynamic_vl53l0x[vl53l0x].device, &ranging_data) != VL53L0X_ERROR_NONE) {
        TRACE_ERR("VL53L0X_GetRangingMeasurementData failed\n");
        
        return false;
    }

    if (VL53L0X_ClearInterruptMask(&g_dynamic_vl53l0x[vl53l0x].device, 0) != VL53L0X_ERROR_NONE) {
        return false;
    }

    if (ranging_data.RangeStatus != 0) {
        TRACE_ERR("Incrored ranging data\n");
        
        return false;
    }
    
    *distance = ranging_data.RangeMilliMeter;

    return true;
}
