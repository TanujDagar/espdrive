#include <stdio.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "can_comm.h"
#include "motor_control.h"
#include "encoder.h"
#include "driver/twai.h"   // for state checks

#define MY_CAN_ID 0x121
#define MOTOR_PWM_GPIO   12   // your wiring
#define MOTOR_DIR_GPIO   11
#define ENCODER_A_GPIO   4    // encoder channel A
#define ENCODER_B_GPIO   5    // encoder channel B

void can_task(void *pvParameters) {
    motor_cmd_t cmd;

    while (1) {
        if (can_comm_receive(&cmd, pdMS_TO_TICKS(1000)) == ESP_OK) {
            printf("Received Motor Command: Speed=%d, Direction=%d\n",
                   cmd.speed, cmd.direction);
            motor_control_set_cmd(&cmd); // send command to motor driver
        }
    }
}

void encoder_task(void *pvParameters) {
    int last_count = 0;
    int ticks_per_rev = 600;  // your encoder PPR
    while (1) {
        int count = encoder_get_count();
        int delta = count - last_count;
        last_count = count;

        // time = 0.5s (500 ms delay)
        float revs = (float)delta / ticks_per_rev;
        float rpm = (revs / 0.5f) * 60.0f;

        printf("Encoder Count=%d, Speed=%.2f RPM\n", count, rpm);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void can_recovery_task(void *pvParameters) {
    while (1) {
        twai_status_info_t status;
        twai_get_status_info(&status);

        if (status.state == TWAI_STATE_BUS_OFF) {
            printf("CAN Bus-Off detected, restarting driver...\n");
            twai_stop();
            twai_driver_uninstall();
            vTaskDelay(pdMS_TO_TICKS(100));  // short wait before reinit
            can_comm_init(MY_CAN_ID);        // reinitialize CAN
        }

        vTaskDelay(pdMS_TO_TICKS(500)); // check every 0.5s
    }
}

void app_main(void) {
    printf("ESP32-S3 CAN + Motor + Encoder Demo Start\n");

    // Retry loops instead of return
    while (can_comm_init(MY_CAN_ID) != ESP_OK) {
        printf("CAN init failed, retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while (motor_control_init(MOTOR_PWM_GPIO, MOTOR_DIR_GPIO) != ESP_OK) {
        printf("Motor control init failed, retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    while (encoder_init(ENCODER_A_GPIO, ENCODER_B_GPIO) != ESP_OK) {
        printf("Encoder init failed, retrying...\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Start tasks
    xTaskCreate(can_task, "can_task", 4096, NULL, 5, NULL);
    xTaskCreate(encoder_task, "encoder_task", 4096, NULL, 5, NULL);
    xTaskCreate(can_recovery_task, "can_recovery_task", 4096, NULL, 6, NULL);
}
