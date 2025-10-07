#pragma once

#include "esp_err.h"

// Initialize encoder with two GPIO pins (A and B channels)
esp_err_t encoder_init(int gpioA, int gpioB);

// Get the current encoder count (relative position)
int encoder_get_count(void);

// Reset encoder count to zero
void encoder_reset(void);
