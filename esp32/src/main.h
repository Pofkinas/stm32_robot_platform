#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "encoder.h"
#include "message.h"

#define CMD_COMMS_TIMEOUT pdMS_TO_TICKS(100) // 100 ms

bool comm_send (const uint8_t command, const sMessage_t params, const uint32_t timeout);
uint16_t get_encoder_rpm (const eEncoderId_t encoder);
