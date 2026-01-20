#include "encoder.h"

#include "esp_timer.h"
#include "driver/gpio.h"
#include <string.h>
#include <debug.h>
#include <logger.h>

Encoder::Encoder(eEncoderId_t id, gpio_num_t pin):
    id(id),
    pin(pin),
    pulse_count(0),
    last_pulse_time_us(0),
    pulse_period_us(0),
    debounce_active(false),
    #if defined(ENCODER_USE_HARDWARE_TIMER) || defined(ENCODER_USE_FREERTOS_TIMER)
    debounce_timer(NULL),
    #endif
    rpm_sample_index(0) {
        memset(rpm_samples, 0, sizeof(rpm_samples));
        lock = portMUX_INITIALIZER_UNLOCKED;
    }

bool Encoder::init() {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_POSEDGE
    };
    
    if (gpio_config(&io_conf) != ESP_OK) {
        return false;
    }

#if defined(ENCODER_USE_HARDWARE_TIMER)
    // Create hardware timer with microsecond precision
    const esp_timer_create_args_t timer_args = {
        .callback = debounceTimerCallback,
        .arg = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name = "DebounceTimer",
        .skip_unhandled_events = false
    };
    
    if (esp_timer_create(&timer_args, &debounce_timer) != ESP_OK) {
        return false;
    }
#elif defined(ENCODER_USE_FREERTOS_TIMER)
    // Create FreeRTOS timer with millisecond precision
    debounce_timer = xTimerCreate("DebounceTimer", pdMS_TO_TICKS((DEBOUNCE_TIME_US/1000)), pdFALSE, this, debounceTimerCallback);
    
    if (debounce_timer == NULL) {
        return false;
    }
#else
    // ISR-only mode: no timer to create, debounce handled inline in ISR
#endif
    
    if (gpio_isr_handler_add(pin, gpioIsrHandler, this) != ESP_OK) {
        return false;
    }
    
    return true;
}

void IRAM_ATTR Encoder::gpioIsrHandler(void* arg) {
    Encoder* encoder = static_cast<Encoder*>(arg);

#if defined(ENCODER_USE_HARDWARE_TIMER) || defined(ENCODER_USE_FREERTOS_TIMER)
    if (encoder->debounce_active) {
        return;
    }

    portENTER_CRITICAL_ISR(&encoder->lock);
    encoder->debounce_active = true;
    portEXIT_CRITICAL_ISR(&encoder->lock);

#if defined(ENCODER_USE_HARDWARE_TIMER)
    // Start hardware timer (microseconds)
    esp_timer_start_once(encoder->debounce_timer, DEBOUNCE_TIME_US);
#else
    // Start FreeRTOS timer (milliseconds)
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTimerStartFromISR(encoder->debounce_timer, &xHigherPriorityTaskWoken);
    
    if (xHigherPriorityTaskWoken) {
        portYIELD_FROM_ISR();
    }
#endif

#else
    int pin_state = gpio_get_level(encoder->pin);

    if (pin_state == 1) {
        update_pulses(encoder);
    }
#endif
}

#ifdef ENCODER_USE_HARDWARE_TIMER
void Encoder::debounceTimerCallback(void* arg) {
    Encoder* encoder = static_cast<Encoder*>(arg);
    
    int pin_state = gpio_get_level(encoder->pin);
    
    portENTER_CRITICAL(&encoder->lock);
    encoder->debounce_active = false;
    portEXIT_CRITICAL(&encoder->lock);

    if (pin_state == 1) {
        update_pulses(encoder);
    }
}
#endif

#ifdef ENCODER_USE_FREERTOS_TIMER
void Encoder::debounceTimerCallback(TimerHandle_t xTimer) {
    Encoder* encoder = static_cast<Encoder*>(arg);
    
    int pin_state = gpio_get_level(encoder->pin);

    portENTER_CRITICAL(&encoder->lock);
    encoder->debounce_active = false;
    portEXIT_CRITICAL(&encoder->lock);
    
    if (pin_state == 1) {
        update_pulses(encoder);
    }
}
#endif

