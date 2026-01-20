#include "web_server.h"

#include "esp_log.h"
#include "esp_littlefs.h"
#include <string.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "config.h"
#include "esp_protocol.h"

static const char* TAG = "WebServer";
WebServer* g_server_instance = NULL;

static size_t json_escape_to_buffer(const char* input, char* output, size_t output_size) {
    if ((input == NULL) || (output == NULL) || (output_size == 0)) {
        return 0;
    }

    size_t out = 0;
    for (size_t i = 0; input[i] != '\0' && out + 2 < output_size; i++) {
        const unsigned char uc = (unsigned char)input[i];
        const char c = (char)uc;

        // JSON strings cannot contain unescaped control chars (0x00-0x1F).
        if (uc < 0x20) {
            if (out + 6 >= output_size) {
                break;
            }
            // Emit as \u00XX
            snprintf(output + out, output_size - out, "\\u%04X", (unsigned int)uc);
            out += 6;
            continue;
        }
        switch (c) {
            case '"':
            case '\\':
                output[out++] = '\\';
                output[out++] = c;
                break;
            default:
                output[out++] = c;
                break;
        }
    }

    output[out] = '\0';
    return out;
}

// Parse JSON command from browser: {"cmd":"modom_set","type":0,"values":3,"params":"50;0;0"}
static bool parse_json_command(const char* json, char* cmd, int* type, sMessage_t *params) {
    if ((json == NULL) || (cmd == NULL) || (type == NULL) || (params == NULL)) {
        return false;
    }
    
    // Extract cmd string
    const char* cmd_ptr = strstr(json, "\"cmd\":\"");
    if (cmd_ptr != NULL) {
        cmd_ptr += 7; // Skip "\"cmd\":\""
        const char* cmd_end = strchr(cmd_ptr, '\"');
        
        if (cmd_end != NULL) {
            size_t len = cmd_end - cmd_ptr;
            
            if (len < CMD_STRING_SIZE - 1) {
                strncpy(cmd, cmd_ptr, len);
                cmd[len] = '\0';
            }
        }
    }
    
    // Extract type
    const char* type_ptr = strstr(json, "\"type\":");
    if (type_ptr != NULL) {
        *type = atoi(type_ptr + 7);
    }

    uint8_t arg_count = 0;
    const char* values_ptr = strstr(json, "\"values\":");
    if (values_ptr != NULL) {
        arg_count = atoi(values_ptr + 9);
    }
    
    // Extract params string
    const char* params_ptr = strstr(json, "\"params\":\"");
    if (params_ptr != NULL) {
        params_ptr += 10; // Skip "\"params\":\""
        const char* params_end = strchr(params_ptr, '\"');
        
        if (params_end != NULL) {
            size_t len = params_end - params_ptr;
            size_t copy_len = (len < CMD_BUFFER_SIZE - 1) ? len : CMD_BUFFER_SIZE - 1;
            
            strncpy(params->data, params_ptr, copy_len);
            params->data[copy_len] = '\0';

            while (copy_len > 0 && params->data[copy_len - 1] == ESP_ARG_SEPARATOR[0]) {
                params->data[copy_len - 1] = '\0';
                copy_len--;
                if (arg_count > 0) {
                    arg_count--;
                }
            }

            const char* sensor_ptr = strstr(json, "\"sensorByte\":");
            if (sensor_ptr != NULL) {
                int sensor_value = atoi(sensor_ptr + 13);
                size_t used = strlen(params->data);
                if (used < (CMD_BUFFER_SIZE - 1)) {
                    if (used > 0 && params->data[used - 1] != ESP_ARG_SEPARATOR[0]) {
                        strncat(params->data, ESP_ARG_SEPARATOR, CMD_BUFFER_SIZE - used - 1);
                        used = strlen(params->data);
                    }

                    char tmp[16];
                    snprintf(tmp, sizeof(tmp), "%d", sensor_value);
                    strncat(params->data, tmp, CMD_BUFFER_SIZE - used - 1);
                }
            }
        }
    }
    
    // Set size to argument count, not string length
    params->size = arg_count;
    
    // Validate required fields
    return (cmd[0] != '\0' && *type >= 0);
}

