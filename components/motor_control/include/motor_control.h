#pragma once
#include "esp_err.h"

// Motor command structure (same as used in CAN)
typedef struct {
    int speed;      // PWM duty cycle (0–100)
    int direction;  // 0 = forward, 1 = reverse
} motor_cmd_t;

// Initialize motor control (setup PWM + DIR GPIO)
esp_err_t motor_control_init(int pwm_gpio, int dir_gpio);

// Apply motor command
esp_err_t motor_control_set_cmd(const motor_cmd_t *cmd);
