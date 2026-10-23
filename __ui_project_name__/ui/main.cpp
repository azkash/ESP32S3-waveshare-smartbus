#include <Arduino.h>
#include "config.h"
#include "display_driver.h"
#include "ui_passenger.h"
#include "network_manager.h"
#include "lvgl_port.h"

// Task FreeRTOS untuk Network (Core 0)
void NetworkTask(void *pvParameters) {
    network_init();
    for (;;) {
        network_loop();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Timer update jam fallback
static void clock_timer_cb(lv_timer_t *timer) {
    (void)timer;
    static uint32_t total_sec = 21 * 3600 + 5 * 60; // Start 21:05
    total_sec++;
    uint8_t h = (total_sec / 3600) % 24;
    uint8_t m = (total_sec % 3600) / 60;
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
    ui_update_clock(buf);
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n===============================================");
    Serial.println("=== SMARTBUS PASSENGER MONITOR ESP32-S3 ===");
    Serial.println("===============================================");

    // Inisialisasi Layar & Touch (Core 1)
    display_driver_init();

    // Inisialisasi UI LVGL (43 Kursi Passenger SmartBus)
    if (lvgl_port_lock(-1)) {
        ui_init();
        lv_timer_create(clock_timer_cb, 1000, NULL);
        lvgl_port_unlock();
    }

    // Jalankan Task Network di Core 0 agar tidak mengganggu render grafis 60 FPS
    xTaskCreatePinnedToCore(
        NetworkTask,
        "NetworkTask",
        8192,
        NULL,
        1,
        NULL,
        0
    );

    Serial.println("[BOOT] SmartBus Passenger Monitor Ready!\n");
}

void loop() {
    display_driver_loop();
    delay(5);
}
