#include "display_driver.h"
#include "config.h"
#include "rgb_lcd_port.h"
#include "gt911.h"
#include "lvgl_port.h"

void display_driver_init() {
    Serial.println("[Display] 1. Init GT911 Touch Controller...");
    esp_lcd_touch_handle_t tp_handle = touch_gt911_init();

    Serial.println("[Display] 2. Init Waveshare RGB LCD Driver (800x480)...");
    esp_lcd_panel_handle_t panel_handle = waveshare_esp32_s3_rgb_lcd_init();

    Serial.println("[Display] 3. Init LVGL Port & Task (Core 1)...");
    ESP_ERROR_CHECK(lvgl_port_init(panel_handle, tp_handle));

    Serial.println("[Display] 4. Turn ON LCD Backlight (CH422G IO2)...");
    wavesahre_rgb_lcd_bl_on();
    Serial.println("[Display] Waveshare Display & Touch Ready!");
}

void display_driver_loop() {
    // LVGL diproses secara asinkron di Core 1 melalui FreeRTOS task yang dibuat oleh lvgl_port_init()
}
