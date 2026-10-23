/*
 * LVGL v8 Seat Monitoring Bus Interface for ESP32-S3 Waveshare (800x480)
 * Ported from gemini-code-1791440611823.html with Modal Dialogs & Logging
 */

#include "ui_Screen1.h"
#include <stdio.h>
#include <string.h>

lv_obj_t * ui_Screen1 = NULL;

// Top Bar
lv_obj_t * ui_lbl_clock = NULL;
lv_obj_t * ui_lbl_loc = NULL;
lv_obj_t * ui_lbl_date = NULL;
lv_obj_t * ui_btn_wifi = NULL;
lv_obj_t * ui_btn_log = NULL;
lv_obj_t * ui_lbl_volt = NULL;

// Sub Bar
lv_obj_t * ui_lbl_title = NULL;
lv_obj_t * ui_lbl_sys_status = NULL;
lv_obj_t * ui_led_status = NULL;

// Footer Stat Values
lv_obj_t * ui_lbl_total_val = NULL;
lv_obj_t * ui_lbl_occupied_val = NULL;
lv_obj_t * ui_lbl_empty_val = NULL;

// Modals
lv_obj_t * ui_modal_wifi = NULL;
lv_obj_t * ui_modal_log = NULL;
lv_obj_t * ui_lbl_log_content = NULL;
lv_obj_t * ui_lbl_wifi_status = NULL;
lv_obj_t * ui_lbl_wifi_ssid = NULL;
lv_obj_t * ui_lbl_wifi_ip = NULL;
static lv_obj_t * g_log_scroll_box = NULL;

// Backward-compatibility references
lv_obj_t * ui_lbl_voltage_val = NULL;
lv_obj_t * ui_lbl_temp_val = NULL;
lv_obj_t * ui_lbl_humidity_val = NULL;

// Color Palette Constants
#define COLOR_BG                  0x050B14
#define COLOR_PANEL_BG            0x09111E
#define COLOR_PANEL_BORDER        0x162438
#define COLOR_BUS_BG              0x070E1A
#define COLOR_BUS_BORDER          0x152438
#define COLOR_HEADLIGHT           0xEAB308
#define COLOR_COCKPIT_BORDER      0x334E68
#define COLOR_COCKPIT_TEXT        0x486581
#define COLOR_DIVIDER             0x1A2C42
#define COLOR_SEAT_EMPTY_BG       0x091322
#define COLOR_SEAT_EMPTY_BORDER   0x00F2FE
#define COLOR_SEAT_EMPTY_TEXT     0x00F2FE
#define COLOR_SEAT_OCCUPIED_BG    0x200D13
#define COLOR_SEAT_OCCUPIED_BORDER 0xEF4444
#define COLOR_SEAT_OCCUPIED_TEXT   0xEF4444
#define COLOR_STAT_LABEL          0x4B647F
#define COLOR_STAT_UNIT           0x7B8EA3

// In-Memory Log Buffer
#define MAX_LOG_SIZE 3072
static char g_log_buffer[MAX_LOG_SIZE] =
    "[13:17:00] Sistem monitor kursi bus aktif.\n"
    "[13:17:00] Membaca konfigurasi armada: 43 jok.\n"
    "[13:17:01] Layar sentuh GT911 & driver RGB LCD siap.\n";

typedef struct {
    const char *id;
    uint8_t col; // 0 to 7
    uint8_t row; // 0 (top pair), 1 (mid pair), 2 (bot pair)
    uint8_t pos; // 0 (left), 1 (right)
    bool occupied;
    lv_obj_t *btn;
    lv_obj_t *headrest;
    lv_obj_t *lbl;
} ui_bus_seat_t;

// Exactly 43 passenger seats matching gemini-code-1791440611823.html
static ui_bus_seat_t g_bus_seats[43] = {
    // Bay 1 (Front Row)
    { "A",   0, 0, 0, false, NULL, NULL, NULL },
    { "B",   0, 0, 1, false, NULL, NULL, NULL },

    // Bay 2
    { "3B",  1, 0, 0, false, NULL, NULL, NULL },
    { "3A",  1, 0, 1, false, NULL, NULL, NULL },
    { "2B",  1, 1, 0, false, NULL, NULL, NULL },
    { "2A",  1, 1, 1, false, NULL, NULL, NULL },
    { "1B",  1, 2, 0, false, NULL, NULL, NULL },
    { "1A",  1, 2, 1, false, NULL, NULL, NULL },

    // Bay 3
    { "6B",  2, 0, 0, false, NULL, NULL, NULL },
    { "6A",  2, 0, 1, false, NULL, NULL, NULL },
    { "5B",  2, 1, 0, false, NULL, NULL, NULL },
    { "5A",  2, 1, 1, false, NULL, NULL, NULL },
    { "4B",  2, 2, 0, false, NULL, NULL, NULL },
    { "4A",  2, 2, 1, false, NULL, NULL, NULL },

    // Bay 4
    { "9B",  3, 0, 0, false, NULL, NULL, NULL },
    { "9A",  3, 0, 1, false, NULL, NULL, NULL },
    { "8B",  3, 1, 0, false, NULL, NULL, NULL },
    { "8A",  3, 1, 1, false, NULL, NULL, NULL },
    { "7B",  3, 2, 0, false, NULL, NULL, NULL },
    { "7A",  3, 2, 1, false, NULL, NULL, NULL },

    // Bay 5
    { "12B", 4, 0, 0, false, NULL, NULL, NULL },
    { "12A", 4, 0, 1, false, NULL, NULL, NULL },
    { "11B", 4, 1, 0, false, NULL, NULL, NULL },
    { "11A", 4, 1, 1, false, NULL, NULL, NULL },
    { "10A", 4, 2, 0, false, NULL, NULL, NULL },

    // Bay 6
    { "15B", 5, 0, 0, false, NULL, NULL, NULL },
    { "15A", 5, 0, 1, false, NULL, NULL, NULL },
    { "14B", 5, 1, 0, false, NULL, NULL, NULL },
    { "14A", 5, 1, 1, false, NULL, NULL, NULL },
    { "13B", 5, 2, 0, false, NULL, NULL, NULL },
    { "13A", 5, 2, 1, false, NULL, NULL, NULL },

    // Bay 7
    { "18B", 6, 0, 0, false, NULL, NULL, NULL },
    { "18A", 6, 0, 1, false, NULL, NULL, NULL },
    { "17B", 6, 1, 0, false, NULL, NULL, NULL },
    { "17A", 6, 1, 1, false, NULL, NULL, NULL },
    { "16B", 6, 2, 0, false, NULL, NULL, NULL },
    { "16A", 6, 2, 1, false, NULL, NULL, NULL },

    // Bay 8 (Rear Row)
    { "21B", 7, 0, 0, false, NULL, NULL, NULL },
    { "21A", 7, 0, 1, false, NULL, NULL, NULL },
    { "20B", 7, 1, 0, false, NULL, NULL, NULL },
    { "20A", 7, 1, 1, false, NULL, NULL, NULL },
    { "19B", 7, 2, 0, false, NULL, NULL, NULL },
    { "19A", 7, 2, 1, false, NULL, NULL, NULL }
};

