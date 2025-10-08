#include "can_comm.h"
#include "esp_log.h"

static const char *TAG = "CAN_COMM";

// Define CAN timings and pins (adjust to your board wiring)
#define CAN_TX_GPIO  43
#define CAN_RX_GPIO  44

static uint32_t my_filter_id = 0;  // ESP's own CAN ID

esp_err_t can_comm_init(uint32_t my_can_id) {
    my_filter_id = my_can_id;

    twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(CAN_TX_GPIO, CAN_RX_GPIO, TWAI_MODE_NORMAL);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();

    // Configure filter to only accept this ESP's CAN ID
    twai_filter_config_t f_config = {
        .acceptance_code = (my_can_id << 21),  // 11-bit std frame → bits shifted left
        .acceptance_mask = ~(0x7FF << 21),     // accept only this ID
        .single_filter = true
    };

    // Install driver
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to install TWAI driver");
        return ESP_FAIL;
    }

    // Start driver
    if (twai_start() != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start TWAI driver");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "CAN/TWAI driver started with filter ID=0x%03X", my_can_id);
    return ESP_OK;
}

esp_err_t can_comm_send(uint32_t target_id, const motor_cmd_t *cmd) {
    int speed = cmd->speed;
    int direction = cmd->direction;

    // Clamp speed [0..100]
    if (speed < 0) speed = 0;
    if (speed > 100) speed = 100;

    twai_message_t tx_msg = {
        .identifier = target_id,  // Send to specific ESP
        .extd = 0,                // standard frame (11-bit ID)
        .data_length_code = 2     // 2 bytes: speed + direction
    };

    tx_msg.data[0] = (uint8_t)(speed & 0xFF);      // 0–100
    tx_msg.data[1] = (uint8_t)(direction & 0xFF);  // 0 or 1

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(1000)) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit CAN frame");
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Sent CAN frame to ID=0x%03X: speed=%d%%, dir=%d",
             target_id, speed, direction);
    return ESP_OK;
}

esp_err_t can_comm_receive(motor_cmd_t *cmd, TickType_t ticks_to_wait) {
    twai_message_t rx_msg;

    if (twai_receive(&rx_msg, ticks_to_wait) == ESP_OK) {
        if (rx_msg.data_length_code >= 2) {
            cmd->speed = rx_msg.data[0];       // 0–100
            cmd->direction = rx_msg.data[1];   // 0/1
            ESP_LOGI(TAG, "Received CAN frame on ID=0x%03X: speed=%d%%, dir=%d", 
                     rx_msg.identifier, cmd->speed, cmd->direction);
            return ESP_OK;
        }
    }

    return ESP_FAIL;
}
