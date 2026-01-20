#pragma once

#include <stdint.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
#include "freertos/portmacro.h"
#include "esp_timer.h"
#include "config.h"

#define ENCODER_PPR 1  // Pulses per revolution
#define RPM_AVERAGE_SAMPLES 5
#define RPM_RESET_TIME_US 2000000 // 2000 ms

typedef enum eEncoderId {
    eEncoderId_First = 0,
    eEncoderId_RIGHT_1 = eEncoderId_First,
    eEncoderId_LEFT_1,
    eEncoderId_Last
} eEncoderId_t;

class Encoder {
public:
    Encoder(eEncoderId_t id, gpio_num_t pin);
    
    bool init();
    uint16_t getRPM();
    uint32_t getPulseCount();
    void resetPulseCount();

private:
    eEncoderId_t id;
    gpio_num_t pin;
    uint32_t pulse_count;
    uint64_t last_pulse_time_us;
    uint64_t pulse_period_us;
    uint64_t pulses_per_rotation_period_us;
    bool debounce_active;
    portMUX_TYPE lock;

#if defined(ENCODER_USE_HARDWARE_TIMER)
    esp_timer_handle_t debounce_timer;
#elif defined(ENCODER_USE_FREERTOS_TIMER)
    TimerHandle_t debounce_timer;
#endif
    uint16_t rpm_samples[RPM_AVERAGE_SAMPLES];
    size_t rpm_sample_index;
    size_t rpm_samples_count;
    
    static void IRAM_ATTR gpioIsrHandler(void* arg);
    static void update_pulses(Encoder* encoder);
    
#ifdef ENCODER_USE_HARDWARE_TIMER
    static void debounceTimerCallback(void* arg);
#else
    static void debounceTimerCallback(TimerHandle_t xTimer);
#endif

    void processPulse();
};