static void create_card_style(lv_obj_t * obj, uint32_t bg_hex, uint32_t border_hex, int16_t radius)
{
    if (!obj) return;
    lv_obj_set_style_bg_color(obj, lv_color_hex(bg_hex), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(obj, lv_color_hex(border_hex), 0);
    lv_obj_set_style_border_width(obj, 1, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
}

static void ui_apply_seat_style(ui_bus_seat_t * seat)
{
    if (!seat || !seat->btn) return;
    if (seat->occupied) {
        lv_obj_set_style_bg_color(seat->btn, lv_color_hex(COLOR_SEAT_OCCUPIED_BG), 0);
        lv_obj_set_style_border_color(seat->btn, lv_color_hex(COLOR_SEAT_OCCUPIED_BORDER), 0);
        if (seat->lbl) {
            lv_obj_set_style_text_color(seat->lbl, lv_color_hex(COLOR_SEAT_OCCUPIED_TEXT), 0);
        }
        if (seat->headrest) {
            lv_obj_set_style_bg_color(seat->headrest, lv_color_hex(COLOR_SEAT_OCCUPIED_BORDER), 0);
            lv_obj_set_style_border_color(seat->headrest, lv_color_hex(COLOR_SEAT_OCCUPIED_BORDER), 0);
        }
    } else {
        lv_obj_set_style_bg_color(seat->btn, lv_color_hex(COLOR_SEAT_EMPTY_BG), 0);
        lv_obj_set_style_border_color(seat->btn, lv_color_hex(COLOR_SEAT_EMPTY_BORDER), 0);
        if (seat->lbl) {
            lv_obj_set_style_text_color(seat->lbl, lv_color_hex(COLOR_SEAT_EMPTY_TEXT), 0);
        }
        if (seat->headrest) {
            lv_obj_set_style_bg_color(seat->headrest, lv_color_hex(COLOR_SEAT_EMPTY_BORDER), 0);
            lv_obj_set_style_border_color(seat->headrest, lv_color_hex(COLOR_SEAT_EMPTY_BORDER), 0);
        }
    }
}

void ui_update_counters(void)
{
    int occ = 0;
    for (int i = 0; i < 43; i++) {
        if (g_bus_seats[i].occupied) {
            occ++;
        }
    }
    int empty = 43 - occ;

    if (ui_lbl_occupied_val) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", occ);
        lv_label_set_text(ui_lbl_occupied_val, buf);
    }
    if (ui_lbl_empty_val) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", empty);
        lv_label_set_text(ui_lbl_empty_val, buf);
    }
}

static void seat_click_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        ui_bus_seat_t * seat = (ui_bus_seat_t *)lv_event_get_user_data(e);
        if (seat) {
            seat->occupied = !seat->occupied;
            ui_apply_seat_style(seat);
            ui_update_counters();

            // Record into live log
            const char * clock_str = ui_lbl_clock ? lv_label_get_text(ui_lbl_clock) : "13:17";
            char log_entry[96];
            snprintf(log_entry, sizeof(log_entry), "[%s] Kursi %s: %s (%d Pax)\n",
                     clock_str, seat->id, seat->occupied ? "TERISI" : "KOSONG",
                     ui_get_occupied_count());
            ui_add_log_entry(log_entry);
        }
    }
}

// ── Modals Construction Forward Declarations ──
static void create_modal_wifi(lv_obj_t * parent);
static void create_modal_log(lv_obj_t * parent);
static void wifi_modal_open_btn_handler(lv_event_t * e);
static void log_modal_open_btn_handler(lv_event_t * e);

