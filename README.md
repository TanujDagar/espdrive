# espdrive

**espdrive** is an ESP32-S3-based embedded project for motor control, encoder feedback, and CAN (Controller Area Network) communication. It demonstrates how to build a distributed motor control system using multiple ESP32 boards communicating over CAN bus, each controlling a motor and reading encoder feedback.

## Features

- **CAN Communication:** Each ESP32 node uses the TWAI (CAN) peripheral to send/receive motor commands using 11-bit standard frames. Devices are addressed by unique CAN IDs.
- **Motor Control:** Controls DC motors using PWM (speed) and a direction pin. Supports 0–100% duty cycle and direction switching.
- **Quadrature Encoder Feedback:** Reads incremental encoder signals (channels A & B) for relative position and RPM calculation.
- **Robustness:** Includes CAN bus-off detection and automatic recovery.

## Hardware Requirements

- ESP32-S3 or compatible module with TWAI peripheral.
- DC motor driver circuit (H-bridge, e.g., L298N, TB6612, or similar).
- Quadrature encoder attached to the motor.
- CAN transceiver (e.g., SN65HVD230, MCP2551) for each ESP32.
- Common CAN bus wiring (CANH, CANL, GND).

## Wiring Example

- **Motor PWM:** Connect to a high-speed PWM-capable GPIO (default: GPIO12).
- **Motor DIR:** Connect to a digital output GPIO (default: GPIO11).
- **Encoder A/B:** Connect encoder signal channels to digital input GPIOs (default: GPIO4 and GPIO5).
- **CAN TX/RX:** Default pins GPIO43 (TX) and GPIO44 (RX); wire to CAN transceiver.

## Software Architecture

- Modular components: `can_comm`, `motor_control`, `encoder`
- Main application initializes all peripherals and starts FreeRTOS tasks for CAN command handling, encoder monitoring, and CAN recovery.

## Quick Start

### 1. Clone the repository

```sh
git clone https://github.com/TanujDagar/espdrive.git
cd espdrive
```

### 2. Configure your hardware mapping

If your wiring differs from the defaults, edit the GPIO defines in `main/main.c`:

```c
#define MOTOR_PWM_GPIO   12
#define MOTOR_DIR_GPIO   11
#define ENCODER_A_GPIO   4
#define ENCODER_B_GPIO   5
#define MY_CAN_ID        0x121  // Unique for each node
```

### 3. Build and Flash

This project uses [ESP-IDF](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/):

```sh
idf.py set-target esp32s3
idf.py build
idf.py -p /dev/ttyUSB0 flash monitor
```

### 4. Run

- On power-up, the ESP32 will initialize the CAN bus, motor driver, and encoder.
- The device listens for CAN frames addressed to its CAN ID and sets the motor accordingly.
- Encoder feedback (count and calculated RPM) is printed periodically.
- If CAN bus-off is detected, the ESP32 will attempt to recover automatically.

## Example: Sending a Motor Command via CAN

Send a frame from another ESP32 or a CAN tool with:

- **ID:** Target CAN node's ID (e.g., `0x121`)
- **Data (2 bytes):**
  - Byte 0: Speed (0–100)
  - Byte 1: Direction (0=forward, 1=reverse)

Example: To set speed to 60% and direction to forward:

| Byte 0 | Byte 1 |
|--------|--------|
|   60   |   0    |

## Code Organization

- `components/can_comm/` – CAN communication module (`can_comm_init`, `can_comm_send`, `can_comm_receive`)
- `components/motor_control/` – Motor driver control (`motor_control_init`, `motor_control_set_cmd`)
- `components/encoder/` – Quadrature encoder interface (`encoder_init`, `encoder_get_count`, `encoder_reset`)
- `main/` – Application logic and FreeRTOS tasks

## Extending

- Close loop system usingg encoder data
- Current sensor readings for more control

## Troubleshooting

- **CAN not initializing:** Check CAN transceiver wiring and that the bus is properly terminated (120Ω at both ends).
- **No motor movement:** Verify motor driver wiring and GPIO assignments.
- **Encoder not counting:** Confirm encoder wiring and correct GPIOs.

## License

[MIT](LICENSE)

---

**Author:** [TanujDagar](https://github.com/TanujDagar)
