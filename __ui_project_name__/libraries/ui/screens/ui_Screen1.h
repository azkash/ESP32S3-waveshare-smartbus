// This file was generated for Seat Monitoring Bus UI
// LVGL version: 8.3.11 / 8.4
// Display: 800 x 480 RGB LCD

#ifndef UI_SCREEN1_H
#define UI_SCREEN1_H

#ifdef __cplusplus
extern "C" {
#endif

#include "../ui.h"

// SCREEN: ui_Screen1
void ui_Screen1_screen_init(void);
void ui_Screen1_screen_destroy(void);

extern lv_obj_t * ui_Screen1;

// Top Bar Labels & Buttons
extern lv_obj_t * ui_lbl_clock;
extern lv_obj_t * ui_lbl_loc;
extern lv_obj_t * ui_lbl_date;
extern lv_obj_t * ui_btn_wifi;
extern lv_obj_t * ui_btn_log;
extern lv_obj_t * ui_lbl_volt;

// Sub Bar / Title & Status
extern lv_obj_t * ui_lbl_title;
extern lv_obj_t * ui_lbl_sys_status;
extern lv_obj_t * ui_led_status;

// Footer Stat Counters
extern lv_obj_t * ui_lbl_total_val;
extern lv_obj_t * ui_lbl_occupied_val;
extern lv_obj_t * ui_lbl_empty_val;

// Modals
extern lv_obj_t * ui_modal_wifi;
extern lv_obj_t * ui_modal_log;
extern lv_obj_t * ui_lbl_log_content;
extern lv_obj_t * ui_lbl_wifi_status;

// Backward-compatibility labels
extern lv_obj_t * ui_lbl_voltage_val;
extern lv_obj_t * ui_lbl_temp_val;
extern lv_obj_t * ui_lbl_humidity_val;

// UI Helper / Controller APIs
void ui_set_clock(const char *time_str);
void ui_set_date(const char *date_str);
void ui_set_location(const char *loc_str);
void ui_set_voltage(float volts);
void ui_set_system_status(const char *status_str, bool online);

// Seat APIs
void ui_set_seat_occupied(int index, bool occupied);
void ui_set_seat_occupied_by_name(const char *seat_name, bool occupied);
bool ui_get_seat_occupied(int index);
int  ui_get_seat_count(void);
int  ui_get_occupied_count(void);
void ui_update_counters(void);
void ui_clear_all_seats(void);
void ui_occupy_all_seats(void);

// Modal & Log APIs
void ui_modal_wifi_open(void);
void ui_modal_wifi_close(void);
void ui_modal_log_open(void);
void ui_modal_log_close(void);
void ui_add_log_entry(const char *entry);
void ui_clear_logs(void);
void ui_set_wifi_info(const char *ssid, const char *ip_addr, const char *status_msg);

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif
