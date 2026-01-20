#pragma once

#include <esp_wifi.h>
#include <esp_event.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>

class WiFi {
public:
    WiFi(const char* ssid, const char* password);
    
    bool init();
    bool connect();
    bool is_connected();
    const char* get_ip_address();
    
private:
    const char* ssid;
    const char* password;
    EventGroupHandle_t wifi_event_group;
    bool connected;
    char ip_address[16];
    
    static void event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
};
