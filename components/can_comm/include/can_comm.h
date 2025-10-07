#ifndef CAN_COMM_H
#define CAN_COMM_H

#include "driver/twai.h"
#include "motor_control.h"   // motor_cmd_t lives here

// Init CAN with a specific RX filter ID (each ESP listens only to its own ID)
esp_err_t can_comm_init(uint32_t my_can_id);

// Send a CAN frame to a target CAN ID
esp_err_t can_comm_send(uint32_t target_id, const motor_cmd_t *cmd);

// Receive a CAN frame (blocking or with timeout)
esp_err_t can_comm_receive(motor_cmd_t *cmd, TickType_t ticks_to_wait);

#endif // CAN_COMM_H
