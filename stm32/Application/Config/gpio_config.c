/**********************************************************************************************************************
 * Includes
 *********************************************************************************************************************/

#include "gpio_config.h"

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
const static sGpioDesc_t g_static_gpio_lut[eGpio_Last] = {
    [eGpio_DebugTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_2,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_DebugRx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_3,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_EspTx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_9,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_EspRx] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_10,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_7
    },
    [eGpio_I2c1_SCL] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_8,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_OPENDRAIN,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_4
    },
    [eGpio_I2c1_SDA] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_9,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_OPENDRAIN,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_4
    },
    [eGpio_OnboardLed] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_VERY_HIGH,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_StartButton] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_UP,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_MotorA_Pwm] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_0,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_MotorA_In1] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_MotorA_In2] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_2,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_MotorB_Pwm] = {
        .port = GPIOB,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_ALTERNATE,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOB,
        .alternate = LL_GPIO_AF_2
    },
    [eGpio_MotorB_In1] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_11,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_MotorB_In2] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_12,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Tcrt5000_Right] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_1,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_UP,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Tcrt5000_Left] = {
        .port = GPIOA,
        .pin = LL_GPIO_PIN_6,
        .mode = LL_GPIO_MODE_INPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_UP,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOA,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Vl53l0_Xshut_Front] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_5,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Vl53l0_Xshut_Right] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_2,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
    [eGpio_Vl53l0_Xshut_Left] = {
        .port = GPIOC,
        .pin = LL_GPIO_PIN_4,
        .mode = LL_GPIO_MODE_OUTPUT,
        .speed = LL_GPIO_SPEED_FREQ_LOW,
        .pull = LL_GPIO_PULL_NO,
        .output = LL_GPIO_OUTPUT_PUSHPULL,
        .clock = LL_AHB1_GRP1_PERIPH_GPIOC,
        .alternate = LL_GPIO_AF_0
    },
};
/* clang-format on */

/**********************************************************************************************************************
 * Private variables
 *********************************************************************************************************************/

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

bool GPIO_Config_IsCorrectGpio (const eGpio_t gpio) {
    return (gpio >= eGpio_First) && (gpio < eGpio_Last);
}

const sGpioDesc_t *GPIO_Config_GetGpioDesc (const eGpio_t gpio_pin) {
    if (!GPIO_Config_IsCorrectGpio(gpio_pin)) {
        return NULL;
    }

    return &g_static_gpio_lut[gpio_pin];
}
