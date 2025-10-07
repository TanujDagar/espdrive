#include "motor_control.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "MOTOR_CONTROL";

// Configurable GPIOs
static int pwm_pin;
static int dir_pin;

esp_err_t motor_control_init(int pwm_gpio, int dir_gpio) {
    pwm_pin = pwm_gpio;
    dir_pin = dir_gpio;

    // Configure direction pin
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << dir_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    // Configure PWM with LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_10_BIT, // 10-bit = 0–1023
        .freq_hz          = 20000,             // 20 kHz PWM
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pwm_pin,
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0,
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI(TAG, "Motor control initialized (PWM=%d, DIR=%d)", pwm_gpio, dir_gpio);
    return ESP_OK;
}

esp_err_t motor_control_set_cmd(const motor_cmd_t *cmd) {
    if (!cmd) return ESP_ERR_INVALID_ARG;

    // Clamp speed between 0–100
    int duty_percent = cmd->speed;
    if (duty_percent < 0) duty_percent = 0;
    if (duty_percent > 100) duty_percent = 100;

    // Convert percent -> duty (10-bit resolution = 0–1023)
    int duty = (duty_percent * 1023) / 100;

    // Set PWM duty
    ESP_ERROR_CHECK(ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty));
    ESP_ERROR_CHECK(ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0));

    // Set direction
    gpio_set_level(dir_pin, cmd->direction ? 1 : 0);

    ESP_LOGI(TAG, "Applied motor command: speed=%d%% (duty=%d), direction=%s",
             duty_percent, duty, cmd->direction ? "Reverse" : "Forward");

    return ESP_OK;
}

