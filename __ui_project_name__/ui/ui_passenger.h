#pragma once
#include <lvgl.h>
#include <Arduino.h>

struct SeatData {
    char id[8];
    bool occupied;
    bool seatbelt;
    bool calling;
    bool online;
    bool isPaired;
};

void ui_init();
void ui_update_seat(const char* seatId, bool occupied, bool seatbelt, bool calling);
void ui_set_seat_paired(const char* seatId, bool isPaired);
void ui_update_voltage(float voltage);
void ui_update_network_status(bool isConnected, const char* ipStr);
void ui_update_clock(const char* timeStr);
void ui_refresh_statistics();

// Roster Full Synchronization (Anti-Stale / Reset Kursi yang Di-Unpair)
void ui_sync_roster_begin();
void ui_sync_roster_seat(const char* seatId, bool occupied, bool seatbelt, bool calling);
void ui_sync_roster_end();

// Modal Daftar Kursi
void ui_open_daftar_kursi_modal();
void ui_close_daftar_kursi_modal();

// Pop-up Pairing Kursi Baru (Saat Modul RandomID Ditekan / Trigger)
void ui_show_pairing_dialog(const char* uid);
void ui_close_pairing_dialog();
void ui_on_pairing_result(bool success, const char* seatId, const char* errorMsg);
