/*
 * LVGL v8 Dashboard Interface for ESP32-S3 Waveshare 4.3" (800x480)
 * Ported from code.html (CORE-S3 MONITOR)
 */

#include "ui_Screen1.h"

lv_obj_t * ui_Screen1 = NULL;

// Dynamic Dashboard Metric Labels
lv_obj_t * ui_lbl_sys_status = NULL;
lv_obj_t * ui_lbl_voltage_val = NULL;
lv_obj_t * ui_lbl_voltage_delta = NULL;
lv_obj_t * ui_lbl_current_val = NULL;
lv_obj_t * ui_lbl_current_tag = NULL;
lv_obj_t * ui_lbl_resistance_val = NULL;
lv_obj_t * ui_lbl_temp_val = NULL;
lv_obj_t * ui_lbl_humidity_val = NULL;

// Navigation Tabs
lv_obj_t * ui_btn_tab_dashboard = NULL;
lv_obj_t * ui_btn_tab_storage = NULL;
lv_obj_t * ui_btn_tab_thermals = NULL;
lv_obj_t * ui_btn_tab_system = NULL;

static void create_card_style(lv_obj_t * obj, uint32_t bg_hex, uint32_t border_hex, int16_t radius)
{
    lv_obj_set_style_bg_color(obj, lv_color_hex(bg_hex), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(border_hex), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, 10, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

void ui_Screen1_screen_init(void)
{
    // Color Constants (Cyber-Industrial Theme)
    const uint32_t COLOR_BG           = 0x0B1326;
    const uint32_t COLOR_SURFACE      = 0x171F33;
    const uint32_t COLOR_WELL         = 0x131B2E;
    const uint32_t COLOR_SURFACE_HIGH = 0x222A3D;
    const uint32_t COLOR_OUTLINE      = 0x424754;
    const uint32_t COLOR_TEXT_MAIN    = 0xDAE2FD;
    const uint32_t COLOR_TEXT_MUTED   = 0x8C909F;
    const uint32_t COLOR_PRIMARY      = 0xADC6FF;
    const uint32_t COLOR_PRIMARY_ACC  = 0x4D8EFF;
    const uint32_t COLOR_SECONDARY    = 0x4EDEA3;
    const uint32_t COLOR_SEC_CONTAIN  = 0x00A572;
    const uint32_t COLOR_ALERT        = 0xFF5451;
    const uint32_t COLOR_ALERT_LIGHT  = 0xFFB3AD;

    // 1. Root Screen
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_set_size(ui_Screen1, 800, 480);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(ui_Screen1, 0, 0);

    // ==========================================
    // 2. HEADER TOP BAR (x:0, y:0, w:800, h:48)
    // ==========================================
    lv_obj_t * header = lv_obj_create(ui_Screen1);
    lv_obj_set_size(header, 800, 48);
    lv_obj_set_pos(header, 0, 0);
    create_card_style(header, COLOR_BG, COLOR_OUTLINE, 0);
    lv_obj_set_style_pad_hor(header, 16, 0);

    // Title Icon & Label (Left)
    lv_obj_t * header_title = lv_label_create(header);
    lv_label_set_text(header_title, LV_SYMBOL_DRIVE " CORE-S3 MONITOR");
    lv_obj_set_style_text_font(header_title, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(header_title, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_pos(header_title, 0, 4);

    // Global Status Pill (Center)
    lv_obj_t * status_pill = lv_obj_create(header);
    lv_obj_set_size(status_pill, 140, 28);
    lv_obj_set_pos(status_pill, 330, 0);
    create_card_style(status_pill, COLOR_SURFACE, COLOR_SECONDARY, 14);
    lv_obj_set_style_pad_all(status_pill, 4, 0);

    lv_obj_t * status_led = lv_obj_create(status_pill);
    lv_obj_set_size(status_led, 8, 8);
    lv_obj_set_pos(status_led, 8, 5);
    create_card_style(status_led, COLOR_SECONDARY, COLOR_SECONDARY, 4);

    ui_lbl_sys_status = lv_label_create(status_pill);
    lv_label_set_text(ui_lbl_sys_status, "SYS_NORMAL");
    lv_obj_set_style_text_font(ui_lbl_sys_status, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_sys_status, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_pos(ui_lbl_sys_status, 24, 2);

    // Settings Button (Right)
    lv_obj_t * btn_settings = lv_label_create(header);
    lv_label_set_text(btn_settings, LV_SYMBOL_SETTINGS);
    lv_obj_set_style_text_font(btn_settings, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(btn_settings, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_pos(btn_settings, 745, 4);

    // ==========================================
    // 3. MAIN CONTENT GRID (y:54, h:352)
    // ==========================================

    // ------------ LEFT COLUMN (w:440) ------------
    // CARD 1: BATTERY VOLTAGE
    lv_obj_t * card_bat = lv_obj_create(ui_Screen1);
    lv_obj_set_size(card_bat, 440, 168);
    lv_obj_set_pos(card_bat, 16, 56);
    create_card_style(card_bat, COLOR_SURFACE, COLOR_OUTLINE, 6);

    lv_obj_t * lbl_bat_title = lv_label_create(card_bat);
    lv_label_set_text(lbl_bat_title, "BATTERY VOLTAGE");
    lv_obj_set_style_text_font(lbl_bat_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_bat_title, lv_color_hex(COLOR_TEXT_MUTED), 0);

    lv_obj_t * icon_bat = lv_label_create(card_bat);
    lv_label_set_text(icon_bat, LV_SYMBOL_CHARGE);
    lv_obj_set_style_text_font(icon_bat, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(icon_bat, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_set_pos(icon_bat, 400, 0);

    // Well Area
    lv_obj_t * well_bat = lv_obj_create(card_bat);
    lv_obj_set_size(well_bat, 416, 116);
    lv_obj_set_pos(well_bat, 0, 24);
    create_card_style(well_bat, COLOR_WELL, COLOR_OUTLINE, 8);

    ui_lbl_voltage_val = lv_label_create(well_bat);
    lv_label_set_text(ui_lbl_voltage_val, "84.2");
    lv_obj_set_style_text_font(ui_lbl_voltage_val, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(ui_lbl_voltage_val, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_pos(ui_lbl_voltage_val, 8, 8);

    lv_obj_t * lbl_v_unit = lv_label_create(well_bat);
    lv_label_set_text(lbl_v_unit, "V");
    lv_obj_set_style_text_font(lbl_v_unit, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_v_unit, lv_color_hex(COLOR_PRIMARY_ACC), 0);
    lv_obj_set_pos(lbl_v_unit, 140, 36);

    ui_lbl_voltage_delta = lv_label_create(well_bat);
    lv_label_set_text(ui_lbl_voltage_delta, LV_SYMBOL_UP " +0.4V/hr");
    lv_obj_set_style_text_font(ui_lbl_voltage_delta, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_voltage_delta, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_pos(ui_lbl_voltage_delta, 12, 68);

    // Sparkline Bars
    const int bar_heights[7] = { 20, 28, 38, 52, 65, 80, 14 };
    const uint32_t bar_colors[7] = { COLOR_SURFACE_HIGH, COLOR_SURFACE_HIGH, COLOR_PRIMARY_ACC, COLOR_PRIMARY_ACC, COLOR_PRIMARY, COLOR_PRIMARY, COLOR_SURFACE_HIGH };
    for (int i = 0; i < 7; i++) {
        lv_obj_t * bar = lv_obj_create(well_bat);
        lv_obj_set_size(bar, 8, bar_heights[i]);
        lv_obj_set_pos(bar, 270 + (i * 18), 85 - bar_heights[i]);
        create_card_style(bar, bar_colors[i], bar_colors[i], 2);
    }

    // CARD 2: LOAD CURRENT
    lv_obj_t * card_curr = lv_obj_create(ui_Screen1);
    lv_obj_set_size(card_curr, 440, 168);
    lv_obj_set_pos(card_curr, 16, 236);
    create_card_style(card_curr, COLOR_SURFACE, COLOR_OUTLINE, 6);

    lv_obj_t * lbl_curr_title = lv_label_create(card_curr);
    lv_label_set_text(lbl_curr_title, "LOAD CURRENT");
    lv_obj_set_style_text_font(lbl_curr_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_curr_title, lv_color_hex(COLOR_TEXT_MUTED), 0);

    lv_obj_t * icon_curr = lv_label_create(card_curr);
    lv_label_set_text(icon_curr, LV_SYMBOL_WARNING);
    lv_obj_set_style_text_font(icon_curr, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(icon_curr, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_set_pos(icon_curr, 400, 0);

    // Well Area
    lv_obj_t * well_curr = lv_obj_create(card_curr);
    lv_obj_set_size(well_curr, 416, 116);
    lv_obj_set_pos(well_curr, 0, 24);
    create_card_style(well_curr, COLOR_WELL, COLOR_OUTLINE, 8);

    ui_lbl_current_val = lv_label_create(well_curr);
    lv_label_set_text(ui_lbl_current_val, "142.5");
    lv_obj_set_style_text_font(ui_lbl_current_val, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(ui_lbl_current_val, lv_color_hex(COLOR_ALERT), 0);
    lv_obj_set_pos(ui_lbl_current_val, 8, 8);

    lv_obj_t * lbl_a_unit = lv_label_create(well_curr);
    lv_label_set_text(lbl_a_unit, "A");
    lv_obj_set_style_text_font(lbl_a_unit, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(lbl_a_unit, lv_color_hex(COLOR_ALERT), 0);
    lv_obj_set_pos(lbl_a_unit, 160, 36);

    ui_lbl_current_tag = lv_label_create(well_curr);
    lv_label_set_text(ui_lbl_current_tag, LV_SYMBOL_WARNING " High Load");
    lv_obj_set_style_text_font(ui_lbl_current_tag, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_current_tag, lv_color_hex(COLOR_ALERT), 0);
    lv_obj_set_pos(ui_lbl_current_tag, 12, 68);

    // Linear Gauge Bar
    lv_obj_t * gauge_bg = lv_obj_create(well_curr);
    lv_obj_set_size(gauge_bg, 140, 16);
    lv_obj_set_pos(gauge_bg, 250, 42);
    create_card_style(gauge_bg, COLOR_SURFACE_HIGH, COLOR_OUTLINE, 8);
    lv_obj_set_style_pad_all(gauge_bg, 0, 0);

    lv_obj_t * g_seg1 = lv_obj_create(gauge_bg);
    lv_obj_set_size(g_seg1, 45, 14);
    lv_obj_set_pos(g_seg1, 0, 0);
    create_card_style(g_seg1, COLOR_SECONDARY, COLOR_SECONDARY, 0);

    lv_obj_t * g_seg2 = lv_obj_create(gauge_bg);
    lv_obj_set_size(g_seg2, 45, 14);
    lv_obj_set_pos(g_seg2, 46, 0);
    create_card_style(g_seg2, COLOR_PRIMARY_ACC, COLOR_PRIMARY_ACC, 0);

    lv_obj_t * g_seg3 = lv_obj_create(gauge_bg);
    lv_obj_set_size(g_seg3, 35, 14);
    lv_obj_set_pos(g_seg3, 92, 0);
    create_card_style(g_seg3, COLOR_ALERT, COLOR_ALERT, 0);


    // ------------ RIGHT COLUMN (w:312) ------------
    // CARD 3: INT RESISTANCE
    lv_obj_t * card_res = lv_obj_create(ui_Screen1);
    lv_obj_set_size(card_res, 312, 106);
    lv_obj_set_pos(card_res, 472, 56);
    create_card_style(card_res, COLOR_SURFACE, COLOR_OUTLINE, 6);

    lv_obj_t * lbl_res_title = lv_label_create(card_res);
    lv_label_set_text(lbl_res_title, "INT RESISTANCE");
    lv_obj_set_style_text_font(lbl_res_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_res_title, lv_color_hex(COLOR_TEXT_MUTED), 0);

    ui_lbl_resistance_val = lv_label_create(card_res);
    lv_label_set_text(ui_lbl_resistance_val, "12.4 mOhm");
    lv_obj_set_style_text_font(ui_lbl_resistance_val, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui_lbl_resistance_val, lv_color_hex(COLOR_TEXT_MAIN), 0);
    lv_obj_set_pos(ui_lbl_resistance_val, 0, 32);

    lv_obj_t * badge_res = lv_obj_create(card_res);
    lv_obj_set_size(badge_res, 32, 32);
    lv_obj_set_pos(badge_res, 255, 24);
    create_card_style(badge_res, COLOR_SEC_CONTAIN, COLOR_SECONDARY, 16);

    lv_obj_t * ok_res = lv_label_create(badge_res);
    lv_label_set_text(ok_res, LV_SYMBOL_OK);
    lv_obj_set_style_text_font(ok_res, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(ok_res, lv_color_hex(COLOR_SECONDARY), 0);
    lv_obj_set_pos(ok_res, -1, -1);

    // CARD 4: CHASSIS TEMP
    lv_obj_t * card_temp = lv_obj_create(ui_Screen1);
    lv_obj_set_size(card_temp, 312, 106);
    lv_obj_set_pos(card_temp, 472, 174);
    create_card_style(card_temp, COLOR_SURFACE, COLOR_OUTLINE, 6);

    lv_obj_t * lbl_temp_title = lv_label_create(card_temp);
    lv_label_set_text(lbl_temp_title, "CHASSIS TEMP");
    lv_obj_set_style_text_font(lbl_temp_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_temp_title, lv_color_hex(COLOR_TEXT_MUTED), 0);

    ui_lbl_temp_val = lv_label_create(card_temp);
    lv_label_set_text(ui_lbl_temp_val, "42.8 degC");
    lv_obj_set_style_text_font(ui_lbl_temp_val, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui_lbl_temp_val, lv_color_hex(COLOR_ALERT_LIGHT), 0);
    lv_obj_set_pos(ui_lbl_temp_val, 0, 32);

    lv_obj_t * icon_temp = lv_label_create(card_temp);
    lv_label_set_text(icon_temp, "degC");
    lv_obj_set_style_text_font(icon_temp, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(icon_temp, lv_color_hex(COLOR_ALERT), 0);
    lv_obj_set_pos(icon_temp, 250, 32);

    // CARD 5: ENV HUMIDITY
    lv_obj_t * card_hum = lv_obj_create(ui_Screen1);
    lv_obj_set_size(card_hum, 312, 106);
    lv_obj_set_pos(card_hum, 472, 292);
    create_card_style(card_hum, COLOR_SURFACE, COLOR_OUTLINE, 6);

    lv_obj_t * lbl_hum_title = lv_label_create(card_hum);
    lv_label_set_text(lbl_hum_title, "ENV HUMIDITY");
    lv_obj_set_style_text_font(lbl_hum_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_hum_title, lv_color_hex(COLOR_TEXT_MUTED), 0);

    ui_lbl_humidity_val = lv_label_create(card_hum);
    lv_label_set_text(ui_lbl_humidity_val, "38.2 %");
    lv_obj_set_style_text_font(ui_lbl_humidity_val, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(ui_lbl_humidity_val, lv_color_hex(COLOR_PRIMARY), 0);
    lv_obj_set_pos(ui_lbl_humidity_val, 0, 32);

    lv_obj_t * icon_hum = lv_label_create(card_hum);
    lv_label_set_text(icon_hum, "%");
    lv_obj_set_style_text_font(icon_hum, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(icon_hum, lv_color_hex(COLOR_PRIMARY_ACC), 0);
    lv_obj_set_pos(icon_hum, 260, 32);

    // ==========================================
    // 4. BOTTOM NAVBAR (x:0, y:416, w:800, h:64)
    // ==========================================
    lv_obj_t * navbar = lv_obj_create(ui_Screen1);
    lv_obj_set_size(navbar, 800, 64);
    lv_obj_set_pos(navbar, 0, 416);
    create_card_style(navbar, COLOR_SURFACE, COLOR_OUTLINE, 0);
    lv_obj_set_style_pad_all(navbar, 8, 0);

    // TAB 1: DASHBOARD (Active)
    ui_btn_tab_dashboard = lv_btn_create(navbar);
    lv_obj_set_size(ui_btn_tab_dashboard, 175, 48);
    lv_obj_set_pos(ui_btn_tab_dashboard, 16, 0);
    create_card_style(ui_btn_tab_dashboard, COLOR_SEC_CONTAIN, COLOR_SECONDARY, 12);
    
    lv_obj_t * lbl_tab1 = lv_label_create(ui_btn_tab_dashboard);
    lv_label_set_text(lbl_tab1, LV_SYMBOL_HOME " DASHBOARD");
    lv_obj_set_style_text_font(lbl_tab1, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_tab1, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl_tab1);

    // TAB 2: STORAGE (Inactive)
    ui_btn_tab_storage = lv_btn_create(navbar);
    lv_obj_set_size(ui_btn_tab_storage, 175, 48);
    lv_obj_set_pos(ui_btn_tab_storage, 211, 0);
    create_card_style(ui_btn_tab_storage, COLOR_SURFACE, COLOR_SURFACE, 12);

    lv_obj_t * lbl_tab2 = lv_label_create(ui_btn_tab_storage);
    lv_label_set_text(lbl_tab2, LV_SYMBOL_CHARGE " STORAGE");
    lv_obj_set_style_text_font(lbl_tab2, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_tab2, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_center(lbl_tab2);

    // TAB 3: THERMALS (Inactive)
    ui_btn_tab_thermals = lv_btn_create(navbar);
    lv_obj_set_size(ui_btn_tab_thermals, 175, 48);
    lv_obj_set_pos(ui_btn_tab_thermals, 406, 0);
    create_card_style(ui_btn_tab_thermals, COLOR_SURFACE, COLOR_SURFACE, 12);

    lv_obj_t * lbl_tab3 = lv_label_create(ui_btn_tab_thermals);
    lv_label_set_text(lbl_tab3, LV_SYMBOL_WARNING " THERMALS");
    lv_obj_set_style_text_font(lbl_tab3, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_tab3, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_center(lbl_tab3);

    // TAB 4: SYSTEM (Inactive)
    ui_btn_tab_system = lv_btn_create(navbar);
    lv_obj_set_size(ui_btn_tab_system, 175, 48);
    lv_obj_set_pos(ui_btn_tab_system, 601, 0);
    create_card_style(ui_btn_tab_system, COLOR_SURFACE, COLOR_SURFACE, 12);

    lv_obj_t * lbl_tab4 = lv_label_create(ui_btn_tab_system);
    lv_label_set_text(lbl_tab4, LV_SYMBOL_SETTINGS " SYSTEM");
    lv_obj_set_style_text_font(lbl_tab4, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_tab4, lv_color_hex(COLOR_TEXT_MUTED), 0);
    lv_obj_center(lbl_tab4);
}

void ui_Screen1_screen_destroy(void)
{
    if (ui_Screen1) {
        lv_obj_del(ui_Screen1);
        ui_Screen1 = NULL;
    }
    ui_lbl_sys_status = NULL;
    ui_lbl_voltage_val = NULL;
    ui_lbl_voltage_delta = NULL;
    ui_lbl_current_val = NULL;
    ui_lbl_current_tag = NULL;
    ui_lbl_resistance_val = NULL;
    ui_lbl_temp_val = NULL;
    ui_lbl_humidity_val = NULL;
    ui_btn_tab_dashboard = NULL;
    ui_btn_tab_storage = NULL;
    ui_btn_tab_thermals = NULL;
    ui_btn_tab_system = NULL;
}