WebServer::WebServer(uint16_t port): 
    server(NULL),
    port(port),
    command_handler(NULL),
    log_index(0),
    log_count(0),
    next_message_id(1) {
        memset(log_buffer, 0, sizeof(log_buffer));
        memset(log_ids, 0, sizeof(log_ids));
        g_server_instance = this;
    }

bool WebServer::init() {
    // Initialize LittleFS
    esp_vfs_littlefs_conf_t conf = {
        .base_path = "/rootfs",
        .partition_label = "storage",
        .partition = NULL,
        .format_if_mount_failed = true,
        .read_only = false,
        .dont_mount = false,
        .grow_on_mount = false
    };
    
    esp_err_t ret = esp_vfs_littlefs_register(&conf);
    
    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        } else if (ret == ESP_ERR_NOT_FOUND) {
            ESP_LOGE(TAG, "Failed to find LittleFS partition");
        } else {
            ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
        }
        return false;
    }
    
    size_t total = 0, used = 0;
    ret = esp_littlefs_info(conf.partition_label, &total, &used);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get LittleFS partition information (%s)", esp_err_to_name(ret));

        return false;
    } else {
        ESP_LOGI(TAG, "LittleFS: %d KB total, %d KB used", total / 1024, used / 1024);
    }
    
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_uri_handlers = 8;
    config.max_open_sockets = 7;
    config.stack_size = WEB_SERVER_STACK_SIZE;
    
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start HTTP server");
        return false;
    }
    
    // Register URI handlers
    httpd_uri_t root_uri = {
        .uri = "/",
        .method = HTTP_GET,
        .handler = http_root_handler,
        .user_ctx = this
    };
    httpd_register_uri_handler(server, &root_uri);
    
    httpd_uri_t command_uri = {
        .uri = "/command",
        .method = HTTP_POST,
        .handler = http_command_handler,
        .user_ctx = this
    };
    httpd_register_uri_handler(server, &command_uri);

    httpd_uri_t logs_uri = {
        .uri = "/logs",
        .method = HTTP_GET,
        .handler = http_logs_handler,
        .user_ctx = this
    };
    httpd_register_uri_handler(server, &logs_uri);

    ESP_LOGI(TAG, "Web server initialized successfully");
    return true;
}

bool WebServer::start() {
    return server != NULL;
}

void WebServer::stop() {
    if (server) {
        httpd_stop(server);
        server = NULL;
    }
}

void WebServer::set_command_handler(CommandHandlerStructured handler) {
    command_handler = handler;
}

void WebServer::add_log_message(const char* message) {
    if (message == NULL) {
        return;
    }

    size_t len = strlen(message);
    if (len >= MAX_LOG_LENGTH) {
        len = MAX_LOG_LENGTH - 1;
    }
    
    strncpy(log_buffer[log_index], message, len);
    log_buffer[log_index][len] = '\0';
    uint32_t msg_id = next_message_id++;
    log_ids[log_index] = msg_id;
    
    log_index = (log_index + 1) % MAX_LOG_MESSAGES;
    
    if (log_count < MAX_LOG_MESSAGES) {
        log_count++;
    }
}

esp_err_t WebServer::http_root_handler(httpd_req_t *req) {
    const char* filepath = "/rootfs/index.html";
    
    FILE* f = fopen(filepath, "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", filepath);
        
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to open HTML file");
        
        return ESP_FAIL;
    }
    
    httpd_resp_set_type(req, "text/html");
    
    char chunk[CHUNK_SIZE];
    size_t chunksize;
    
    do {
        chunksize = fread(chunk, 1, sizeof(chunk), f);
        if (chunksize > 0) {
            if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
                fclose(f);
                
                ESP_LOGE(TAG, "File sending failed");
                httpd_resp_sendstr_chunk(req, NULL);
                
                return ESP_FAIL;
            }
        }
    } while (chunksize != 0);
    
    fclose(f);
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}

