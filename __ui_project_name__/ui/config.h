#pragma once
#include <Arduino.h>

// ================= KONEKSI JARINGAN =================
#define WIFI_SSID         "iot_bus"
#define WIFI_PASS         "iot_BUS@5758"
#define MASTER_MDNS_HOST  "bus-led"
#define MASTER_UDP_PORT   5758
#define MASTER_WS_PORT    81

// ================= RESOLUSI LAYAR =================
#define LCD_WIDTH   800
#define LCD_HEIGHT  480

// ================= HARDWARE PROFILE =================
// Waveshare ESP32-S3 Touch LCD 4.3" (800x480 RGB Parallel + GT911 + CH422G)
// RGB bus, touch, dan backlight ditangani via driver waveshare di libraries/.
