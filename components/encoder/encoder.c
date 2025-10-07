#include "encoder.h"
#include "driver/pulse_cnt.h"
#include "esp_log.h"

static const char *TAG = "ENCODER";

static pcnt_unit_handle_t pcnt_unit = NULL;

esp_err_t encoder_init(int gpioA, int gpioB) {
    pcnt_unit_config_t unit_config = {
        .low_limit = -32768,
        .high_limit = 32767,
    };
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    // Channel A (pulse input)
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num = gpioA,
        .level_gpio_num = gpioB,   // B acts as direction
    };
    pcnt_channel_handle_t chan_a = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &chan_a));

    // Set edge/level actions
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,  // rising edge on A -> count up
        PCNT_CHANNEL_EDGE_ACTION_DECREASE   // falling edge on A -> count down
    ));
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(
        chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,     // B=0 -> keep action
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE   // B=1 -> invert action
    ));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcnt_unit));
    ESP_ERROR_CHECK(pcnt_unit_start(pcnt_unit));

    ESP_LOGI(TAG, "Encoder initialized on GPIOA=%d, GPIOB=%d", gpioA, gpioB);
    return ESP_OK;
}

int encoder_get_count(void) {
    int count = 0;
    pcnt_unit_get_count(pcnt_unit, &count);
    return count;
}

void encoder_reset(void) {
    pcnt_unit_clear_count(pcnt_unit);
}
