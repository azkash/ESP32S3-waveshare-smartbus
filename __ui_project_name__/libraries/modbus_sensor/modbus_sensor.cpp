/*
 * Modbus RTU Driver for MD02 / XY-MD02 Temperature & Humidity Sensor
 * Clean implementation using ModbusMaster library without troubleshooting overhead.
 */

#include "modbus_sensor.h"
#include <Arduino.h>
#include <HardwareSerial.h>
#include <ModbusMaster.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

static HardwareSerial RS485Serial(1);
static ModbusMaster node;
static modbus_sensor_data_t _sensor_data;
static SemaphoreHandle_t _data_mutex = NULL;
static TaskHandle_t _poll_task = NULL;

static void modbus_task(void *arg)
{
    (void)arg;

    RS485Serial.begin(RS485_BAUD, SERIAL_8N1, RS485_RX_PIN, RS485_TX_PIN);
    node.begin(MD02_SLAVE_ADDR, RS485Serial);

    vTaskDelay(pdMS_TO_TICKS(1000));

    for (;;) {
        // Try reading Input Registers 0x0001 (standard MD02)
        uint8_t result = node.readInputRegisters(0x0001, 2);

        // Fallback to Holding Registers 0x0001 if input registers fail
        if (result != node.ku8MBSuccess) {
            result = node.readHoldingRegisters(0x0001, 2);
        }

        if (xSemaphoreTake(_data_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
            if (result == node.ku8MBSuccess) {
                uint16_t rawTemp = node.getResponseBuffer(0);
                uint16_t rawHum  = node.getResponseBuffer(1);

                _sensor_data.temperature = (int16_t)rawTemp / 10.0f;
                _sensor_data.humidity    = rawHum / 10.0f;
                _sensor_data.valid       = true;
            } else {
                _sensor_data.valid       = false;
            }
            xSemaphoreGive(_data_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void modbus_sensor_init(void)
{
    _data_mutex = xSemaphoreCreateMutex();
    memset(&_sensor_data, 0, sizeof(_sensor_data));

    xTaskCreatePinnedToCore(
        modbus_task,
        "modbus_task",
        3072,
        NULL,
        1,
        &_poll_task,
        0
    );
}

modbus_sensor_data_t modbus_sensor_get(void)
{
    modbus_sensor_data_t copy;
    memset(&copy, 0, sizeof(copy));
    if (xSemaphoreTake(_data_mutex, pdMS_TO_TICKS(50)) == pdTRUE) {
        copy = _sensor_data;
        xSemaphoreGive(_data_mutex);
    }
    return copy;
}