esp_err_t WebServer::http_command_handler(httpd_req_t *req) {
    WebServer* server = static_cast<WebServer*>(req->user_ctx);
    
    char content[CMD_BUFFER_SIZE];
    int retrieved_bytes = httpd_req_recv(req, content, sizeof(content) - 1);
    
    if (retrieved_bytes <= 0) {
        httpd_resp_send_500(req);
        
        return ESP_FAIL;
    }
    
    content[retrieved_bytes] = '\0';
    
    // ESP_LOGI(TAG, "Received JSON: %s", content);
    
    // Parse JSON command
    char command_str[CMD_STRING_SIZE] = {0};
    int type = -1;
    char params_str[CMD_BUFFER_SIZE] = {0};
    
    sMessage_t params = {
        .data = params_str,
        .size = 0
    };
    
    if (!parse_json_command(content, command_str, &type, &params)) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        
        return ESP_FAIL;
    }
    
    char response[CMD_RESPONSE_SIZE] = {0};
    sMessage_t response_msg = {.data = response, .size = sizeof(response)};
    bool success = false;
    
    // Log command in the same style as the UI
    char log_msg[MAX_LOG_LENGTH];
    if (params_str[0] != '\0') {
        snprintf(log_msg, sizeof(log_msg), "> %s:%s", command_str, params_str);
    } else {
        snprintf(log_msg, sizeof(log_msg), "> %s", command_str);
    }
    server->add_log_message(log_msg);
    
    if (server->command_handler != NULL) {
        success = server->command_handler(command_str, type, params, &response_msg);
        
        // Log result
        if (success) {
            snprintf(log_msg, sizeof(log_msg), "< OK: %s", response[0] ? response : "Command sent");
        } else {
            snprintf(log_msg, sizeof(log_msg), "< ERROR: %s", response[0] ? response : "Failed");
        }
    } else {
        snprintf(log_msg, sizeof(log_msg), "< ERROR: No handler");
    }

    server->add_log_message(log_msg);
    
    // Send response
    httpd_resp_set_type(req, "application/json");
    char json_response[MAX_LOG_LENGTH];

    char escaped_response[CMD_RESPONSE_SIZE * 2];
    json_escape_to_buffer(response, escaped_response, sizeof(escaped_response));
    snprintf(json_response, sizeof(json_response), "{\"success\":%s,\"response\":\"%.*s\"}", success ? "true" : "false", 200, escaped_response);
    httpd_resp_send(req, json_response, strlen(json_response));
    
    return ESP_OK;
}

esp_err_t WebServer::http_logs_handler(httpd_req_t *req) {
    WebServer* server = static_cast<WebServer*>(req->user_ctx);

    // Avoid stale log responses when polling
    httpd_resp_set_hdr(req, "Cache-Control", "no-store");
    
    // Parse since_id parameter from query string
    char query[64] = {0};
    uint32_t since_id = 0;
    
    if (httpd_req_get_url_query_str(req, query, sizeof(query)) == ESP_OK) {
        char param[32] = {0};
        if (httpd_query_key_value(query, "since", param, sizeof(param)) == ESP_OK) {
            since_id = atoi(param);
        }
    }
    
    // Build JSON array of new messages
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr_chunk(req, "[\n");
    
    bool first = true;
    int start_index = (server->log_index - server->log_count + MAX_LOG_MESSAGES) % MAX_LOG_MESSAGES;
    
    for (int i = 0; i < server->log_count; i++) {
        int idx = (start_index + i) % MAX_LOG_MESSAGES;
        
        if (server->log_ids[idx] > since_id) {
            if (!first) {
                httpd_resp_sendstr_chunk(req, ",\n");
            }
            first = false;
            
            char json_entry[MAX_LOG_LENGTH * 2 + 100];
            char escaped[MAX_LOG_LENGTH * 2];

            json_escape_to_buffer(server->log_buffer[idx], escaped, sizeof(escaped));
            
            snprintf(json_entry, sizeof(json_entry), 
                     "{\"id\":%lu,\"message\":\"%s\"}",
                     server->log_ids[idx], escaped);
            
            httpd_resp_sendstr_chunk(req, json_entry);
        }
    }
    
    httpd_resp_sendstr_chunk(req, "\n]");
    httpd_resp_send_chunk(req, NULL, 0);
    
    return ESP_OK;
}