void ui_Screen1_screen_init(void)
{
    // 1. Root Screen (800 x 480)
    ui_Screen1 = lv_obj_create(NULL);
    lv_obj_set_size(ui_Screen1, 800, 480);
    lv_obj_set_style_bg_color(ui_Screen1, lv_color_hex(COLOR_BG), 0);
    lv_obj_set_style_bg_opa(ui_Screen1, LV_OPA_COVER, 0);
    lv_obj_clear_flag(ui_Screen1, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_pad_all(ui_Screen1, 0, 0);

    // =========================================================================
    // 2. TOP BAR (x: 12, y: 8, w: 776, h: 42)
    // =========================================================================
    lv_obj_t * top_bar = lv_obj_create(ui_Screen1);
    lv_obj_set_size(top_bar, 776, 42);
    lv_obj_set_pos(top_bar, 12, 8);
    create_card_style(top_bar, COLOR_PANEL_BG, COLOR_PANEL_BORDER, 8);

    // Clock Display (Left)
    ui_lbl_clock = lv_label_create(top_bar);
    lv_label_set_text(ui_lbl_clock, "13:17");
    lv_obj_set_style_text_font(ui_lbl_clock, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(ui_lbl_clock, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(ui_lbl_clock, 14, 4);

    // Meta-Data: Location & Date
    ui_lbl_loc = lv_label_create(top_bar);
    lv_label_set_text(ui_lbl_loc, LV_SYMBOL_GPS " LOCATING...");
    lv_obj_set_style_text_font(ui_lbl_loc, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(ui_lbl_loc, lv_color_hex(0x5D7593), 0);
    lv_obj_set_pos(ui_lbl_loc, 108, 6);

    ui_lbl_date = lv_label_create(top_bar);
    lv_label_set_text(ui_lbl_date, "SENIN, 28 SEP 2026");
    lv_obj_set_style_text_font(ui_lbl_date, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(ui_lbl_date, lv_color_hex(0x5D7593), 0);
    lv_obj_set_pos(ui_lbl_date, 108, 22);

    // Right Action: Wi-Fi Setting Button
    ui_btn_wifi = lv_btn_create(top_bar);
    lv_obj_set_size(ui_btn_wifi, 128, 28);
    lv_obj_set_pos(ui_btn_wifi, 432, 7);
    create_card_style(ui_btn_wifi, 0x0D2038, 0x1A385C, 6);
    lv_obj_add_flag(ui_btn_wifi, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * lbl_wifi = lv_label_create(ui_btn_wifi);
    lv_label_set_text(lbl_wifi, LV_SYMBOL_WIFI " SETTING WI-FI");
    lv_obj_set_style_text_font(lbl_wifi, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_wifi, lv_color_hex(0x7AA2CF), 0);
    lv_obj_center(lbl_wifi);

    // Right Action: Log Button
    ui_btn_log = lv_btn_create(top_bar);
    lv_obj_set_size(ui_btn_log, 80, 28);
    lv_obj_set_pos(ui_btn_log, 568, 7);
    create_card_style(ui_btn_log, 0x0D2038, 0x1A385C, 6);
    lv_obj_add_flag(ui_btn_log, LV_OBJ_FLAG_CLICKABLE);

    lv_obj_t * lbl_log = lv_label_create(ui_btn_log);
    lv_label_set_text(lbl_log, LV_SYMBOL_FILE " LOG");
    lv_obj_set_style_text_font(lbl_log, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_log, lv_color_hex(0x7AA2CF), 0);
    lv_obj_center(lbl_log);

    // Right Action: Voltage Badge
    lv_obj_t * badge_volt = lv_obj_create(top_bar);
    lv_obj_set_size(badge_volt, 98, 28);
    lv_obj_set_pos(badge_volt, 656, 7);
    create_card_style(badge_volt, 0x0C1C30, 0x1E3A5F, 6);

    ui_lbl_volt = lv_label_create(badge_volt);
    lv_label_set_text(ui_lbl_volt, LV_SYMBOL_CHARGE " 24.0V");
    lv_obj_set_style_text_font(ui_lbl_volt, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_volt, lv_color_hex(0x38BDF8), 0);
    lv_obj_center(ui_lbl_volt);

    // Map backwards-compatible label pointer
    ui_lbl_voltage_val = ui_lbl_volt;

    // =========================================================================
    // 3. SUB HEADER / TITLE BAR (x: 12, y: 54, w: 776, h: 28)
    // =========================================================================
    lv_obj_t * title_badge = lv_obj_create(ui_Screen1);
    lv_obj_set_size(title_badge, 210, 28);
    lv_obj_set_pos(title_badge, 12, 54);
    create_card_style(title_badge, 0x0D1A2D, 0x1C3352, 6);

    ui_lbl_title = lv_label_create(title_badge);
    lv_label_set_text(ui_lbl_title, "SEAT MONITORING BUS");
    lv_obj_set_style_text_font(ui_lbl_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(ui_lbl_title);

    // Status Pill (Right)
    lv_obj_t * status_badge = lv_obj_create(ui_Screen1);
    lv_obj_set_size(status_badge, 210, 28);
    lv_obj_set_pos(status_badge, 578, 54);
    create_card_style(status_badge, 0x002B36, 0x00E5FF, 14);

    ui_led_status = lv_obj_create(status_badge);
    lv_obj_set_size(ui_led_status, 8, 8);
    lv_obj_set_pos(ui_led_status, 10, 10);
    create_card_style(ui_led_status, 0x00E5FF, 0x00E5FF, 4);

    ui_lbl_sys_status = lv_label_create(status_badge);
    lv_label_set_text(ui_lbl_sys_status, "SISTEM ONLINE (BRIDGE)");
    lv_obj_set_style_text_font(ui_lbl_sys_status, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(ui_lbl_sys_status, lv_color_hex(0x00F2FE), 0);
    lv_obj_set_pos(ui_lbl_sys_status, 26, 7);

    // =========================================================================
    // 4. BUS FRAME (MAIN DECK) (x: 12, y: 86, w: 776, h: 282)
    // =========================================================================
    lv_obj_t * bus_frame = lv_obj_create(ui_Screen1);
    lv_obj_set_size(bus_frame, 776, 282);
    lv_obj_set_pos(bus_frame, 12, 86);
    create_card_style(bus_frame, COLOR_BUS_BG, COLOR_BUS_BORDER, 24);
    lv_obj_set_style_border_width(bus_frame, 2, 0);

    // Headlights (Front Left of the Bus)
    lv_obj_t * headlight_top = lv_obj_create(bus_frame);
    lv_obj_set_size(headlight_top, 6, 26);
    lv_obj_set_pos(headlight_top, 4, 18);
    create_card_style(headlight_top, COLOR_HEADLIGHT, COLOR_HEADLIGHT, 3);

    lv_obj_t * headlight_bot = lv_obj_create(bus_frame);
    lv_obj_set_size(headlight_bot, 6, 26);
    lv_obj_set_pos(headlight_bot, 4, 238);
    create_card_style(headlight_bot, COLOR_HEADLIGHT, COLOR_HEADLIGHT, 3);

    // Cockpit / Driver Cabin (Left Section)
    lv_obj_t * drv_seat = lv_obj_create(bus_frame);
    lv_obj_set_size(drv_seat, 44, 44);
    lv_obj_set_pos(drv_seat, 22, 16);
    create_card_style(drv_seat, COLOR_PANEL_BG, COLOR_COCKPIT_BORDER, 6);

    lv_obj_t * drv_lbl = lv_label_create(drv_seat);
    lv_label_set_text(drv_lbl, "DRV");
    lv_obj_set_style_text_font(drv_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(drv_lbl, lv_color_hex(COLOR_COCKPIT_TEXT), 0);
    lv_obj_center(drv_lbl);

    lv_obj_t * ron_seat = lv_obj_create(bus_frame);
    lv_obj_set_size(ron_seat, 44, 44);
    lv_obj_set_pos(ron_seat, 22, 222);
    create_card_style(ron_seat, COLOR_PANEL_BG, COLOR_COCKPIT_BORDER, 6);

    lv_obj_t * ron_lbl = lv_label_create(ron_seat);
    lv_label_set_text(ron_lbl, "RON");
    lv_obj_set_style_text_font(ron_lbl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ron_lbl, lv_color_hex(COLOR_COCKPIT_TEXT), 0);
    lv_obj_center(ron_lbl);

    // Cockpit Vertical Divider Line
    lv_obj_t * cockpit_div = lv_obj_create(bus_frame);
    lv_obj_set_size(cockpit_div, 2, 258);
    lv_obj_set_pos(cockpit_div, 78, 12);
    create_card_style(cockpit_div, COLOR_DIVIDER, COLOR_DIVIDER, 0);

    // Passenger Cabin Grid (8 Bays)
    const int cabin_start_x = 88;
    const int bay_w = 84;
    const int seat_w = 36;
    const int seat_h = 48;

    // Draw Subtle Bay Vertical Dividers
    for (int col = 0; col < 7; col++) {
        lv_obj_t * col_div = lv_obj_create(bus_frame);
        lv_obj_set_size(col_div, 1, 254);
        lv_obj_set_pos(col_div, cabin_start_x + ((col + 1) * bay_w) - 2, 14);
        create_card_style(col_div, COLOR_PANEL_BORDER, COLOR_PANEL_BORDER, 0);
    }

    // Row Y Coordinates:
    const int row_y_coords[3] = { 14, 117, 220 };

    // Instantiate All 43 Passenger Seats
    for (int i = 0; i < 43; i++) {
        ui_bus_seat_t * seat = &g_bus_seats[i];

        int col_x = cabin_start_x + (seat->col * bay_w);
        int seat_x = (seat->pos == 0) ? (col_x + 4) : (col_x + 44);
        int seat_y = row_y_coords[seat->row];

        if (strcmp(seat->id, "10A") == 0) {
            seat_x = col_x + 4;
        }

        // Seat Container Button
        seat->btn = lv_btn_create(bus_frame);
        lv_obj_set_size(seat->btn, seat_w, seat_h);
        lv_obj_set_pos(seat->btn, seat_x, seat_y);
        create_card_style(seat->btn, COLOR_SEAT_EMPTY_BG, COLOR_SEAT_EMPTY_BORDER, 6);
        lv_obj_set_style_border_width(seat->btn, 2, 0);
        lv_obj_add_flag(seat->btn, LV_OBJ_FLAG_CLICKABLE);

        // Seat Headrest Bar
        seat->headrest = lv_obj_create(seat->btn);
        lv_obj_set_size(seat->headrest, 18, 4);
        lv_obj_align(seat->headrest, LV_ALIGN_TOP_MID, 0, 0);
        create_card_style(seat->headrest, COLOR_SEAT_EMPTY_BORDER, COLOR_SEAT_EMPTY_BORDER, 2);
        lv_obj_clear_flag(seat->headrest, LV_OBJ_FLAG_CLICKABLE);

        // Seat Identifier Label
        seat->lbl = lv_label_create(seat->btn);
        lv_label_set_text(seat->lbl, seat->id);
        lv_obj_set_style_text_font(seat->lbl, &lv_font_montserrat_10, 0);
        lv_obj_set_style_text_color(seat->lbl, lv_color_hex(COLOR_SEAT_EMPTY_TEXT), 0);
        lv_obj_align(seat->lbl, LV_ALIGN_CENTER, 0, 2);
        lv_obj_clear_flag(seat->lbl, LV_OBJ_FLAG_CLICKABLE);

        // Attach Click Event Handler
        lv_obj_add_event_cb(seat->btn, seat_click_event_handler, LV_EVENT_CLICKED, seat);
    }

    // =========================================================================
    // 5. STATS FOOTER (SUMMARY BAR) (x: 12, y: 374, w: 776, h: 96)
    // =========================================================================
    lv_obj_t * stats_footer = lv_obj_create(ui_Screen1);
    lv_obj_set_size(stats_footer, 776, 96);
    lv_obj_set_pos(stats_footer, 12, 374);
    create_card_style(stats_footer, COLOR_PANEL_BG, COLOR_PANEL_BORDER, 10);

    // Card 1: Kapasitas Armada
    lv_obj_t * card_total = lv_obj_create(stats_footer);
    lv_obj_set_size(card_total, 248, 86);
    lv_obj_set_pos(card_total, 4, 4);
    create_card_style(card_total, 0x060D18, COLOR_PANEL_BORDER, 8);

    lv_obj_t * lbl_tot_title = lv_label_create(card_total);
    lv_label_set_text(lbl_tot_title, "KAPASITAS ARMADA");
    lv_obj_set_style_text_font(lbl_tot_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_tot_title, lv_color_hex(COLOR_STAT_LABEL), 0);
    lv_obj_align(lbl_tot_title, LV_ALIGN_TOP_MID, 0, 10);

    ui_lbl_total_val = lv_label_create(card_total);
    lv_label_set_text(ui_lbl_total_val, "43");
    lv_obj_set_style_text_font(ui_lbl_total_val, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(ui_lbl_total_val, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(ui_lbl_total_val, LV_ALIGN_CENTER, -18, 12);

    lv_obj_t * lbl_tot_unit = lv_label_create(card_total);
    lv_label_set_text(lbl_tot_unit, "Jok");
    lv_obj_set_style_text_font(lbl_tot_unit, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_tot_unit, lv_color_hex(COLOR_STAT_UNIT), 0);
    lv_obj_align(lbl_tot_unit, LV_ALIGN_CENTER, 28, 15);

    // Card 2: Terisi
    lv_obj_t * card_occ = lv_obj_create(stats_footer);
    lv_obj_set_size(card_occ, 248, 86);
    lv_obj_set_pos(card_occ, 260, 4);
    create_card_style(card_occ, 0x1A0B10, 0x44161C, 8);

    lv_obj_t * lbl_occ_title = lv_label_create(card_occ);
    lv_label_set_text(lbl_occ_title, "TERISI (OCCUPIED)");
    lv_obj_set_style_text_font(lbl_occ_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_occ_title, lv_color_hex(COLOR_SEAT_OCCUPIED_BORDER), 0);
    lv_obj_align(lbl_occ_title, LV_ALIGN_TOP_MID, 0, 10);

    ui_lbl_occupied_val = lv_label_create(card_occ);
    lv_label_set_text(ui_lbl_occupied_val, "0");
    lv_obj_set_style_text_font(ui_lbl_occupied_val, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(ui_lbl_occupied_val, lv_color_hex(COLOR_SEAT_OCCUPIED_BORDER), 0);
    lv_obj_align(ui_lbl_occupied_val, LV_ALIGN_CENTER, -18, 12);

    lv_obj_t * lbl_occ_unit = lv_label_create(card_occ);
    lv_label_set_text(lbl_occ_unit, "Pax");
    lv_obj_set_style_text_font(lbl_occ_unit, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_occ_unit, lv_color_hex(0xFCA5A5), 0);
    lv_obj_align(lbl_occ_unit, LV_ALIGN_CENTER, 28, 15);

    // Card 3: Kosong
    lv_obj_t * card_emp = lv_obj_create(stats_footer);
    lv_obj_set_size(card_emp, 248, 86);
    lv_obj_set_pos(card_emp, 516, 4);
    create_card_style(card_emp, 0x031822, 0x0C3549, 8);

    lv_obj_t * lbl_emp_title = lv_label_create(card_emp);
    lv_label_set_text(lbl_emp_title, "KOSONG (AVAILABLE)");
    lv_obj_set_style_text_font(lbl_emp_title, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_emp_title, lv_color_hex(0x00E5FF), 0);
    lv_obj_align(lbl_emp_title, LV_ALIGN_TOP_MID, 0, 10);

    ui_lbl_empty_val = lv_label_create(card_emp);
    lv_label_set_text(ui_lbl_empty_val, "43");
    lv_obj_set_style_text_font(ui_lbl_empty_val, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(ui_lbl_empty_val, lv_color_hex(0x00E5FF), 0);
    lv_obj_align(ui_lbl_empty_val, LV_ALIGN_CENTER, -18, 12);

    lv_obj_t * lbl_emp_unit = lv_label_create(card_emp);
    lv_label_set_text(lbl_emp_unit, "Jok");
    lv_obj_set_style_text_font(lbl_emp_unit, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_emp_unit, lv_color_hex(0x7DD3FC), 0);
    lv_obj_align(lbl_emp_unit, LV_ALIGN_CENTER, 28, 15);

    // Initial counter sync
    ui_update_counters();

    // =========================================================================
    // 6. MODAL POPUPS (WI-FI & LOG) - LAZY LOADED ON DEMAND
    // =========================================================================
    // Modals are created on-demand when the user clicks the buttons

    // Attach Top Bar Button Click Handlers
    lv_obj_add_event_cb(ui_btn_wifi, wifi_modal_open_btn_handler, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(ui_btn_log, log_modal_open_btn_handler, LV_EVENT_CLICKED, NULL);
}

// =========================================================================
// MODAL BUILDERS
// =========================================================================

static void wifi_modal_open_btn_handler(lv_event_t * e)
{
    (void)e;
    ui_modal_wifi_open();
}

static void log_modal_open_btn_handler(lv_event_t * e)
{
    (void)e;
    ui_modal_log_open();
}

static void wifi_modal_close_btn_handler(lv_event_t * e)
{
    (void)e;
    ui_modal_wifi_close();
}

static void log_modal_close_btn_handler(lv_event_t * e)
{
    (void)e;
    ui_modal_log_close();
}

static void log_modal_clear_btn_handler(lv_event_t * e)
{
    (void)e;
    ui_clear_logs();
}

static void log_modal_reset_seats_handler(lv_event_t * e)
{
    (void)e;
    ui_clear_all_seats();
    ui_add_log_entry("[SYSTEM] Semua kursi di-reset ke status KOSONG.\n");
}

static void create_modal_wifi(lv_obj_t * parent)
{
    // Full screen overlay backdrop
    ui_modal_wifi = lv_obj_create(parent);
    lv_obj_set_size(ui_modal_wifi, 800, 480);
    lv_obj_set_pos(ui_modal_wifi, 0, 0);
    lv_obj_set_style_bg_color(ui_modal_wifi, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(ui_modal_wifi, LV_OPA_70, 0);
    lv_obj_set_style_border_width(ui_modal_wifi, 0, 0);
    lv_obj_set_style_pad_all(ui_modal_wifi, 0, 0);
    lv_obj_clear_flag(ui_modal_wifi, LV_OBJ_FLAG_SCROLLABLE);

    // Inner Card
    lv_obj_t * card = lv_obj_create(ui_modal_wifi);
    lv_obj_set_size(card, 560, 360);
    lv_obj_center(card);
    create_card_style(card, 0x091424, 0x1E3A5F, 14);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_pad_all(card, 16, 0);

    // Title Bar
    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_WIFI " PENGATURAN WI-FI & JARINGAN");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00F2FE), 0);
    lv_obj_set_pos(title, 4, 4);

    // Close Button [X]
    lv_obj_t * btn_x = lv_btn_create(card);
    lv_obj_set_size(btn_x, 32, 32);
    lv_obj_set_pos(btn_x, 492, 0);
    create_card_style(btn_x, 0x1C2D42, 0x2C4566, 6);
    lv_obj_t * lbl_x = lv_label_create(btn_x);
    lv_label_set_text(lbl_x, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_font(lbl_x, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_x, lv_color_hex(0x8C909F), 0);
    lv_obj_center(lbl_x);
    lv_obj_add_event_cb(btn_x, wifi_modal_close_btn_handler, LV_EVENT_CLICKED, NULL);

    // Network Info Box
    lv_obj_t * info_box = lv_obj_create(card);
    lv_obj_set_size(info_box, 528, 100);
    lv_obj_set_pos(info_box, 0, 44);
    create_card_style(info_box, 0x060E1A, 0x162438, 8);
    lv_obj_set_style_pad_all(info_box, 10, 0);

    // Mode Pill
    lv_obj_t * mode_pill = lv_obj_create(info_box);
    lv_obj_set_size(mode_pill, 140, 22);
    lv_obj_set_pos(mode_pill, 368, 6);
    create_card_style(mode_pill, 0x002B36, 0x00A572, 10);
    lv_obj_t * lbl_mode = lv_label_create(mode_pill);
    lv_label_set_text(lbl_mode, "HOTSPOT AKTIF");
    lv_obj_set_style_text_font(lbl_mode, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_mode, lv_color_hex(0x4EDEA3), 0);
    lv_obj_center(lbl_mode);

    ui_lbl_wifi_ssid = lv_label_create(info_box);
    lv_label_set_text(ui_lbl_wifi_ssid, "SSID: SeatMonitor-Bus-01");
    lv_obj_set_style_text_font(ui_lbl_wifi_ssid, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_wifi_ssid, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(ui_lbl_wifi_ssid, 8, 8);

    ui_lbl_wifi_ip = lv_label_create(info_box);
    lv_label_set_text(ui_lbl_wifi_ip, "IP: 192.168.4.1 (Gateway Ready)");
    lv_obj_set_style_text_font(ui_lbl_wifi_ip, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_wifi_ip, lv_color_hex(0x38BDF8), 0);
    lv_obj_set_pos(ui_lbl_wifi_ip, 8, 32);

    ui_lbl_wifi_status = lv_label_create(info_box);
    lv_label_set_text(ui_lbl_wifi_status, "Web Server: http://192.168.4.1/ (Siap diakses dari HP)");
    lv_obj_set_style_text_font(ui_lbl_wifi_status, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(ui_lbl_wifi_status, lv_color_hex(0x8C909F), 0);
    lv_obj_set_pos(ui_lbl_wifi_status, 8, 58);

    // Action Buttons
    lv_obj_t * btn_scan = lv_btn_create(card);
    lv_obj_set_size(btn_scan, 256, 38);
    lv_obj_set_pos(btn_scan, 0, 154);
    create_card_style(btn_scan, 0x0D223D, 0x1E4675, 8);
    lv_obj_t * lbl_sc = lv_label_create(btn_scan);
    lv_label_set_text(lbl_sc, LV_SYMBOL_REFRESH " PINDAI WI-FI (SCAN)");
    lv_obj_set_style_text_font(lbl_sc, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_sc, lv_color_hex(0x7DD3FC), 0);
    lv_obj_center(lbl_sc);

    lv_obj_t * btn_ap = lv_btn_create(card);
    lv_obj_set_size(btn_ap, 256, 38);
    lv_obj_set_pos(btn_ap, 272, 154);
    create_card_style(btn_ap, 0x0D223D, 0x1E4675, 8);
    lv_obj_t * lbl_ap = lv_label_create(btn_ap);
    lv_label_set_text(lbl_ap, LV_SYMBOL_WIFI " RESTART HOTSPOT");
    lv_obj_set_style_text_font(lbl_ap, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_ap, lv_color_hex(0x7DD3FC), 0);
    lv_obj_center(lbl_ap);

    // Status Panel / Instructions
    lv_obj_t * note_box = lv_obj_create(card);
    lv_obj_set_size(note_box, 528, 70);
    lv_obj_set_pos(note_box, 0, 202);
    create_card_style(note_box, 0x060E1A, 0x162438, 8);
    lv_obj_set_style_pad_all(note_box, 8, 0);

    lv_obj_t * lbl_note = lv_label_create(note_box);
    lv_label_set_text(lbl_note,
        "- Sambungkan smartphone/laptop ke WiFi \"SeatMonitor-Bus-01\"\n"
        "- Buka browser ke alamat http://192.168.4.1 untuk live seat map\n"
        "- Status kursi terupdate otomatis dua arah secara real-time");
    lv_obj_set_style_text_font(lbl_note, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(lbl_note, lv_color_hex(0x8C909F), 0);
    lv_obj_set_pos(lbl_note, 4, 4);

    // Bottom Close Button
    lv_obj_t * btn_close = lv_btn_create(card);
    lv_obj_set_size(btn_close, 180, 36);
    lv_obj_set_pos(btn_close, 174, 282);
    create_card_style(btn_close, 0x00A572, 0x4EDEA3, 8);
    lv_obj_t * lbl_cls = lv_label_create(btn_close);
    lv_label_set_text(lbl_cls, LV_SYMBOL_OK " TUTUP");
    lv_obj_set_style_text_font(lbl_cls, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_cls, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl_cls);
    lv_obj_add_event_cb(btn_close, wifi_modal_close_btn_handler, LV_EVENT_CLICKED, NULL);

    // Hidden by default
    lv_obj_add_flag(ui_modal_wifi, LV_OBJ_FLAG_HIDDEN);
}

static void create_modal_log(lv_obj_t * parent)
{
    // Full screen overlay backdrop
    ui_modal_log = lv_obj_create(parent);
    lv_obj_set_size(ui_modal_log, 800, 480);
    lv_obj_set_pos(ui_modal_log, 0, 0);
    lv_obj_set_style_bg_color(ui_modal_log, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(ui_modal_log, LV_OPA_70, 0);
    lv_obj_set_style_border_width(ui_modal_log, 0, 0);
    lv_obj_set_style_pad_all(ui_modal_log, 0, 0);
    lv_obj_clear_flag(ui_modal_log, LV_OBJ_FLAG_SCROLLABLE);

    // Inner Card
    lv_obj_t * card = lv_obj_create(ui_modal_log);
    lv_obj_set_size(card, 640, 400);
    lv_obj_center(card);
    create_card_style(card, 0x091424, 0x1E3A5F, 14);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_pad_all(card, 16, 0);

    // Title Bar
    lv_obj_t * title = lv_label_create(card);
    lv_label_set_text(title, LV_SYMBOL_FILE " LOG RIWAYAT KURSI PENUMPANG");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0x00F2FE), 0);
    lv_obj_set_pos(title, 4, 4);

    // Close Button [X]
    lv_obj_t * btn_x = lv_btn_create(card);
    lv_obj_set_size(btn_x, 32, 32);
    lv_obj_set_pos(btn_x, 572, 0);
    create_card_style(btn_x, 0x1C2D42, 0x2C4566, 6);
    lv_obj_t * lbl_x = lv_label_create(btn_x);
    lv_label_set_text(lbl_x, LV_SYMBOL_CLOSE);
    lv_obj_set_style_text_font(lbl_x, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_x, lv_color_hex(0x8C909F), 0);
    lv_obj_center(lbl_x);
    lv_obj_add_event_cb(btn_x, log_modal_close_btn_handler, LV_EVENT_CLICKED, NULL);

    // Info Strip
    lv_obj_t * info_strip = lv_label_create(card);
    lv_label_set_text(info_strip, "Armada: 43 Jok  |  Media: RAM Ring Buffer & SD Card  |  Real-time Event Logger");
    lv_obj_set_style_text_font(info_strip, &lv_font_montserrat_10, 0);
    lv_obj_set_style_text_color(info_strip, lv_color_hex(0x5D7593), 0);
    lv_obj_set_pos(info_strip, 4, 38);

    // Scrollable Log View
    g_log_scroll_box = lv_obj_create(card);
    lv_obj_set_size(g_log_scroll_box, 608, 240);
    lv_obj_set_pos(g_log_scroll_box, 0, 60);
    create_card_style(g_log_scroll_box, 0x050C16, 0x162438, 8);
    lv_obj_set_style_pad_all(g_log_scroll_box, 10, 0);
    lv_obj_add_flag(g_log_scroll_box, LV_OBJ_FLAG_SCROLLABLE);

    ui_lbl_log_content = lv_label_create(g_log_scroll_box);
    lv_label_set_text(ui_lbl_log_content, g_log_buffer);
    lv_obj_set_style_text_font(ui_lbl_log_content, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(ui_lbl_log_content, lv_color_hex(0xDAE2FD), 0);
    lv_label_set_long_mode(ui_lbl_log_content, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(ui_lbl_log_content, 580);
    lv_obj_set_pos(ui_lbl_log_content, 0, 0);

    // Action Buttons
    lv_obj_t * btn_clr = lv_btn_create(card);
    lv_obj_set_size(btn_clr, 170, 36);
    lv_obj_set_pos(btn_clr, 0, 318);
    create_card_style(btn_clr, 0x221319, 0x551E28, 8);
    lv_obj_t * lbl_c = lv_label_create(btn_clr);
    lv_label_set_text(lbl_c, LV_SYMBOL_TRASH " HAPUS LOG");
    lv_obj_set_style_text_font(lbl_c, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_c, lv_color_hex(0xF87171), 0);
    lv_obj_center(lbl_c);
    lv_obj_add_event_cb(btn_clr, log_modal_clear_btn_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_rst = lv_btn_create(card);
    lv_obj_set_size(btn_rst, 220, 36);
    lv_obj_set_pos(btn_rst, 180, 318);
    create_card_style(btn_rst, 0x0D223D, 0x1E4675, 8);
    lv_obj_t * lbl_r = lv_label_create(btn_rst);
    lv_label_set_text(lbl_r, LV_SYMBOL_REFRESH " KOSONGKAN SEMUA");
    lv_obj_set_style_text_font(lbl_r, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_r, lv_color_hex(0x7DD3FC), 0);
    lv_obj_center(lbl_r);
    lv_obj_add_event_cb(btn_rst, log_modal_reset_seats_handler, LV_EVENT_CLICKED, NULL);

    lv_obj_t * btn_cls = lv_btn_create(card);
    lv_obj_set_size(btn_cls, 190, 36);
    lv_obj_set_pos(btn_cls, 418, 318);
    create_card_style(btn_cls, 0x00A572, 0x4EDEA3, 8);
    lv_obj_t * lbl_cl = lv_label_create(btn_cls);
    lv_label_set_text(lbl_cl, LV_SYMBOL_OK " TUTUP");
    lv_obj_set_style_text_font(lbl_cl, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(lbl_cl, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(lbl_cl);
    lv_obj_add_event_cb(btn_cls, log_modal_close_btn_handler, LV_EVENT_CLICKED, NULL);

    // Hidden by default
    lv_obj_add_flag(ui_modal_log, LV_OBJ_FLAG_HIDDEN);
}

// =========================================================================
// MODAL CONTROLLER APIS
// =========================================================================

void ui_modal_wifi_open(void)
{
    if (!ui_modal_wifi && ui_Screen1) {
        create_modal_wifi(ui_Screen1);
    }
    if (ui_modal_wifi) {
        lv_obj_clear_flag(ui_modal_wifi, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(ui_modal_wifi);
    }
}

void ui_modal_wifi_close(void)
{
    if (ui_modal_wifi) {
        lv_obj_add_flag(ui_modal_wifi, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_modal_log_open(void)
{
    if (!ui_modal_log && ui_Screen1) {
        create_modal_log(ui_Screen1);
    }
    if (ui_modal_log) {
        lv_obj_clear_flag(ui_modal_log, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(ui_modal_log);
        if (ui_lbl_log_content) {
            lv_label_set_text(ui_lbl_log_content, g_log_buffer);
        }
        if (g_log_scroll_box) {
            lv_obj_scroll_to_y(g_log_scroll_box, LV_COORD_MAX, LV_ANIM_OFF);
        }
    }
}

void ui_modal_log_close(void)
{
    if (ui_modal_log) {
        lv_obj_add_flag(ui_modal_log, LV_OBJ_FLAG_HIDDEN);
    }
}

void ui_add_log_entry(const char * entry)
{
    if (!entry) return;
    size_t cur_len = strlen(g_log_buffer);
    size_t ent_len = strlen(entry);

    if (cur_len + ent_len >= MAX_LOG_SIZE - 64) {
        char * nl = strchr(g_log_buffer, '\n');
        if (nl) {
            memmove(g_log_buffer, nl + 1, strlen(nl + 1) + 1);
        }
    }
    strncat(g_log_buffer, entry, MAX_LOG_SIZE - strlen(g_log_buffer) - 1);

    if (ui_lbl_log_content) {
        lv_label_set_text(ui_lbl_log_content, g_log_buffer);
        if (g_log_scroll_box) {
            lv_obj_scroll_to_y(g_log_scroll_box, LV_COORD_MAX, LV_ANIM_ON);
        }
    }
}

void ui_clear_logs(void)
{
    g_log_buffer[0] = '\0';
    ui_add_log_entry("[00:00:00] Riwayat log telah dibersihkan.\n");
}

void ui_set_wifi_info(const char * ssid, const char * ip_addr, const char * status_msg)
{
    if (ui_lbl_wifi_ssid && ssid) {
        char buf[64];
        snprintf(buf, sizeof(buf), "SSID: %s", ssid);
        lv_label_set_text(ui_lbl_wifi_ssid, buf);
    }
    if (ui_lbl_wifi_ip && ip_addr) {
        char buf[64];
        snprintf(buf, sizeof(buf), "IP: %s", ip_addr);
        lv_label_set_text(ui_lbl_wifi_ip, buf);
    }
    if (ui_lbl_wifi_status && status_msg) {
        lv_label_set_text(ui_lbl_wifi_status, status_msg);
    }
}

void ui_Screen1_screen_destroy(void)
{
    if (ui_Screen1) {
        lv_obj_del(ui_Screen1);
        ui_Screen1 = NULL;
    }
    ui_lbl_clock = NULL;
    ui_lbl_loc = NULL;
    ui_lbl_date = NULL;
    ui_btn_wifi = NULL;
    ui_btn_log = NULL;
    ui_lbl_volt = NULL;
    ui_lbl_title = NULL;
    ui_lbl_sys_status = NULL;
    ui_led_status = NULL;
    ui_lbl_total_val = NULL;
    ui_lbl_occupied_val = NULL;
    ui_lbl_empty_val = NULL;
    ui_modal_wifi = NULL;
    ui_modal_log = NULL;
    ui_lbl_log_content = NULL;
    ui_lbl_wifi_status = NULL;
    ui_lbl_wifi_ssid = NULL;
    ui_lbl_wifi_ip = NULL;
    g_log_scroll_box = NULL;
    ui_lbl_voltage_val = NULL;
    ui_lbl_temp_val = NULL;
    ui_lbl_humidity_val = NULL;

    for (int i = 0; i < 43; i++) {
        g_bus_seats[i].btn = NULL;
        g_bus_seats[i].headrest = NULL;
        g_bus_seats[i].lbl = NULL;
    }
}

// =========================================================================
// UI CONTROLLER & EXTERNAL API FUNCTIONS
// =========================================================================

void ui_set_clock(const char * time_str)
{
    if (ui_lbl_clock && time_str) {
        lv_label_set_text(ui_lbl_clock, time_str);
    }
}

void ui_set_date(const char * date_str)
{
    if (ui_lbl_date && date_str) {
        lv_label_set_text(ui_lbl_date, date_str);
    }
}

void ui_set_location(const char * loc_str)
{
    if (ui_lbl_loc && loc_str) {
        char buf[48];
        snprintf(buf, sizeof(buf), LV_SYMBOL_GPS " %s", loc_str);
        lv_label_set_text(ui_lbl_loc, buf);
    }
}

void ui_set_voltage(float volts)
{
    if (ui_lbl_volt) {
        char buf[24];
        snprintf(buf, sizeof(buf), LV_SYMBOL_CHARGE " %.1fV", volts);
        lv_label_set_text(ui_lbl_volt, buf);
    }
}

void ui_set_system_status(const char * status_str, bool online)
{
    if (ui_lbl_sys_status && status_str) {
        lv_label_set_text(ui_lbl_sys_status, status_str);
    }
    if (ui_led_status) {
        uint32_t col = online ? 0x00E5FF : 0xEF4444;
        lv_obj_set_style_bg_color(ui_led_status, lv_color_hex(col), 0);
    }
}

void ui_set_seat_occupied(int index, bool occupied)
{
    if (index >= 0 && index < 43) {
        g_bus_seats[index].occupied = occupied;
        ui_apply_seat_style(&g_bus_seats[index]);
        ui_update_counters();

        const char * clock_str = ui_lbl_clock ? lv_label_get_text(ui_lbl_clock) : "13:17";
        char log_entry[96];
        snprintf(log_entry, sizeof(log_entry), "[%s] Kursi %s: %s (API)\n",
                 clock_str, g_bus_seats[index].id, occupied ? "TERISI" : "KOSONG");
        ui_add_log_entry(log_entry);
    }
}

void ui_set_seat_occupied_by_name(const char * seat_name, bool occupied)
{
    if (!seat_name) return;
    for (int i = 0; i < 43; i++) {
        if (strcmp(g_bus_seats[i].id, seat_name) == 0) {
            ui_set_seat_occupied(i, occupied);
            return;
        }
    }
}

bool ui_get_seat_occupied(int index)
{
    if (index >= 0 && index < 43) {
        return g_bus_seats[index].occupied;
    }
    return false;
}

int ui_get_seat_count(void)
{
    return 43;
}

int ui_get_occupied_count(void)
{
    int count = 0;
    for (int i = 0; i < 43; i++) {
        if (g_bus_seats[i].occupied) count++;
    }
    return count;
}

void ui_clear_all_seats(void)
{
    for (int i = 0; i < 43; i++) {
        g_bus_seats[i].occupied = false;
        ui_apply_seat_style(&g_bus_seats[i]);
    }
    ui_update_counters();
}

void ui_occupy_all_seats(void)
{
    for (int i = 0; i < 43; i++) {
        g_bus_seats[i].occupied = true;
        ui_apply_seat_style(&g_bus_seats[i]);
    }
    ui_update_counters();
}
