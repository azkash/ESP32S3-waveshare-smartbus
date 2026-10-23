#include "ui_passenger.h"
#include "config.h"
#include "lvgl_port.h"

// Palet Warna Standar SmartBus
#define COLOR_BG          lv_color_hex(0x03070C)
#define COLOR_PANEL       lv_color_hex(0x0A0F1D)
#define COLOR_BORDER      lv_color_hex(0x1E293B)
#define COLOR_EMERALD     lv_color_hex(0x10B981) // Kosong / Belum Paired
#define COLOR_BLUE        lv_color_hex(0x3B82F6) // Terisi tanpa seatbelt
#define COLOR_RED         lv_color_hex(0xEF4444) // Terisi pakai seatbelt
#define COLOR_AMBER       lv_color_hex(0xF59E0B) // Calling / Chime
#define COLOR_PAIRED      lv_color_hex(0x334155) // Paired (Abu-abu Slate)
#define COLOR_UNPAIRED    lv_color_hex(0x10B981) // Belum Paired (Hijau Emerald)

extern void sendClearCallToMaster(const char* seatId);
extern void sendUnpairSeatToMaster(const char* seatId);
extern void sendAssignSeatIdToMaster(const char* uid, const char* seatId);

// Struktur Data Kursi
struct SeatWidget {
    const char* id;
    int16_t x;
    int16_t y;
    lv_obj_t* btn;
    lv_obj_t* lbl;
    bool occupied;
    bool seatbelt;
    bool calling;
    bool blinkState;
    bool isPaired;
    bool foundInRoster; // Flag rekonsiliasi roster aktif
    lv_obj_t* modalBtn;
};

