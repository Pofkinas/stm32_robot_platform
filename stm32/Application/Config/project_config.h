#ifndef CONFIG_PROJECT_CONFIG_H_
#define CONFIG_PROJECT_CONFIG_H_

//=============================================================================
// FEATURE FLAGS
//-----------------------------------------------------------------------------
/// Uncomment the flags for the peripherals and modules you need:

/// -- GPIO                    // Enable GPIO functionality
#define ENABLE_GPIO

/// -- TIMER                   // Enable TIMER functionality
#define ENABLE_TIMER

/// -- UART                    // Enable UART functionality
#define ENABLE_UART

/// -- PWM                     // Enable PWM functionality
#define ENABLE_PWM

/// -- DEBUG UART              // Enable Debug UART functionality
#define ENABLE_UART_DEBUG                   

/// -- CLI                     // Enable Command Line Interface (CLI) over UART
// #define ENABLE_CLI

/// -- CMD                     // Enable Command API modules
#define ENABLE_CMD
#define ENABLE_CMD_HELPER
#define ENABLE_DEFAULT_CMD
#define ENABLE_CUSTOM_CMD

/// -- LEDs                    // Enable LED functionality
#define ENABLE_LED
// #define ENABLE_PWM_LED

/// -- I/Os                    // Enable Input/Output functionality
#define ENABLE_IO

/// -- EXTI                    // Enable EXTI functionality
#define ENABLE_EXTI

/// -- DMA                     // Enable DMA functionality
// #define ENABLE_DMA

/// -- I²C bus                 // Enable I2C functionality
#define ENABLE_I2C

/// -- WS2812B LED strips      // Enable WS2812B LED strip functionality
// #define ENABLE_WS2812B

/// -- LED animation           // Enable LED animation functionality
// #define ENABLE_LED_ANIMATION

/// -- Time-of-flight sensors  // Enable VL53L0X sensor
#define ENABLE_VL53L0X         

/// -- Motors                  // Enable Motor functionality
#define ENABLE_MOTOR
#define ENABLE_ODOMETRY

/// -- LCD                     // Enable LCD functionality
// #define ENABLE_LCD

//=============================================================================
// SYSTEM CONFIGURATION
//-----------------------------------------------------------------------------
/// System clock frequency (Hz)

#define SYSTEM_CLOCK_HZ 100000000UL

//=============================================================================
// UART CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_UART
#define UART1 eUart_Esp
#define UART_1_BAUDRATE eBaudrate_115200

#define UART2 eUart_Debug
#define UART_2_BAUDRATE eBaudrate_115200

#ifdef ENABLE_UART_DEBUG
// #define DEBUG_UART UART2
#define DEBUG_UART UART1

// #define DEBUG_DELIMITER "\r\n"
#define DEBUG_DELIMITER "\n"
#define DEBUG_MESSAGE_SIZE 256
#define UART_DEBUG_BUFFER_CAPACITY 256

#define DEBUG_MESSAGE_TIMEOUT 1000
#define DEBUG_MUTEX_TIMEOUT 0U
#endif /* ENABLE_UART_DEBUG */

#define MESSAGE_QUEUE_PRIORITY 0U
#define MESSAGE_QUEUE_CAPACITY 10
#define MESSAGE_QUEUE_PUT_TIMEOUT 0U
#endif /* ENABLE_UART */

//=============================================================================
// LED CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_LED
/// Blink Maximum time (s)
#define MAX_BLINK_TIME 59
/// Blink frequency limits (Hz)
#define MIN_BLINK_FREQUENCY 2
#define MAX_BLINK_FREQUENCY 100

#define LED_COMMAND_MESSAGE_CAPACITY 10
#define LED_APP_MESSAGE_QUEUE_PRIORITY 0U
#define LED_APP_MESSAGE_QUEUE_TIMEOUT osWaitForever
#endif /* ENABLE_LED */

#ifdef ENABLE_PWM_LED
// Pulsing Maximum time (s)
#define MAX_PULSING_TIME 59
// Pulsing frequency limits (Hz)
#define MAX_PULSE_FREQUENCY 500
#define MAX_DUTY_CYCLE 65535
#define MIN_PULSE_FREQUENCY (MAX_DUTY_CYCLE / 1000)
#endif /* ENABLE_PWM_LED */

//=============================================================================
// EXTI CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_EXTI
#define EXTI0 eExti_StartButton
#define EXTI1 eExti_Tcrt5000_Right
// #define EXTI2
// #define EXTI3
// #define EXTI4
#define EXTI9_5 eExti_Tcrt5000_Left
// #define EXTI15_10
#endif /* ENABLE_EXTI */

//=============================================================================
// I²C BUS CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_I2C
#define I2C_1 eI2c_1
// #define I2C_2

#define I2C_MAX_DATA_SIZE 16

#ifdef I2C_1
#define I2C_1_CLOCK_SPEED 100000U

#define VL53L0X_FRONT_I2C_ADDRESS 0x30
#define VL53L0X_RIGHT_I2C_ADDRESS 0x31
#define VL53L0X_LEFT_I2C_ADDRESS 0x32
#endif /* I2C_1 */

#ifdef I2C_2
#endif /* I2C_2 */
#endif /* ENABLE_I2C */

