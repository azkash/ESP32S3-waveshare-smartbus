#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#define RS485_RX_PIN    16
#define RS485_TX_PIN    15
#define RS485_BAUD      9600
#define MD02_SLAVE_ADDR 0x01

typedef struct {
    float temperature;  // °C
    float humidity;     // %RH
    bool  valid;        // true when last read succeeded
} modbus_sensor_data_t;

/**
 * @brief Initialise RS485 UART and start the background Modbus polling task.
 */
void modbus_sensor_init(void);

/**
 * @brief Return latest sensor readings (thread-safe copy).
 */
modbus_sensor_data_t modbus_sensor_get(void);

#ifdef __cplusplus
}
#endif