// 43 Kursi dengan Posisi Pixel Absolut (dalam Bodi Bus 780x355)
static SeatWidget busSeats[] = {
    // Depan
    {"A", 124, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"B", 124, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 1
    {"3B", 182, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"2B", 182, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"1B", 182, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    {"3A", 215, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"2A", 215, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"1A", 215, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 2
    {"6B", 268, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"5B", 268, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"4B", 268, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    {"6A", 301, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"5A", 301, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"4A", 301, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 3
    {"9B", 354, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"8B", 354, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"7B", 354, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    {"9A", 386, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"8A", 386, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"7A", 386, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 4 (10B Kosong)
    {"12B", 439, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"11B", 439, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"12A", 472, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"11A", 472, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"10A", 472, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 5
    {"15B", 525, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"14B", 525, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"13B", 525, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    {"15A", 558, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"14A", 558, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"13A", 558, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 6
    {"18B", 611, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"17B", 611, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"16B", 611, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    {"18A", 644, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"17A", 644, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"16A", 644, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    // Bagian 7
    {"21B", 697, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"20B", 697, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"19B", 697, 265, NULL, NULL, false, false, false, false, false, false, NULL},
    {"21A", 730, 75, NULL, NULL, false, false, false, false, false, false, NULL},
    {"20A", 730, 135, NULL, NULL, false, false, false, false, false, false, NULL},
    {"19A", 730, 265, NULL, NULL, false, false, false, false, false, false, NULL}
};

static const int TOTAL_SEATS = sizeof(busSeats) / sizeof(busSeats[0]);

// Widgets Global
static lv_obj_t* lbl_voltage = NULL;
static lv_obj_t* box_wifi = NULL;
static lv_obj_t* led_wifi = NULL;
static lv_obj_t* lbl_wifi_status = NULL;
static lv_obj_t* lbl_clock = NULL;
static lv_obj_t* lbl_stat_occupied = NULL;
static lv_obj_t* lbl_stat_empty = NULL;
static lv_timer_t* blink_timer = NULL;
static bool voltage_warning = false;
static bool voltage_blink = false;

// Floating Call Notification Banner (Single compact banner)
static lv_obj_t* call_banner = NULL;
static lv_obj_t* lbl_banner_text = NULL;
static lv_obj_t* btn_banner_dismiss = NULL;
static lv_obj_t* lbl_banner_btn = NULL;
static char currentCallingSeatId[16] = "";

// Modal "DAFTAR KURSI"
static lv_obj_t* modal_backdrop = NULL;
static lv_obj_t* modal_card = NULL;
static lv_obj_t* unpair_confirm_box = NULL;
static char unpairTargetSeatId[16] = "";

// Pop-up Pairing Kursi Baru (Saat Modul RandomID Ditekan)
static lv_obj_t* pairing_backdrop = NULL;
static lv_obj_t* pairing_card = NULL;
static lv_obj_t* pairing_confirm_box = NULL;
static char pendingPairingUid[32] = "";
static char pairingTargetSeatId[16] = "";

// Forward declarations internal
static void ui_refresh_call_banner_internal();
static void ui_refresh_statistics_internal();

// ================= REFRESH STATISTIK =================
static void ui_refresh_statistics_internal() {
    int occupiedCount = 0;
    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (busSeats[i].occupied) occupiedCount++;
    }
    int emptyCount = TOTAL_SEATS - occupiedCount;
    char buf[64];
    if (lbl_stat_occupied) {
        snprintf(buf, sizeof(buf), "TERISI (OCCUPIED)\n#EF4444 %d # Pax", occupiedCount);
        lv_label_set_text(lbl_stat_occupied, buf);
    }
    if (lbl_stat_empty) {
        snprintf(buf, sizeof(buf), "KOSONG (AVAILABLE)\n#10B981 %d # Jok", emptyCount);
        lv_label_set_text(lbl_stat_empty, buf);
    }
}

// ================= REFRESH CALL NOTIFICATION BANNER =================
static void ui_refresh_call_banner_internal() {
    if (!call_banner || !lbl_banner_text || !lbl_banner_btn) return;

    const char* activeCallId = NULL;
    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (busSeats[i].calling) {
            activeCallId = busSeats[i].id;
            break;
        }
    }

    if (activeCallId) {
        strlcpy(currentCallingSeatId, activeCallId, sizeof(currentCallingSeatId));
        char msgBuf[64];
        snprintf(msgBuf, sizeof(msgBuf), "PANGGILAN: KURSI %s", activeCallId);
        lv_label_set_text(lbl_banner_text, msgBuf);

        char btnBuf[32];
        snprintf(btnBuf, sizeof(btnBuf), "OKE [%s]", activeCallId);
        lv_label_set_text(lbl_banner_btn, btnBuf);

        lv_obj_clear_flag(call_banner, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(call_banner);
    } else {
        currentCallingSeatId[0] = '\0';
        lv_obj_add_flag(call_banner, LV_OBJ_FLAG_HIDDEN);
    }
}

// Callback Dismiss Floating Banner
static void banner_dismiss_event_cb(lv_event_t* e) {
    (void)e;
    if (currentCallingSeatId[0] != '\0') {
        char targetId[16];
        strlcpy(targetId, currentCallingSeatId, sizeof(targetId));

        sendClearCallToMaster(targetId);

        for (int i = 0; i < TOTAL_SEATS; i++) {
            if (strcasecmp(busSeats[i].id, targetId) == 0) {
                busSeats[i].calling = false;
                if (busSeats[i].btn) {
                    lv_color_t targetColor;
                    if (busSeats[i].occupied) {
                        targetColor = busSeats[i].seatbelt ? COLOR_RED : COLOR_BLUE;
                    } else {
                        targetColor = COLOR_EMERALD;
                    }
                    lv_obj_set_style_bg_color(busSeats[i].btn, targetColor, 0);
                    lv_obj_set_style_border_color(busSeats[i].btn, targetColor, 0);
                    lv_obj_invalidate(busSeats[i].btn);
                }
                break;
            }
        }
        ui_refresh_call_banner_internal();
    }
}

// Callback Saat Kursi di Denah Bus Ditekan
static void seat_click_event_cb(lv_event_t* e) {
    SeatWidget* s = (SeatWidget*)lv_event_get_user_data(e);
    if (s && s->calling) {
        s->calling = false;
        ui_update_seat(s->id, s->occupied, s->seatbelt, false);
        sendClearCallToMaster(s->id);
        ui_refresh_call_banner_internal();
    }
}

// Timer Animasi Kedip (Calling Amber & Low Voltage Warning)
static void blink_timer_cb(lv_timer_t* timer) {
    (void)timer;
    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (busSeats[i].calling && busSeats[i].btn) {
            busSeats[i].blinkState = !busSeats[i].blinkState;
            lv_color_t c = busSeats[i].blinkState ? COLOR_AMBER : lv_color_hex(0x451A03);
            lv_obj_set_style_bg_color(busSeats[i].btn, c, 0);
            lv_obj_set_style_border_color(busSeats[i].btn, COLOR_AMBER, 0);
        }
    }
    if (voltage_warning && lbl_voltage) {
        voltage_blink = !voltage_blink;
        lv_obj_set_style_text_color(lbl_voltage, voltage_blink ? COLOR_RED : COLOR_AMBER, 0);
    }
}

// ================= REKONSILIASI ROSTER LENGKAP =================
// Menjamin kursi yang di-unpair / hilang dari roster Master seketika di-reset menjadi hijau (kosong)
void ui_sync_roster_begin() {
    if (!lvgl_port_lock(-1)) return;
    for (int i = 0; i < TOTAL_SEATS; i++) {
        busSeats[i].foundInRoster = false;
    }
    lvgl_port_unlock();
}

void ui_sync_roster_seat(const char* seatId, bool occupied, bool seatbelt, bool calling) {
    if (!seatId || strlen(seatId) == 0) return;
    if (!lvgl_port_lock(-1)) return;

    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (strcasecmp(busSeats[i].id, seatId) == 0) {
            busSeats[i].foundInRoster = true;
            busSeats[i].isPaired = true;
            busSeats[i].occupied = occupied;
            busSeats[i].seatbelt = seatbelt;
            busSeats[i].calling = calling;

            // Update warna di layar utama
            if (busSeats[i].btn) {
                lv_color_t targetColor;
                if (calling) targetColor = COLOR_AMBER;
                else if (occupied) targetColor = seatbelt ? COLOR_RED : COLOR_BLUE;
                else targetColor = COLOR_EMERALD;

                lv_obj_set_style_bg_color(busSeats[i].btn, targetColor, 0);
                lv_obj_set_style_border_color(busSeats[i].btn, targetColor, 0);
                lv_obj_invalidate(busSeats[i].btn);
            }

            // Update warna di modal daftar kursi jika sedang terbuka
            if (busSeats[i].modalBtn) {
                lv_obj_set_style_bg_color(busSeats[i].modalBtn, COLOR_PAIRED, 0);
                lv_obj_set_style_border_color(busSeats[i].modalBtn, lv_color_hex(0x64748B), 0);
                lv_obj_invalidate(busSeats[i].modalBtn);
            }
            break;
        }
    }
    lvgl_port_unlock();
}

void ui_sync_roster_end() {
    if (!lvgl_port_lock(-1)) return;

    // Setiap kursi yang TIDAK ditemukan di roster Master di-reset menjadi UNPAIRED & KOSONG (Hijau)
    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (!busSeats[i].foundInRoster) {
            busSeats[i].isPaired = false;
            busSeats[i].occupied = false;
            busSeats[i].seatbelt = false;
            busSeats[i].calling = false;

            if (busSeats[i].btn) {
                lv_obj_set_style_bg_color(busSeats[i].btn, COLOR_EMERALD, 0);
                lv_obj_set_style_border_color(busSeats[i].btn, COLOR_EMERALD, 0);
                lv_obj_invalidate(busSeats[i].btn);
            }

            if (busSeats[i].modalBtn) {
                lv_obj_set_style_bg_color(busSeats[i].modalBtn, COLOR_UNPAIRED, 0);
                lv_obj_set_style_border_color(busSeats[i].modalBtn, lv_color_hex(0x059669), 0);
                lv_obj_invalidate(busSeats[i].modalBtn);
            }
        }
    }

    ui_refresh_call_banner_internal();
    ui_refresh_statistics_internal();
    lvgl_port_unlock();
}

// ================= MODAL DAFTAR KURSI & UNPAIR =================
static void close_unpair_confirm_cb(lv_event_t* e) {
    (void)e;
    if (unpair_confirm_box) {
        lv_obj_del(unpair_confirm_box);
        unpair_confirm_box = NULL;
    }
}

static void confirm_unpair_action_cb(lv_event_t* e) {
    (void)e;
    if (unpairTargetSeatId[0] != '\0') {
        Serial.printf("[UI] Menjalankan unpair untuk seat: %s\n", unpairTargetSeatId);
        sendUnpairSeatToMaster(unpairTargetSeatId);

        // Langsung update status & warna lokal menjadi HIJAU seketika di layar utama dan modal
        for (int i = 0; i < TOTAL_SEATS; i++) {
            if (strcasecmp(busSeats[i].id, unpairTargetSeatId) == 0) {
                busSeats[i].isPaired = false;
                busSeats[i].occupied = false;
                busSeats[i].seatbelt = false;
                busSeats[i].calling = false;

                // Update warna di modal -> HIJAU UNPAIRED
                if (busSeats[i].modalBtn) {
                    lv_obj_set_style_bg_color(busSeats[i].modalBtn, COLOR_UNPAIRED, 0);
                    lv_obj_set_style_border_color(busSeats[i].modalBtn, lv_color_hex(0x059669), 0);
                    lv_obj_invalidate(busSeats[i].modalBtn);
                }

                // Update warna di layar utama -> HIJAU EMERALD
                if (busSeats[i].btn) {
                    lv_obj_set_style_bg_color(busSeats[i].btn, COLOR_EMERALD, 0);
                    lv_obj_set_style_border_color(busSeats[i].btn, COLOR_EMERALD, 0);
                    lv_obj_invalidate(busSeats[i].btn);
                }
                break;
            }
        }
        ui_refresh_statistics_internal();
    }

    if (unpair_confirm_box) {
        lv_obj_del(unpair_confirm_box);
        unpair_confirm_box = NULL;
    }
}

// Buka dialog konfirmasi unpair
static void show_unpair_confirm_dialog(const char* seatId) {
    if (!modal_backdrop || !seatId) return;
    strlcpy(unpairTargetSeatId, seatId, sizeof(unpairTargetSeatId));

    if (unpair_confirm_box) {
        lv_obj_del(unpair_confirm_box);
        unpair_confirm_box = NULL;
    }

    unpair_confirm_box = lv_obj_create(modal_backdrop);
    lv_obj_set_size(unpair_confirm_box, 420, 210);
    lv_obj_align(unpair_confirm_box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(unpair_confirm_box, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_color(unpair_confirm_box, lv_color_hex(0xEF4444), 0);
    lv_obj_set_style_border_width(unpair_confirm_box, 2, 0);
    lv_obj_set_style_radius(unpair_confirm_box, 14, 0);
    lv_obj_set_style_pad_all(unpair_confirm_box, 16, 0);
    lv_obj_clear_flag(unpair_confirm_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(unpair_confirm_box);
    lv_label_set_text(title, "KONFIRMASI HAPUS PAIRING");
    lv_obj_set_style_text_color(title, lv_color_hex(0xEF4444), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t* msg = lv_label_create(unpair_confirm_box);
    char buf[128];
    snprintf(buf, sizeof(buf), "Apakah Anda yakin ingin menghapus pairing\nmodul Kursi #EF4444 %s # dari Master ESP32?", seatId);
    lv_label_set_text(msg, buf);
    lv_label_set_recolor(msg, true);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(msg, lv_color_white(), 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t* btn_cancel = lv_btn_create(unpair_confirm_box);
    lv_obj_set_size(btn_cancel, 120, 38);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 20, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x334155), 0);
    lv_obj_set_style_radius(btn_cancel, 8, 0);
    lv_obj_t* lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, "BATAL");
    lv_obj_align(lbl_cancel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_cancel, close_unpair_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_ok = lv_btn_create(unpair_confirm_box);
    lv_obj_set_size(btn_ok, 150, 38);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_RIGHT, -20, 0);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0xDC2626), 0);
    lv_obj_set_style_radius(btn_ok, 8, 0);
    lv_obj_t* lbl_ok = lv_label_create(btn_ok);
    lv_label_set_text(lbl_ok, "HAPUS PAIRING");
    lv_obj_align(lbl_ok, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_ok, confirm_unpair_action_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_move_foreground(unpair_confirm_box);
}

// Buka dialog info kursi belum paired
static void show_unpaired_info_dialog(const char* seatId) {
    if (!modal_backdrop || !seatId) return;

    if (unpair_confirm_box) {
        lv_obj_del(unpair_confirm_box);
        unpair_confirm_box = NULL;
    }

    unpair_confirm_box = lv_obj_create(modal_backdrop);
    lv_obj_set_size(unpair_confirm_box, 400, 190);
    lv_obj_align(unpair_confirm_box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(unpair_confirm_box, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_color(unpair_confirm_box, COLOR_EMERALD, 0);
    lv_obj_set_style_border_width(unpair_confirm_box, 2, 0);
    lv_obj_set_style_radius(unpair_confirm_box, 14, 0);
    lv_obj_set_style_pad_all(unpair_confirm_box, 16, 0);
    lv_obj_clear_flag(unpair_confirm_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(unpair_confirm_box);
    lv_label_set_text(title, "STATUS: BELUM PAIRED");
    lv_obj_set_style_text_color(title, COLOR_EMERALD, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t* msg = lv_label_create(unpair_confirm_box);
    char buf[128];
    snprintf(buf, sizeof(buf), "Kursi #10B981 %s # belum terhubung modul seat.\nNyalakan & tekan tombol modul untuk pairing.", seatId);
    lv_label_set_text(msg, buf);
    lv_label_set_recolor(msg, true);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(msg, lv_color_white(), 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, -5);

    lv_obj_t* btn_close = lv_btn_create(unpair_confirm_box);
    lv_obj_set_size(btn_close, 110, 36);
    lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_radius(btn_close, 8, 0);
    lv_obj_t* lbl_c = lv_label_create(btn_close);
    lv_label_set_text(lbl_c, "TUTUP");
    lv_obj_align(lbl_c, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_close, close_unpair_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_move_foreground(unpair_confirm_box);
}

// Callback klik kursi di dalam modal daftar kursi
static void modal_seat_click_event_cb(lv_event_t* e) {
    SeatWidget* s = (SeatWidget*)lv_event_get_user_data(e);
    if (!s) return;

    if (s->isPaired) {
        show_unpair_confirm_dialog(s->id);
    } else {
        show_unpaired_info_dialog(s->id);
    }
}

// Callback tombol tutup modal
static void close_modal_event_cb(lv_event_t* e) {
    (void)e;
    ui_close_daftar_kursi_modal();
}

// Buka Modal DAFTAR KURSI
void ui_open_daftar_kursi_modal() {
    if (!lvgl_port_lock(-1)) return;

    if (modal_backdrop) {
        lvgl_port_unlock();
        return;
    }

    lv_obj_t* scr = lv_scr_act();

    modal_backdrop = lv_obj_create(scr);
    lv_obj_set_size(modal_backdrop, 800, 480);
    lv_obj_set_pos(modal_backdrop, 0, 0);
    lv_obj_set_style_bg_color(modal_backdrop, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(modal_backdrop, LV_OPA_80, 0);
    lv_obj_set_style_border_width(modal_backdrop, 0, 0);
    lv_obj_set_style_pad_all(modal_backdrop, 0, 0);
    lv_obj_clear_flag(modal_backdrop, LV_OBJ_FLAG_SCROLLABLE);

    modal_card = lv_obj_create(modal_backdrop);
    lv_obj_set_size(modal_card, 760, 430);
    lv_obj_align(modal_card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(modal_card, lv_color_hex(0x0B1220), 0);
    lv_obj_set_style_border_color(modal_card, lv_color_hex(0x334155), 0);
    lv_obj_set_style_border_width(modal_card, 2, 0);
    lv_obj_set_style_radius(modal_card, 16, 0);
    lv_obj_set_style_pad_all(modal_card, 12, 0);
    lv_obj_clear_flag(modal_card, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(modal_card);
    lv_label_set_text(title, "DAFTAR PAIRING KURSI (43 SEATS)");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 5);

    lv_obj_t* legend = lv_label_create(modal_card);
    lv_label_set_text(legend, "#94A3B8 [■] Abu-abu: Paired (Klik utk Unpair)  # | #10B981 [■] Hijau: Belum Paired #");
    lv_label_set_recolor(legend, true);
    lv_obj_align(legend, LV_ALIGN_TOP_LEFT, 290, 6);

    lv_obj_t* btn_close = lv_btn_create(modal_card);
    lv_obj_set_size(btn_close, 36, 32);
    lv_obj_align(btn_close, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0xEF4444), 0);
    lv_obj_set_style_radius(btn_close, 6, 0);
    lv_obj_t* lbl_x = lv_label_create(btn_close);
    lv_label_set_text(lbl_x, "X");
    lv_obj_align(lbl_x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_close, close_modal_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* mini_bus = lv_obj_create(modal_card);
    lv_obj_set_size(mini_bus, 736, 355);
    lv_obj_set_pos(mini_bus, 0, 48);
    lv_obj_set_style_bg_color(mini_bus, lv_color_hex(0x040810), 0);
    lv_obj_set_style_border_color(mini_bus, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(mini_bus, 2, 0);
    lv_obj_set_style_radius(mini_bus, 16, 0);
    lv_obj_clear_flag(mini_bus, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* driver_box = lv_obj_create(mini_bus);
    lv_obj_set_size(driver_box, 45, 335);
    lv_obj_set_pos(driver_box, 10, 0);
    lv_obj_set_style_bg_color(driver_box, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_color(driver_box, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(driver_box, 1, 0);
    lv_obj_t* d_lbl = lv_label_create(driver_box);
    lv_label_set_text(d_lbl, "SUPIR");
    lv_obj_set_style_text_color(d_lbl, lv_color_hex(0x64748B), 0);
    lv_obj_align(d_lbl, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* mini_aisle = lv_obj_create(mini_bus);
    lv_obj_set_size(mini_aisle, 655, 36);
    lv_obj_set_pos(mini_aisle, 65, 195);
    lv_obj_set_style_bg_color(mini_aisle, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_color(mini_aisle, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(mini_aisle, 1, 0);

    for (int i = 0; i < TOTAL_SEATS; i++) {
        lv_obj_t* btn = lv_btn_create(mini_bus);
        lv_obj_set_size(btn, 28, 40);
        int16_t mx = busSeats[i].x - 35;
        int16_t my = busSeats[i].y - 15;
        lv_obj_set_pos(btn, mx, my);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        lv_color_t c = busSeats[i].isPaired ? COLOR_PAIRED : COLOR_UNPAIRED;
        lv_color_t borderC = busSeats[i].isPaired ? lv_color_hex(0x64748B) : lv_color_hex(0x059669);
        lv_obj_set_style_bg_color(btn, c, 0);
        lv_obj_set_style_border_color(btn, borderC, 0);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, busSeats[i].id);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

        lv_obj_add_event_cb(btn, modal_seat_click_event_cb, LV_EVENT_CLICKED, &busSeats[i]);
        busSeats[i].modalBtn = btn;
    }

    lvgl_port_unlock();
}

void ui_close_daftar_kursi_modal() {
    if (!lvgl_port_lock(-1)) return;

    for (int i = 0; i < TOTAL_SEATS; i++) {
        busSeats[i].modalBtn = NULL;
    }

    if (unpair_confirm_box) {
        lv_obj_del(unpair_confirm_box);
        unpair_confirm_box = NULL;
    }

    if (modal_backdrop) {
        lv_obj_del(modal_backdrop);
        modal_backdrop = NULL;
        modal_card = NULL;
    }

    lvgl_port_unlock();
}

// ================= POP-UP PAIRING KURSI BARU =================
static void close_pairing_confirm_cb(lv_event_t* e) {
    (void)e;
    if (pairing_confirm_box) {
        lv_obj_del(pairing_confirm_box);
        pairing_confirm_box = NULL;
    }
}

static void confirm_assign_action_cb(lv_event_t* e) {
    (void)e;
    if (pendingPairingUid[0] != '\0' && pairingTargetSeatId[0] != '\0') {
        Serial.printf("[UI] Mengirim assignSeatId: UID=%s -> Kursi=%s\n", pendingPairingUid, pairingTargetSeatId);
        sendAssignSeatIdToMaster(pendingPairingUid, pairingTargetSeatId);

        // Update lokal agar seketika berubah
        for (int i = 0; i < TOTAL_SEATS; i++) {
            if (strcasecmp(busSeats[i].id, pairingTargetSeatId) == 0) {
                busSeats[i].isPaired = true;
                if (busSeats[i].btn) {
                    lv_obj_set_style_bg_color(busSeats[i].btn, COLOR_EMERALD, 0);
                    lv_obj_set_style_border_color(busSeats[i].btn, COLOR_EMERALD, 0);
                    lv_obj_invalidate(busSeats[i].btn);
                }
                break;
            }
        }
    }

    ui_close_pairing_dialog();
}

static void show_pairing_confirm_dialog(const char* seatId) {
    if (!pairing_backdrop || !seatId) return;
    strlcpy(pairingTargetSeatId, seatId, sizeof(pairingTargetSeatId));

    if (pairing_confirm_box) {
        lv_obj_del(pairing_confirm_box);
        pairing_confirm_box = NULL;
    }

    pairing_confirm_box = lv_obj_create(pairing_backdrop);
    lv_obj_set_size(pairing_confirm_box, 440, 220);
    lv_obj_align(pairing_confirm_box, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(pairing_confirm_box, lv_color_hex(0x061224), 0);
    lv_obj_set_style_border_color(pairing_confirm_box, lv_color_hex(0x0284C7), 0);
    lv_obj_set_style_border_width(pairing_confirm_box, 2, 0);
    lv_obj_set_style_radius(pairing_confirm_box, 14, 0);
    lv_obj_set_style_pad_all(pairing_confirm_box, 16, 0);
    lv_obj_clear_flag(pairing_confirm_box, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* title = lv_label_create(pairing_confirm_box);
    lv_label_set_text(title, "KONFIRMASI PASANGKAN KURSI");
    lv_obj_set_style_text_color(title, lv_color_hex(0x38BDF8), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 5);

    lv_obj_t* msg = lv_label_create(pairing_confirm_box);
    char buf[160];
    snprintf(buf, sizeof(buf), "Pasangkan modul terdeteksi\nUID: #38BDF8 %s #\nke Kursi #10B981 %s #?", pendingPairingUid, seatId);
    lv_label_set_text(msg, buf);
    lv_label_set_recolor(msg, true);
    lv_obj_set_style_text_align(msg, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(msg, lv_color_white(), 0);
    lv_obj_align(msg, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t* btn_cancel = lv_btn_create(pairing_confirm_box);
    lv_obj_set_size(btn_cancel, 120, 38);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_LEFT, 20, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x334155), 0);
    lv_obj_set_style_radius(btn_cancel, 8, 0);
    lv_obj_t* lbl_cancel = lv_label_create(btn_cancel);
    lv_label_set_text(lbl_cancel, "BATAL");
    lv_obj_align(lbl_cancel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_cancel, close_pairing_confirm_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* btn_ok = lv_btn_create(pairing_confirm_box);
    lv_obj_set_size(btn_ok, 170, 38);
    lv_obj_align(btn_ok, LV_ALIGN_BOTTOM_RIGHT, -20, 0);
    lv_obj_set_style_bg_color(btn_ok, lv_color_hex(0x0284C7), 0);
    lv_obj_set_style_radius(btn_ok, 8, 0);
    lv_obj_t* lbl_ok = lv_label_create(btn_ok);
    char btnTxt[32];
    snprintf(btnTxt, sizeof(btnTxt), "PASANGKAN [%s]", seatId);
    lv_label_set_text(lbl_ok, btnTxt);
    lv_obj_align(lbl_ok, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_ok, confirm_assign_action_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_move_foreground(pairing_confirm_box);
}

static void pairing_seat_click_event_cb(lv_event_t* e) {
    SeatWidget* s = (SeatWidget*)lv_event_get_user_data(e);
    if (!s) return;

    if (s->isPaired) {
        // Kursi sudah terpasang, tidak bisa dipilih
        if (pairing_confirm_box) {
            lv_obj_del(pairing_confirm_box);
            pairing_confirm_box = NULL;
        }
        pairing_confirm_box = lv_obj_create(pairing_backdrop);
        lv_obj_set_size(pairing_confirm_box, 380, 160);
        lv_obj_align(pairing_confirm_box, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(pairing_confirm_box, lv_color_hex(0x0F172A), 0);
        lv_obj_set_style_border_color(pairing_confirm_box, lv_color_hex(0xEF4444), 0);
        lv_obj_set_style_border_width(pairing_confirm_box, 2, 0);
        lv_obj_set_style_radius(pairing_confirm_box, 12, 0);

        lv_obj_t* t = lv_label_create(pairing_confirm_box);
        lv_label_set_text(t, "KURSI SUDAH TERPAIRED!");
        lv_obj_set_style_text_color(t, lv_color_hex(0xEF4444), 0);
        lv_obj_align(t, LV_ALIGN_TOP_MID, 0, 10);

        lv_obj_t* m = lv_label_create(pairing_confirm_box);
        char b[128];
        snprintf(b, sizeof(b), "Kursi %s sudah memiliki modul.\nSilakan pilih kursi yang masih HIJAU.", s->id);
        lv_label_set_text(m, b);
        lv_obj_set_style_text_align(m, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(m, lv_color_white(), 0);
        lv_obj_align(m, LV_ALIGN_CENTER, 0, 0);

        lv_obj_t* bc = lv_btn_create(pairing_confirm_box);
        lv_obj_set_size(bc, 100, 32);
        lv_obj_align(bc, LV_ALIGN_BOTTOM_MID, 0, -5);
        lv_obj_set_style_bg_color(bc, lv_color_hex(0x334155), 0);
        lv_obj_t* lc = lv_label_create(bc);
        lv_label_set_text(lc, "OK");
        lv_obj_align(lc, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_event_cb(bc, close_pairing_confirm_cb, LV_EVENT_CLICKED, NULL);
    } else {
        show_pairing_confirm_dialog(s->id);
    }
}

static void close_pairing_dialog_btn_cb(lv_event_t* e) {
    (void)e;
    ui_close_pairing_dialog();
}

// Menampilkan Dialog Pop-up Pairing Kursi Baru
void ui_show_pairing_dialog(const char* uid) {
    if (!uid || strlen(uid) == 0) return;
    if (!lvgl_port_lock(-1)) return;

    strlcpy(pendingPairingUid, uid, sizeof(pendingPairingUid));

    // Jika pop-up sudah terbuka, jangan buat ulang
    if (pairing_backdrop) {
        lvgl_port_unlock();
        return;
    }

    lv_obj_t* scr = lv_scr_act();

    pairing_backdrop = lv_obj_create(scr);
    lv_obj_set_size(pairing_backdrop, 800, 480);
    lv_obj_set_pos(pairing_backdrop, 0, 0);
    lv_obj_set_style_bg_color(pairing_backdrop, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(pairing_backdrop, LV_OPA_80, 0);
    lv_obj_set_style_border_width(pairing_backdrop, 0, 0);
    lv_obj_set_style_pad_all(pairing_backdrop, 0, 0);
    lv_obj_clear_flag(pairing_backdrop, LV_OBJ_FLAG_SCROLLABLE);

    pairing_card = lv_obj_create(pairing_backdrop);
    lv_obj_set_size(pairing_card, 760, 430);
    lv_obj_align(pairing_card, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(pairing_card, lv_color_hex(0x071120), 0);
    lv_obj_set_style_border_color(pairing_card, lv_color_hex(0x0284C7), 0);
    lv_obj_set_style_border_width(pairing_card, 2, 0);
    lv_obj_set_style_radius(pairing_card, 16, 0);
    lv_obj_set_style_pad_all(pairing_card, 12, 0);
    lv_obj_clear_flag(pairing_card, LV_OBJ_FLAG_SCROLLABLE);

    // Judul & Badge
    lv_obj_t* title = lv_label_create(pairing_card);
    char tbuf[128];
    snprintf(tbuf, sizeof(tbuf), "PAIRING MODUL KURSI BARU | UID: #38BDF8 %s #", uid);
    lv_label_set_text(title, tbuf);
    lv_label_set_recolor(title, true);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 5);

    lv_obj_t* subtitle = lv_label_create(pairing_card);
    lv_label_set_text(subtitle, "Sentuh salah satu kursi #10B981 HIJAU (KOSONG) # untuk memasangkan:");
    lv_label_set_recolor(subtitle, true);
    lv_obj_set_style_text_color(subtitle, lv_color_hex(0x94A3B8), 0);
    lv_obj_align(subtitle, LV_ALIGN_TOP_LEFT, 10, 26);

    // Tombol Tutup [X]
    lv_obj_t* btn_close = lv_btn_create(pairing_card);
    lv_obj_set_size(btn_close, 36, 32);
    lv_obj_align(btn_close, LV_ALIGN_TOP_RIGHT, 0, 0);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0xEF4444), 0);
    lv_obj_set_style_radius(btn_close, 6, 0);
    lv_obj_t* lbl_x = lv_label_create(btn_close);
    lv_label_set_text(lbl_x, "X");
    lv_obj_align(lbl_x, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_close, close_pairing_dialog_btn_cb, LV_EVENT_CLICKED, NULL);

    // Bodi Bus Kabin di dalam Dialog Pairing
    lv_obj_t* mini_bus = lv_obj_create(pairing_card);
    lv_obj_set_size(mini_bus, 736, 345);
    lv_obj_set_pos(mini_bus, 0, 55);
    lv_obj_set_style_bg_color(mini_bus, lv_color_hex(0x040810), 0);
    lv_obj_set_style_border_color(mini_bus, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(mini_bus, 2, 0);
    lv_obj_set_style_radius(mini_bus, 16, 0);
    lv_obj_clear_flag(mini_bus, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* driver_box = lv_obj_create(mini_bus);
    lv_obj_set_size(driver_box, 45, 325);
    lv_obj_set_pos(driver_box, 10, 0);
    lv_obj_set_style_bg_color(driver_box, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_color(driver_box, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(driver_box, 1, 0);
    lv_obj_t* d_lbl = lv_label_create(driver_box);
    lv_label_set_text(d_lbl, "SUPIR");
    lv_obj_set_style_text_color(d_lbl, lv_color_hex(0x64748B), 0);
    lv_obj_align(d_lbl, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t* mini_aisle = lv_obj_create(mini_bus);
    lv_obj_set_size(mini_aisle, 655, 36);
    lv_obj_set_pos(mini_aisle, 65, 195);
    lv_obj_set_style_bg_color(mini_aisle, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_color(mini_aisle, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(mini_aisle, 1, 0);

    for (int i = 0; i < TOTAL_SEATS; i++) {
        lv_obj_t* btn = lv_btn_create(mini_bus);
        lv_obj_set_size(btn, 28, 40);
        int16_t mx = busSeats[i].x - 35;
        int16_t my = busSeats[i].y - 15;
        lv_obj_set_pos(btn, mx, my);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);

        if (busSeats[i].isPaired) {
            // Sudah paired: Abu-abu redup
            lv_obj_set_style_bg_color(btn, COLOR_PAIRED, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x475569), 0);
            lv_obj_set_style_border_width(btn, 1, 0);
        } else {
            // Belum paired: Hijau Terang bersinar siap dipilih
            lv_obj_set_style_bg_color(btn, COLOR_UNPAIRED, 0);
            lv_obj_set_style_border_color(btn, lv_color_hex(0x34D399), 0);
            lv_obj_set_style_border_width(btn, 2, 0);
        }

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, busSeats[i].id);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

        lv_obj_add_event_cb(btn, pairing_seat_click_event_cb, LV_EVENT_CLICKED, &busSeats[i]);
    }

    lv_obj_move_foreground(pairing_backdrop);
    lvgl_port_unlock();
}

void ui_close_pairing_dialog() {
    if (!lvgl_port_lock(-1)) return;

    if (pairing_confirm_box) {
        lv_obj_del(pairing_confirm_box);
        pairing_confirm_box = NULL;
    }

    if (pairing_backdrop) {
        lv_obj_del(pairing_backdrop);
        pairing_backdrop = NULL;
        pairing_card = NULL;
    }

    pendingPairingUid[0] = '\0';
    pairingTargetSeatId[0] = '\0';

    lvgl_port_unlock();
}

void ui_on_pairing_result(bool success, const char* seatId, const char* errorMsg) {
    (void)errorMsg;
    if (!lvgl_port_lock(-1)) return;

    if (success && seatId) {
        for (int i = 0; i < TOTAL_SEATS; i++) {
            if (strcasecmp(busSeats[i].id, seatId) == 0) {
                busSeats[i].isPaired = true;
                if (busSeats[i].btn) {
                    lv_obj_set_style_bg_color(busSeats[i].btn, COLOR_EMERALD, 0);
                    lv_obj_set_style_border_color(busSeats[i].btn, COLOR_EMERALD, 0);
                    lv_obj_invalidate(busSeats[i].btn);
                }
                break;
            }
        }
    }

    lvgl_port_unlock();
    ui_close_pairing_dialog();
}

// Callback Tombol Nav Atas "DAFTAR KURSI"
static void open_daftar_kursi_btn_cb(lv_event_t* e) {
    (void)e;
    ui_open_daftar_kursi_modal();
}

// ================= INISIALISASI UI UTAMA =================
void ui_init() {
    lv_obj_t* scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, COLOR_BG, 0);

    // ================= 1. HEADER BAR (45px) =================
    lv_obj_t* header = lv_obj_create(scr);
    lv_obj_set_size(header, 800, 45);
    lv_obj_set_pos(header, 0, 0);
    lv_obj_set_style_bg_color(header, COLOR_PANEL, 0);
    lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_border_color(header, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(header, 1, 0);
    lv_obj_set_style_pad_all(header, 8, 0);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Judul
    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, "SMARTBUS MONITOR");
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);

    // Tombol "DAFTAR KURSI" di Nav Atas
    lv_obj_t* btn_daftar = lv_btn_create(header);
    lv_obj_set_size(btn_daftar, 130, 30);
    lv_obj_align(btn_daftar, LV_ALIGN_LEFT_MID, 190, 0);
    lv_obj_set_style_bg_color(btn_daftar, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_color(btn_daftar, lv_color_hex(0x3B82F6), 0);
    lv_obj_set_style_border_width(btn_daftar, 1, 0);
    lv_obj_set_style_radius(btn_daftar, 6, 0);
    lv_obj_t* lbl_d = lv_label_create(btn_daftar);
    lv_label_set_text(lbl_d, "DAFTAR KURSI");
    lv_obj_set_style_text_color(lbl_d, lv_color_white(), 0);
    lv_obj_align(lbl_d, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_daftar, open_daftar_kursi_btn_cb, LV_EVENT_CLICKED, NULL);

    // Jam (Tengah Atas)
    lbl_clock = lv_label_create(header);
    lv_label_set_text(lbl_clock, "21:05");
    lv_obj_set_style_text_color(lbl_clock, lv_color_white(), 0);
    lv_obj_align(lbl_clock, LV_ALIGN_CENTER, 20, 0);

    // Indikator Voltase Aki Bus
    lbl_voltage = lv_label_create(header);
    lv_label_set_text(lbl_voltage, "24.0V");
    lv_obj_set_style_text_color(lbl_voltage, COLOR_AMBER, 0);
    lv_obj_align(lbl_voltage, LV_ALIGN_RIGHT_MID, -165, 0);

    // Box Indikator WiFi (Pojok Kanan Atas)
    box_wifi = lv_obj_create(header);
    lv_obj_set_size(box_wifi, LV_SIZE_CONTENT, 30);
    lv_obj_align(box_wifi, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_flex_flow(box_wifi, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(box_wifi, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_hor(box_wifi, 10, 0);
    lv_obj_set_style_pad_ver(box_wifi, 2, 0);
    lv_obj_set_style_pad_column(box_wifi, 6, 0);
    lv_obj_set_style_radius(box_wifi, 6, 0);
    lv_obj_set_style_border_width(box_wifi, 1, 0);
    lv_obj_clear_flag(box_wifi, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_set_style_border_color(box_wifi, COLOR_RED, 0);
    lv_obj_set_style_bg_color(box_wifi, lv_color_hex(0x450A0A), 0);
    lv_obj_set_style_bg_opa(box_wifi, LV_OPA_80, 0);

    led_wifi = lv_obj_create(box_wifi);
    lv_obj_set_size(led_wifi, 8, 8);
    lv_obj_set_style_radius(led_wifi, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(led_wifi, COLOR_RED, 0);
    lv_obj_set_style_border_width(led_wifi, 0, 0);
    lv_obj_set_style_pad_all(led_wifi, 0, 0);

    lbl_wifi_status = lv_label_create(box_wifi);
    lv_label_set_text(lbl_wifi_status, "DISCONNECTED");
    lv_obj_set_style_text_color(lbl_wifi_status, COLOR_RED, 0);

    // ================= 2. BODI BUS & DENAH KURSI (355px) =================
    lv_obj_t* bus_body = lv_obj_create(scr);
    lv_obj_set_size(bus_body, 780, 355);
    lv_obj_set_pos(bus_body, 10, 48);
    lv_obj_set_style_bg_color(bus_body, lv_color_hex(0x040810), 0);
    lv_obj_set_style_border_color(bus_body, lv_color_hex(0x334155), 0);
    lv_obj_set_style_border_width(bus_body, 3, 0);
    lv_obj_set_style_radius(bus_body, 25, 0);
    lv_obj_clear_flag(bus_body, LV_OBJ_FLAG_SCROLLABLE);

    // Sekat Kaca Depan (Windshield)
    lv_obj_t* windshield = lv_obj_create(bus_body);
    lv_obj_set_size(windshield, 15, 335);
    lv_obj_set_pos(windshield, 15, 0);
    lv_obj_set_style_bg_color(windshield, lv_color_hex(0x0891B2), 0);
    lv_obj_set_style_bg_opa(windshield, LV_OPA_30, 0);
    lv_obj_set_style_border_width(windshield, 0, 0);

    // Sekat Kabin Supir
    lv_obj_t* driver_cabin = lv_obj_create(bus_body);
    lv_obj_set_size(driver_cabin, 55, 335);
    lv_obj_set_pos(driver_cabin, 35, 0);
    lv_obj_set_style_bg_color(driver_cabin, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_border_color(driver_cabin, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(driver_cabin, 2, 0);
    lv_obj_t* driver_lbl = lv_label_create(driver_cabin);
    lv_label_set_text(driver_lbl, "SUPIR");
    lv_obj_set_style_text_color(driver_lbl, lv_color_hex(0x64748B), 0);
    lv_obj_align(driver_lbl, LV_ALIGN_CENTER, 0, 0);

    // Lorong Jalan Bus (Garis Tengah)
    lv_obj_t* aisle = lv_obj_create(bus_body);
    lv_obj_set_size(aisle, 670, 40);
    lv_obj_set_pos(aisle, 95, 195);
    lv_obj_set_style_bg_color(aisle, lv_color_hex(0x020617), 0);
    lv_obj_set_style_border_color(aisle, lv_color_hex(0x1E293B), 0);
    lv_obj_set_style_border_width(aisle, 1, 0);

    // Garis Pembatas Putih Per Kompartemen (7 Pembatas)
    const int16_t partitionX[] = {153, 241, 327, 413, 499, 585, 670};
    for (int p = 0; p < 7; p++) {
        lv_obj_t* line = lv_obj_create(bus_body);
        lv_obj_set_size(line, 2, 330);
        lv_obj_set_pos(line, partitionX[p], 5);
        lv_obj_set_style_bg_color(line, lv_color_hex(0xE2E8F0), 0);
        lv_obj_set_style_bg_opa(line, LV_OPA_60, 0);
        lv_obj_set_style_border_width(line, 0, 0);
    }

    // Render 43 Kursi Bus
    for (int i = 0; i < TOTAL_SEATS; i++) {
        lv_obj_t* btn = lv_btn_create(bus_body);
        lv_obj_set_size(btn, 28, 42);
        lv_obj_set_pos(btn, busSeats[i].x, busSeats[i].y);
        lv_obj_set_style_radius(btn, 6, 0);
        lv_obj_set_style_pad_all(btn, 0, 0);
        lv_obj_set_style_border_width(btn, 1, 0);

        // Awal default: Hijau Emerald (Kosong)
        lv_obj_set_style_bg_color(btn, COLOR_EMERALD, 0);
        lv_obj_set_style_border_color(btn, COLOR_EMERALD, 0);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, busSeats[i].id);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 0);

        lv_obj_add_event_cb(btn, seat_click_event_cb, LV_EVENT_CLICKED, &busSeats[i]);
        busSeats[i].btn = btn;
        busSeats[i].lbl = lbl;
    }

    // ================= 3. FLOATING CALL BANNER =================
    call_banner = lv_obj_create(scr);
    lv_obj_set_size(call_banner, 340, 40);
    lv_obj_set_pos(call_banner, 230, 52);
    lv_obj_set_style_bg_color(call_banner, lv_color_hex(0xD97706), 0);
    lv_obj_set_style_border_color(call_banner, lv_color_hex(0xFDE68A), 0);
    lv_obj_set_style_border_width(call_banner, 2, 0);
    lv_obj_set_style_radius(call_banner, 20, 0);
    lv_obj_set_style_pad_hor(call_banner, 12, 0);
    lv_obj_set_style_pad_ver(call_banner, 4, 0);
    lv_obj_set_flex_flow(call_banner, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(call_banner, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(call_banner, LV_OBJ_FLAG_SCROLLABLE);

    lbl_banner_text = lv_label_create(call_banner);
    lv_label_set_text(lbl_banner_text, "PANGGILAN: KURSI --");
    lv_obj_set_style_text_color(lbl_banner_text, lv_color_white(), 0);

    btn_banner_dismiss = lv_btn_create(call_banner);
    lv_obj_set_size(btn_banner_dismiss, 95, 28);
    lv_obj_set_style_bg_color(btn_banner_dismiss, lv_color_hex(0x78350F), 0);
    lv_obj_set_style_border_color(btn_banner_dismiss, lv_color_hex(0xFDE68A), 0);
    lv_obj_set_style_border_width(btn_banner_dismiss, 1, 0);
    lv_obj_set_style_radius(btn_banner_dismiss, 14, 0);
    lbl_banner_btn = lv_label_create(btn_banner_dismiss);
    lv_label_set_text(lbl_banner_btn, "OKE");
    lv_obj_set_style_text_color(lbl_banner_btn, lv_color_white(), 0);
    lv_obj_align(lbl_banner_btn, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(btn_banner_dismiss, banner_dismiss_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_flag(call_banner, LV_OBJ_FLAG_HIDDEN);

    // ================= 4. FOOTER STATISTIK (65px) =================
    lv_obj_t* footer = lv_obj_create(scr);
    lv_obj_set_size(footer, 780, 65);
    lv_obj_set_pos(footer, 10, 410);
    lv_obj_set_style_bg_color(footer, COLOR_PANEL, 0);
    lv_obj_set_style_border_color(footer, COLOR_BORDER, 0);
    lv_obj_set_style_border_width(footer, 1, 0);
    lv_obj_set_style_radius(footer, 12, 0);
    lv_obj_clear_flag(footer, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* l1 = lv_label_create(footer);
    lv_label_set_text(l1, "KAPASITAS ARMADA\n#FFFFFF 43 # Jok");
    lv_label_set_recolor(l1, true);
    lv_obj_align(l1, LV_ALIGN_LEFT_MID, 40, 0);

    lbl_stat_occupied = lv_label_create(footer);
    lv_label_set_text(lbl_stat_occupied, "TERISI (OCCUPIED)\n#EF4444 0 # Pax");
    lv_label_set_recolor(lbl_stat_occupied, true);
    lv_obj_align(lbl_stat_occupied, LV_ALIGN_CENTER, 0, 0);

    lbl_stat_empty = lv_label_create(footer);
    lv_label_set_text(lbl_stat_empty, "KOSONG (AVAILABLE)\n#10B981 43 # Jok");
    lv_label_set_recolor(lbl_stat_empty, true);
    lv_obj_align(lbl_stat_empty, LV_ALIGN_RIGHT_MID, -40, 0);

    blink_timer = lv_timer_create(blink_timer_cb, 500, NULL);
}

// ================= UPDATE STATE KURSI DARI WEBSOCKET =================
void ui_update_seat(const char* seatId, bool occupied, bool seatbelt, bool calling) {
    if (!seatId || strlen(seatId) == 0) return;
    if (!lvgl_port_lock(-1)) return;

    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (strcasecmp(busSeats[i].id, seatId) == 0) {
            busSeats[i].occupied = occupied;
            busSeats[i].seatbelt = seatbelt;
            busSeats[i].calling = calling;
            busSeats[i].isPaired = true;

            if (busSeats[i].btn) {
                lv_color_t targetColor;
                if (calling) {
                    targetColor = COLOR_AMBER;
                } else if (occupied) {
                    targetColor = seatbelt ? COLOR_RED : COLOR_BLUE;
                } else {
                    targetColor = COLOR_EMERALD;
                }
                lv_obj_set_style_bg_color(busSeats[i].btn, targetColor, 0);
                lv_obj_set_style_border_color(busSeats[i].btn, targetColor, 0);
                lv_obj_invalidate(busSeats[i].btn);
            }

            if (busSeats[i].modalBtn) {
                lv_obj_set_style_bg_color(busSeats[i].modalBtn, COLOR_PAIRED, 0);
                lv_obj_set_style_border_color(busSeats[i].modalBtn, lv_color_hex(0x64748B), 0);
                lv_obj_invalidate(busSeats[i].modalBtn);
            }
            break;
        }
    }
    ui_refresh_call_banner_internal();
    ui_refresh_statistics_internal();
    lvgl_port_unlock();
}

void ui_set_seat_paired(const char* seatId, bool isPaired) {
    if (!seatId || strlen(seatId) == 0) return;
    if (!lvgl_port_lock(-1)) return;

    for (int i = 0; i < TOTAL_SEATS; i++) {
        if (strcasecmp(busSeats[i].id, seatId) == 0) {
            busSeats[i].isPaired = isPaired;
            if (busSeats[i].modalBtn) {
                lv_color_t c = isPaired ? COLOR_PAIRED : COLOR_UNPAIRED;
                lv_color_t borderC = isPaired ? lv_color_hex(0x64748B) : lv_color_hex(0x059669);
                lv_obj_set_style_bg_color(busSeats[i].modalBtn, c, 0);
                lv_obj_set_style_border_color(busSeats[i].modalBtn, borderC, 0);
                lv_obj_invalidate(busSeats[i].modalBtn);
            }
            if (busSeats[i].btn && !isPaired) {
                lv_obj_set_style_bg_color(busSeats[i].btn, COLOR_EMERALD, 0);
                lv_obj_set_style_border_color(busSeats[i].btn, COLOR_EMERALD, 0);
                lv_obj_invalidate(busSeats[i].btn);
            }
            break;
        }
    }
    lvgl_port_unlock();
}

void ui_refresh_statistics() {
    if (!lvgl_port_lock(-1)) return;
    ui_refresh_statistics_internal();
    lvgl_port_unlock();
}

void ui_update_voltage(float voltage) {
    if (!lvgl_port_lock(-1)) return;

    if (lbl_voltage) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%.1fV", voltage);
        lv_label_set_text(lbl_voltage, buf);
        voltage_warning = (voltage > 0 && voltage <= 22.5f);
        if (!voltage_warning) {
            lv_obj_set_style_text_color(lbl_voltage, COLOR_AMBER, 0);
        }
    }
    lvgl_port_unlock();
}

void ui_update_network_status(bool isConnected, const char* ipStr) {
    (void)ipStr;
    if (!lvgl_port_lock(-1)) return;

    if (box_wifi && lbl_wifi_status && led_wifi) {
        if (isConnected) {
            lv_obj_set_style_border_color(box_wifi, COLOR_EMERALD, 0);
            lv_obj_set_style_bg_color(box_wifi, lv_color_hex(0x064E3B), 0);
            lv_obj_set_style_bg_opa(box_wifi, LV_OPA_80, 0);
            lv_obj_set_style_bg_color(led_wifi, COLOR_EMERALD, 0);

            lv_label_set_text(lbl_wifi_status, "CONNECTED");
            lv_obj_set_style_text_color(lbl_wifi_status, COLOR_EMERALD, 0);
        } else {
            lv_obj_set_style_border_color(box_wifi, COLOR_RED, 0);
            lv_obj_set_style_bg_color(box_wifi, lv_color_hex(0x450A0A), 0);
            lv_obj_set_style_bg_opa(box_wifi, LV_OPA_80, 0);
            lv_obj_set_style_bg_color(led_wifi, COLOR_RED, 0);

            lv_label_set_text(lbl_wifi_status, "DISCONNECTED");
            lv_obj_set_style_text_color(lbl_wifi_status, COLOR_RED, 0);
        }
    }
    lvgl_port_unlock();
}

void ui_update_clock(const char* timeStr) {
    if (!lvgl_port_lock(-1)) return;

    if (lbl_clock && timeStr) {
        lv_label_set_text(lbl_clock, timeStr);
    }
    lvgl_port_unlock();
}