//=============================================================================
// DMA CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_DMA
// #define DMA_1_STREAM_0
// #define DMA_1_STREAM_1
// #define DMA_1_STREAM_2
// #define DMA_1_STREAM_3
// #define DMA_1_STREAM_4
// #define DMA_1_STREAM_5
// #define DMA_1_STREAM_6
// #define DMA_1_STREAM_7

// #define DMA_2_STREAM_0
// #define DMA_2_STREAM_1
// #define DMA_2_STREAM_2
// #define DMA_2_STREAM_3
// #define DMA_2_STREAM_4
// #define DMA_2_STREAM_5
// #define DMA_2_STREAM_6
// #define DMA_2_STREAM_7
#endif /* ENABLE_DMA */

//=============================================================================
// VL53L0X CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_VL53L0X
#define VL53L0X_I2C_PHERIPH I2C_1
#define NEXT_MEASUREMENT_POLL_DELAY 10U
#endif /* ENABLE_VL53L0X */

//=============================================================================
// DEBUG
//-----------------------------------------------------------------------------

#ifdef ENABLE_UART_DEBUG
// Custom debug flags
#define DEBUG_MAIN
#define DEBUG_MISSION_APP
#define DEBUG_ESP_COMM_APP
// #define DEBUG_ESP_PROTOCOL

#define DEBUG_ACTION_DRIVE_DISTANCE
#define DEBUG_ACTION_MEASURE
// #define DEBUG_ACTION_MEASURE_DATA
#define DEBUG_ACTION_WALL_FOLLOW
#define DEBUG_ACTION_WALL_FOLLOW_DATA

// APP layer debug flags
#define DEBUG_CLI_APP
#define DEBUG_DEFAULT_CMD
#define DEBUG_CUSTOM_CMD
#define DEBUG_LED_APP
#define DEBUG_MOTOR_APP

// API layer debug flags
#define DEBUG_CMD_API
#define DEBUG_CMD_API_HELPER
#define DEBUG_UART_API
#define DEBUG_I2C_API
#define DEBUG_IO_API
#define DEBUG_LCD_API
#define DEBUG_LED_API
#define DEBUG_MOTOR_API
#ifdef ENABLE_ODOMETRY
#define DEBUG_ODOMETRY_API
#endif /* ENABLE_ODOMETRY */
// #define DEBUG_VL53L0X_API
// #define DEBUG_VL53L0X_RANGE_STATUS
// #define DEBUG_VL53L0X_DETAILS
#define DEBUG_WS2812B_API
#endif /* ENABLE_UART_DEBUG */

//=============================================================================
// MOTOR CONFIGURATION
//-----------------------------------------------------------------------------

#ifdef ENABLE_MOTOR
// #define USE_MX1508
#define USE_TB6612FNG

#define MOTOR_MESSAGE_QUEUE_CAPACITY 10
#define MOTOR_MESSAGE_QUEUE_PRIORITY 0U
#define MOTOR_MESSAGE_QUEUE_TIMEOUT osWaitForever

#define ENABLE_PID_CONTROL
#endif /* ENABLE_MOTOR */

#ifdef ENABLE_ODOMETRY
#define ODOMETRY_MUTEX_TIMEOUT 0U
#endif /* ENABLE_ODOMETRY */

//=============================================================================
// MISCELLANEOUS
//-----------------------------------------------------------------------------

#ifdef ENABLE_CLI
#define CLI_APP_THREAD_STACK_SIZE (256 * 6)
#define CLI_APP_THREAD_PRIORITY osPriorityNormal

#define CLI_COMMAND_MESSAGE_CAPACITY 20
#define RESPONSE_MESSAGE_CAPACITY 128
#endif /* ENABLE_CLI */

#if defined(ENABLE_DEFAULT_CMD) || defined(ENABLE_CUSTOM_CMD)
#define CMD_SEPARATOR ","
#define CMD_SEPARATOR_LENGTH (sizeof(CMD_SEPARATOR) - 1)
#endif /* ENABLE_DEFAULT_CMD || ENABLE_CUSTOM_CMD */

#define HEAP_API_MUTEX_TIMEOUT 0U

#define BYTE 8
#define BASE_10 10
#define MAX_PID_DT 0.5f  // Maximum dt for PID update to avoid large jumps

//=============================================================================
// CUSTOM FLAGS
//-----------------------------------------------------------------------------
//=============================================================================
// ESP COMM
//-----------------------------------------------------------------------------

#define ESP_COMM_UART UART1
#define ESP_COMM_BAUDRATE UART_1_BAUDRATE

#define ESP_COMM_ARG_SEPARATOR CMD_SEPARATOR
#define ESP_COMM_DELIMITER "\n"
#define ESP_COMM_MESSAGE_SIZE 256
#define ESP_COMM_BUFFER_CAPACITY 256

//=============================================================================
// ROBOT CONFIGURATION
//-----------------------------------------------------------------------------

#define MISSION_APP_UPDATE_RATE 50U

#define WHEEL_DIAMETER_MM 37.5f
#define WHEEL_CIRCUMFERENCE_MM 117.81f
#define WHEEL_BASE_MM 84.5f

#define FRONT_DISTANCE_SENSOR eVl53l0x_Front
#define RIGHT_DISTANCE_SENSOR eVl53l0x_Right
#define LEFT_DISTANCE_SENSOR eVl53l0x_Left

#endif /* CONFIG_PROJECT_CONFIG_H_ */
