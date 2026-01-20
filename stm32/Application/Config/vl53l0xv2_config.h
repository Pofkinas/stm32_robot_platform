#ifndef CONFIG_VL53L0XV2_CONFIG_H_
#define CONFIG_VL53L0XV2_CONFIG_H_
/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "framework_config.h"

#include <stdbool.h>
#include <stdint.h>
#include "i2c_config.h"
#include "vl53l0x_api.h"
#include "gpio_config.h"

/**********************************************************************************************************************
 * Exported definitions and macros
 *********************************************************************************************************************/

#define VL53L0X_DEFAULT_ADDRESS 0x29

#define DEFAULT_STOP_TIMEOUT_MS 100
/// Calibration distances (mm)
#define DEFAULT_OFFSET_CALIB_DISTANCE_MM 100
#define DEFAULT_CROSSTALK_CALIB_DISTANCE_MM 200
#define FIX1616_FROM_INT(x) ((FixPoint1616_t)((uint32_t)(x) << 16))
#define FIX1616_FROM_FLOAT(x)  ((FixPoint1616_t)((float)(x) * 65536.0f + 0.5f))
#define FIX1616_TO_INT(x) ((int32_t)((x) >> 16))

#define MIN_DEFAULT_TIMEOUT 35 // Standard timing budget 30ms + overhead (max range: < 1.2 m (white target))
#define MIN_HIGH_ACCURACY_TIMEOUT 210 // Precise measurement timing budget 200ms + overhead (max range: < 1.2 m (white target))
#define MIN_LONG_RANGE_TIMEOUT 40 // Long range timing budget 33ms + overhead (max range: < 2 m (white target))
#define MIN_HIGH_SPEED_TIMEOUT 25 // High speed timing budget 20ms + overhead (max range: < 1.2 m (white target))

#define CUSTOM_RANGE_PROFILE_SIGNAL_RATE 0.1f
#define CUSTOM_RANGE_PROFILE_SIGMA 60
#define CUSTOM_RANGE_PROFILE_MEASUREMENT_TIME 25000

/**********************************************************************************************************************
 * Exported types
 *********************************************************************************************************************/

typedef enum eVl53l0x {
    eVl53l0x_First = 0,
    eVl53l0x_Front = eVl53l0x_First,
    eVl53l0x_Right,
    eVl53l0x_Left,
    eVl53l0x_Last
} eVl53l0x_t;

typedef enum eVl53l0xRangeProfile {
    eVl53l0xRangeProfile_First = 0,
    eVl53l0xRangeProfile_Default = eVl53l0xRangeProfile_First,
    eVl53l0xRangeProfile_HighAccuracy,
    eVl53l0xRangeProfile_LongRange,
    eVl53l0xRangeProfile_HighSpeed,
    eVl53l0xRangeProfile_Custom,
    eVl53l0xRangeProfile_Last
} eVl53l0xRangeProfile_t;

typedef enum eVl53l0xState {
    eVl53l0xState_First = 0,
    eVl53l0xState_Off = eVl53l0xState_First,
    eVl53l0xState_HwStandby,
    eVl53l0xState_Init,
    eVl53l0xState_SwStandby,
    eVl53l0xState_Measuring,
    eVl53l0xState_Last
} eVl53l0xState_t;

typedef struct sVl53l0xStaticDesc {
    eI2c_t i2c;
    uint8_t i2c_address;
    uint8_t crosstalk_compensation_en;
    FixPoint1616_t crosstalk_distance;
    VL53L0X_DeviceModes device_mode;
    bool has_xshut_pin;
    eGpio_t xshut_pin;
    eVl53l0xRangeProfile_t range_profile;
} sVl53l0xStaticDesc_t;

typedef struct sVl53l0xDynamicDesc {
    VL53L0X_Dev_t device;
    eVl53l0xState_t state;
    bool has_calib_data;
    uint32_t calib_SpadCount;
	uint8_t calib_isApertureSpads;
    uint8_t calib_VhvSettings;
	uint8_t calib_PhaseCal;
    int32_t offset;
    FixPoint1616_t crosstalk_value;
} sVl53l0xDynamicDesc_t;

/**********************************************************************************************************************
 * Exported variables
 *********************************************************************************************************************/

/**********************************************************************************************************************
 * Prototypes of exported functions
 *********************************************************************************************************************/

bool VL53L0XV2_Config_IsCorrectVl53l0x (const eVl53l0x_t vl53l0x_device);
const sVl53l0xStaticDesc_t *VL53L0XV2_Config_GetVl53l0xStaticDesc (const eVl53l0x_t vl53l0x_device);
const sVl53l0xDynamicDesc_t *VL53L0XV2_Config_GetVl53l0xDynamicDesc (const eVl53l0x_t vl53l0x_device);

#endif /* CONFIG_VL53L0XV2_CONFIG_H_ */
