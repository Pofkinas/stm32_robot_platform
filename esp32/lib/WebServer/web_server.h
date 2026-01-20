#pragma once

#include <esp_http_server.h>
#include "message.h"
#include <functional>

#define MAX_LOG_MESSAGES 20
#define MAX_LOG_LENGTH 512

#define CHUNK_SIZE 1024

#define CMD_STRING_SIZE 64
#define ENUM_JSON_SIZE 1024

class WebServer {
public:
    typedef std::function<bool(char* cmd, int type, sMessage_t params, sMessage_t *response)> CommandHandlerStructured;
    
    WebServer(uint16_t port = 80);
    
    bool init();
    bool start();
    void stop();
    
    void set_command_handler(CommandHandlerStructured handler);
    void add_log_message(const char* message);
    
private:
    httpd_handle_t server;
    uint16_t port;
    CommandHandlerStructured command_handler;
    
    // Circular buffer for log messages
    char log_buffer[MAX_LOG_MESSAGES][MAX_LOG_LENGTH];
    uint32_t log_ids[MAX_LOG_MESSAGES];
    int log_index;
    int log_count;
    uint32_t next_message_id;
    
    // HTTP handlers
    static esp_err_t http_root_handler(httpd_req_t *req);
    static esp_err_t http_command_handler(httpd_req_t *req);
    static esp_err_t http_logs_handler(httpd_req_t *req);
};

// Global pointer to the running WebServer instance. Set by the WebServer constructor.
extern WebServer* g_server_instance;