void Encoder::update_pulses(Encoder* encoder) {
    if (encoder == nullptr) {
        return;
    }
    
    uint64_t current_time_us = esp_timer_get_time();
        
    portENTER_CRITICAL(&encoder->lock);
    if (encoder->last_pulse_time_us > 0) {
        encoder->pulse_period_us = current_time_us - encoder->last_pulse_time_us;

        if (encoder->pulse_period_us > RPM_RESET_TIME_US) {
            uint64_t period = encoder->pulse_period_us;
            
            portEXIT_CRITICAL(&encoder->lock);
            
            encoder->resetPulseCount();
            
            // DEBUG_WARNING("Pulse reset on pin %d, period %llu us\n", encoder->pin, period);
            //LOG_ESP("Pulse reset on pin %d, period %llu us\n", encoder->pin, period);

            return;
        }
    }
    
    encoder->last_pulse_time_us = current_time_us;
    
    encoder->pulse_count++;
    encoder->pulses_per_rotation_period_us += encoder->pulse_period_us;
    
    uint16_t rpm = 0;
    
    if ((encoder->pulse_count % ENCODER_PPR) == 0) {
        if (encoder->pulses_per_rotation_period_us > 0) {
            rpm = (60 * 1000000) / (encoder->pulses_per_rotation_period_us / ENCODER_PPR);

            encoder->rpm_samples[encoder->rpm_sample_index] = rpm;
            encoder->rpm_sample_index = (encoder->rpm_sample_index + 1) % RPM_AVERAGE_SAMPLES;
            encoder->rpm_samples_count = (encoder->rpm_samples_count < RPM_AVERAGE_SAMPLES) ? (encoder->rpm_samples_count + 1) : RPM_AVERAGE_SAMPLES;
        }

        encoder->pulses_per_rotation_period_us = 0;

        // DEBUG_INFO("RPM: %d on pin %d\n", rpm, encoder->pin);
        // LOG_ESP("RPM: %d on pin %d\n", rpm, encoder->pin);
    }

    portEXIT_CRITICAL(&encoder->lock);
}

uint16_t Encoder::getRPM() {
    uint64_t current_time_us = esp_timer_get_time();

    portENTER_CRITICAL(&lock);
    uint64_t last = last_pulse_time_us;
    size_t samples_count = rpm_samples_count;
    portEXIT_CRITICAL(&lock);

    if (last == 0) {
        // DEBUG_WARNING("No prior pulses pin %d\n", pin);
        // LOG_ESP("No prior pulses pin %d\n", pin);
        
        return 0;
    }

    uint64_t time_since_last_pulse = current_time_us - last;

    if (time_since_last_pulse > RPM_RESET_TIME_US) {
        resetPulseCount();
        
        // DEBUG_WARNING("No pulses pin %d for %llu us\n", pin, time_since_last_pulse);
        // LOG_ESP("No pulses pin %d for %llu us\n", pin, time_since_last_pulse);
        
        return 0;
    }
    
    uint16_t sum = 0;
    
    portENTER_CRITICAL(&lock);
    for (int i = 0; i < samples_count; i++) {
        sum += rpm_samples[i];
    }
    portEXIT_CRITICAL(&lock);
    
    return (samples_count > 0 ? sum / samples_count : sum);
}

uint32_t Encoder::getPulseCount() {
    portENTER_CRITICAL(&lock);
    uint32_t c = pulse_count;
    portEXIT_CRITICAL(&lock);
    return c;
}

void Encoder::resetPulseCount() {
    portENTER_CRITICAL(&lock);
    pulse_count = 0;
    last_pulse_time_us = 0;
    pulse_period_us = 0;
    pulses_per_rotation_period_us = 0;
    rpm_sample_index = 0;
    rpm_samples_count = 0;
    memset(rpm_samples, 0, sizeof(rpm_samples));
    portEXIT_CRITICAL(&lock);
}
