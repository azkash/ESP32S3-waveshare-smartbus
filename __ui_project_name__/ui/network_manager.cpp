#include "network_manager.h"
#include "config.h"
#include "ui_passenger.h"
#include <WiFi.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>

using namespace websockets;

static WebsocketsClient wsClient;
static WiFiUDP udpClient;
static bool wsConnected = false;
static bool wifiConnected = false;
static bool mdnsStarted = false;
static unsigned long lastWsRetry = 0;
static unsigned long lastWifiRetry = 0;
static unsigned long lastWsMessageTime = 0;
static String masterIp = "";
static String cachedMasterIp = "";

// ================= FUNGSI BANTU PENORMALAN ID =================
static String normalizeSeatId(const char* rawId) {
    if (!rawId) return "";
    String s = rawId;
    s.trim();
    s.toUpperCase();
    if (s.startsWith("SEAT-")) s = s.substring(5);
    else if (s.startsWith("SEAT_")) s = s.substring(5);
    else if (s.startsWith("SEAT"))  s = s.substring(4);
    return s;
}

// ================= KIRIM PERINTAH CLEAR CALL KE MASTER =================
void sendClearCallToMaster(const char* seatId) {
    if (!seatId || strlen(seatId) == 0) return;
    String cleanId = normalizeSeatId(seatId);

    if (wsConnected) {
        StaticJsonDocument<128> doc;
        doc["action"] = "clearCall";
        doc["id"] = cleanId;
        String msg;
        serializeJson(doc, msg);
        wsClient.send(msg);

        String legacyMsg = "clearCall_" + cleanId;
        wsClient.send(legacyMsg);

        Serial.printf("[WS-TX] Clear call terkirim untuk seat: %s\n", cleanId.c_str());
    } else {
        Serial.printf("[WS-TX WARN] Gagal kirim clear call, WS belum terhubung: %s\n", cleanId.c_str());
    }
}

// ================= KIRIM PERINTAH UNPAIR SEAT KE MASTER =================
void sendUnpairSeatToMaster(const char* seatId) {
    if (!seatId || strlen(seatId) == 0) return;
    String cleanId = normalizeSeatId(seatId);

    if (wsConnected) {
        StaticJsonDocument<128> doc;
        doc["action"] = "unpairSeat";
        doc["id"] = cleanId;
        String msg;
        serializeJson(doc, msg);
        wsClient.send(msg);

        StaticJsonDocument<128> doc2;
        doc2["action"] = "clearSeatId";
        doc2["id"] = cleanId;
        String msg2;
        serializeJson(doc2, msg2);
        wsClient.send(msg2);

        Serial.printf("[WS-TX] Unpair terkirim untuk seat: %s\n", cleanId.c_str());
    } else {
        Serial.printf("[WS-TX WARN] Gagal kirim unpair, WS belum terhubung: %s\n", cleanId.c_str());
    }
}

// ================= KIRIM PERINTAH ASSIGN SEAT ID KE MASTER =================
void sendAssignSeatIdToMaster(const char* uid, const char* seatId) {
    if (!uid || !seatId) return;
    String cleanId = normalizeSeatId(seatId);

    if (wsConnected) {
        StaticJsonDocument<160> doc;
        doc["action"] = "assignSeatId";
        doc["uid"] = uid;
        doc["id"] = cleanId;
        String msg;
        serializeJson(doc, msg);
        wsClient.send(msg);
        Serial.printf("[WS-TX] assignSeatId terkirim: UID=%s -> Kursi=%s\n", uid, cleanId.c_str());
    } else {
        Serial.printf("[WS-TX WARN] Gagal kirim assignSeatId, WS belum terhubung!\n");
    }
}

// ================= TES KONEKSI CEPAT KE PORT (TCP PROBE) =================
static bool probePort(IPAddress ip, uint16_t port, uint32_t timeoutMs = 150) {
    WiFiClient probe;
    bool ok = probe.connect(ip, port, timeoutMs);
    if (ok) {
        probe.stop();
    }
    return ok;
}

// ================= TIER 1: UDP BROADCAST AUTO-DISCOVERY =================
static IPAddress discoverMasterViaUdp(uint32_t timeoutMs = 600) {
    if (WiFi.status() != WL_CONNECTED) return IPAddress(0, 0, 0, 0);

    udpClient.begin(0);

    IPAddress ip = WiFi.localIP();
    IPAddress mask = WiFi.subnetMask();
    IPAddress bcast(ip[0] | (~mask[0]), ip[1] | (~mask[1]), ip[2] | (~mask[2]), ip[3] | (~mask[3]));

    const char* req = "{\"type\":\"discover\",\"role\":\"headunit\"}";

    udpClient.beginPacket(bcast, MASTER_UDP_PORT);
    udpClient.write((const uint8_t*)req, strlen(req));
    udpClient.endPacket();

    udpClient.beginPacket(IPAddress(255, 255, 255, 255), MASTER_UDP_PORT);
    udpClient.write((const uint8_t*)req, strlen(req));
    udpClient.endPacket();

    Serial.printf("[UDP-DISCOVERY] Mengirim broadcast discover ke port %u...\n", MASTER_UDP_PORT);

    unsigned long start = millis();
    while (millis() - start < timeoutMs) {
        int packetSize = udpClient.parsePacket();
        if (packetSize > 0) {
            char buf[256];
            int len = udpClient.read((uint8_t*)buf, sizeof(buf) - 1);
            if (len > 0) {
                buf[len] = '\0';
                StaticJsonDocument<256> doc;
                if (!deserializeJson(doc, buf)) {
                    const char* type = doc["type"] | "";
                    if (strcmp(type, "master") == 0) {
                        IPAddress masterAddr = udpClient.remoteIP();
                        udpClient.stop();
                        Serial.printf("[UDP-DISCOVERY] Master berhasil ditemukan di %s:%u!\n",
                                      masterAddr.toString().c_str(), MASTER_UDP_PORT);
                        return masterAddr;
                    }
                }
            }
        }
        delay(10);
    }
    udpClient.stop();
    return IPAddress(0, 0, 0, 0);
}

// ================= TIER 2: mDNS QUERY DISCOVERY =================
static IPAddress discoverMasterViaMdns(uint32_t timeoutMs = 1200) {
    if (WiFi.status() != WL_CONNECTED) return IPAddress(0, 0, 0, 0);

    Serial.printf("[mDNS] Mencari responder '%s'...\n", MASTER_MDNS_HOST);
    IPAddress resolvedIp = MDNS.queryHost(MASTER_MDNS_HOST, timeoutMs);
    if (resolvedIp != IPAddress(0, 0, 0, 0)) {
        Serial.printf("[mDNS] Host '%s' ditemukan di %s!\n", MASTER_MDNS_HOST, resolvedIp.toString().c_str());
        return resolvedIp;
    }

    resolvedIp = MDNS.queryHost("bus-led.local", 500);
    if (resolvedIp != IPAddress(0, 0, 0, 0)) {
        Serial.printf("[mDNS] Host 'bus-led.local' ditemukan di %s!\n", resolvedIp.toString().c_str());
        return resolvedIp;
    }

    return IPAddress(0, 0, 0, 0);
}

// ================= TIER 3: SUBNET SCAN DISCOVERY (PORT 81) =================
static IPAddress discoverMasterViaSubnetScan(uint32_t perHostTimeoutMs = 100) {
    if (WiFi.status() != WL_CONNECTED) return IPAddress(0, 0, 0, 0);

    IPAddress localIp = WiFi.localIP();
    Serial.printf("[SCAN] Memulai pemindaian subnet %d.%d.%d.1-254 pada port %u...\n",
                  localIp[0], localIp[1], localIp[2], MASTER_WS_PORT);

    for (int i = 1; i <= 254; i++) {
        if (i == localIp[3]) continue;
        IPAddress target(localIp[0], localIp[1], localIp[2], i);
        if (probePort(target, MASTER_WS_PORT, perHostTimeoutMs)) {
            Serial.printf("[SCAN] Master ditemukan di %s:%u!\n", target.toString().c_str(), MASTER_WS_PORT);
            return target;
        }
    }
    return IPAddress(0, 0, 0, 0);
}

// ================= STRATEGI PENEMUAN MASTER LENGKAP =================
static bool resolveMasterIp() {
    if (cachedMasterIp.length() > 0) {
        IPAddress cached;
        if (cached.fromString(cachedMasterIp)) {
            if (probePort(cached, MASTER_WS_PORT, 200)) {
                masterIp = cachedMasterIp;
                Serial.printf("[CACHE] Master aktif di IP tersimpan: %s\n", masterIp.c_str());
                return true;
            } else {
                Serial.printf("[CACHE] IP %s tidak merespons, auto-discovery dimulai...\n", cachedMasterIp.c_str());
                cachedMasterIp = "";
            }
        }
    }

    IPAddress found = discoverMasterViaUdp(600);
    if (found != IPAddress(0, 0, 0, 0)) {
        masterIp = found.toString();
        cachedMasterIp = masterIp;
        return true;
    }

    found = discoverMasterViaMdns(1000);
    if (found != IPAddress(0, 0, 0, 0)) {
        masterIp = found.toString();
        cachedMasterIp = masterIp;
        return true;
    }

    found = discoverMasterViaSubnetScan(100);
    if (found != IPAddress(0, 0, 0, 0)) {
        masterIp = found.toString();
        cachedMasterIp = masterIp;
        return true;
    }

    return false;
}

// ================= CALLBACK PENERIMAAN PESAN DARI MASTER =================
static void onMessageCallback(WebsocketsMessage message) {
    String payload = message.data();
    if (payload.length() == 0) return;

    lastWsMessageTime = millis();

    if (!payload.startsWith("[")) {
        Serial.printf("[WS-RX]: %s\n", payload.c_str());
    }

    DynamicJsonDocument doc(16384);
    DeserializationError err = deserializeJson(doc, payload);
    if (err) {
        Serial.printf("[JSON ERR]: %s\n", err.c_str());
        return;
    }

    // ================= 1. JIKA PAYLOAD BERUPA ARRAY [...] =================
    // Broadcast roster lengkap dari Master.
    // Menjamin setiap kursi yang tidak ada di roster otomatis di-reset menjadi kosong (hijau).
    if (doc.is<JsonArray>()) {
        JsonArray arr = doc.as<JsonArray>();
        ui_sync_roster_begin();
        for (JsonObject obj : arr) {
            const char* rawId = obj["id"] | "";
            if (strlen(rawId) == 0) continue;
            String cleanId = normalizeSeatId(rawId);
            if (cleanId == "RANDOMID") continue;

            bool occupied = obj["occupied"] == true || obj["occupied"] == 1 || obj["occupied"] == "1";
            bool seatbelt = obj["seatbelt"] == true || obj["seatbelt"] == 1 || obj["seatbelt"] == "1";
            bool calling = obj["calling"] == true || obj["callPending"] == true || obj["calling"] == 1;

            ui_sync_roster_seat(cleanId.c_str(), occupied, seatbelt, calling);
        }
        ui_sync_roster_end();
    }
    // ================= 2. JIKA PAYLOAD BERUPA OBJEK {...} =================
    else if (doc.is<JsonObject>()) {
        const char* status = doc["status"] | "";
        const char* event = doc["event"] | "";

        // Handshake role
        if (strcmp(status, "identify") == 0) {
            Serial.println("[WS] Master meminta identify -> Mengirim role:headunit");
            wsClient.send("{\"role\":\"headunit\"}");
            wsClient.send("roster");
            return;
        }

        // ================= EVENT PAIRING DARI MASTER =================
        // Diterima saat modul kursi tanpa ID (RandomID) memencet tombol atau diduduki
        if (strcmp(event, "seatPairingRequired") == 0 || strcmp(event, "seatPairingRequest") == 0) {
            const char* uid = doc["uid"] | "";
            if (strlen(uid) > 0) {
                Serial.printf("[WS-PAIRING] Event %s terdeteksi untuk UID: %s -> Munculkan Pop-up!\n", event, uid);
                ui_show_pairing_dialog(uid);
                return;
            }
        }
        else if (strcmp(event, "seatPairingResult") == 0) {
            bool success = doc["success"] | false;
            const char* id = doc["id"] | "";
            const char* errMsg = doc["error"] | "";
            Serial.printf("[WS-PAIRING] Hasil pairing: success=%d, id=%s, err=%s\n", success, id, errMsg);
            ui_on_pairing_result(success, id, errMsg);
            return;
        }

        // Failsafe deteksi modul RandomID yang trigger unassigned
        if ((doc["unassigned"] == true || strcmp(doc["id"] | "", "RandomID") == 0) && doc.containsKey("uid")) {
            const char* uid = doc["uid"] | "";
            if (strlen(uid) > 0) {
                Serial.printf("[WS-PAIRING-FAILSAFE] Modul unassigned terpicu, UID: %s\n", uid);
                ui_show_pairing_dialog(uid);
                return;
            }
        }

        // Pembaruan Voltase
        if (doc.containsKey("voltage")) {
            ui_update_voltage(doc["voltage"]);
        } else if (doc.containsKey("vdc")) {
            ui_update_voltage(doc["vdc"]);
        } else if (strcmp(status, "inverter") == 0 && doc.containsKey("vdc")) {
            ui_update_voltage(doc["vdc"]);
        }

        // Pembaruan Jam
        if (doc.containsKey("time")) {
            const char* t = doc["time"];
            ui_update_clock(t);
        }

        // Format Map "seats": {"1A": {...}, "1B": {...}}
        if (doc.containsKey("seats") && doc["seats"].is<JsonObject>()) {
            JsonObject seatsMap = doc["seats"].as<JsonObject>();
            for (JsonPair kv : seatsMap) {
                String cleanId = normalizeSeatId(kv.key().c_str());
                if (cleanId == "RANDOMID") continue;
                JsonObject val = kv.value().as<JsonObject>();
                bool occ = val["occupied"] == true || val["occupied"] == 1 || val["occupied"] == "1";
                bool sb = val["seatbelt"] == true || val["seatbelt"] == 1 || val["seatbelt"] == "1";
                bool call = val["calling"] == true || val["callPending"] == true || val["calling"] == 1;
                ui_update_seat(cleanId.c_str(), occ, sb, call);
            }
        }
        // Event Individual Sensor Trigger
        else if (doc.containsKey("id") || doc.containsKey("seatId")) {
            const char* rawId = doc.containsKey("id") ? doc["id"] : doc["seatId"];
            String cleanId = normalizeSeatId(rawId);
            if (cleanId != "RANDOMID" && cleanId.length() > 0) {
                int sensorState = doc["sensorState"] | 0;
                bool occ = doc["occupied"] == true || doc["occupied"] == 1 || 
                           doc["seatOccupied"] == true || sensorState == 1;
                bool sb = doc["seatbelt"] == true || doc["seatbelt"] == 1 || doc["seatbelt"] == "1";
                bool call = doc["calling"] == true || doc["callPending"] == true || sensorState == 1;
                ui_update_seat(cleanId.c_str(), occ, sb, call);
            }
        }
    }
}

// ================= CALLBACK EVENT WEBSOCKET =================
static void onEventsCallback(WebsocketsEvent event, String data) {
    if (event == WebsocketsEvent::ConnectionOpened) {
        Serial.println("[WS] WebSocket Terhubung ke Master!");
        wsConnected = true;
        lastWsMessageTime = millis();
        ui_update_network_status(true, masterIp.c_str());

        wsClient.send("{\"role\":\"headunit\"}");
        wsClient.send("roster");
    } else if (event == WebsocketsEvent::ConnectionClosed) {
        Serial.println("[WS] WebSocket Terputus dari Master.");
        wsConnected = false;
        ui_update_network_status(false, "DISCONNECTED");
    }
}

// ================= MENYAMBUNGKAN WEBSOCKET KE MASTER =================
static void connectWebSocket() {
    if (WiFi.status() != WL_CONNECTED) return;

    if (masterIp.length() == 0) {
        if (!resolveMasterIp()) {
            Serial.println("[WS] Master belum ditemukan di jaringan. Akan mencoba lagi...");
            return;
        }
    }

    String wsUrl = "ws://" + masterIp + ":" + String(MASTER_WS_PORT) + "/";
    Serial.printf("[WS] Menyambung ke %s...\n", wsUrl.c_str());

    wsClient.onMessage(onMessageCallback);
    wsClient.onEvent(onEventsCallback);

    if (wsClient.connect(wsUrl)) {
        wsConnected = true;
        lastWsMessageTime = millis();
        Serial.println("[WS] Handshake WebSocket berhasil!");
        ui_update_network_status(true, masterIp.c_str());
        wsClient.send("{\"role\":\"headunit\"}");
        wsClient.send("roster");
    } else {
        wsConnected = false;
        Serial.printf("[WS] Gagal menyambung ke %s. Mencari IP ulang...\n", wsUrl.c_str());
        masterIp = "";
        ui_update_network_status(false, "DISCONNECTED");
    }
}

// ================= INISIALISASI NETWORK =================
void network_init() {
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.persistent(true);
    if (strlen(WIFI_PASS) > 0) {
        WiFi.begin(WIFI_SSID, WIFI_PASS);
    } else {
        WiFi.begin(WIFI_SSID);
    }
    Serial.printf("[WiFi] Memulai koneksi ke SSID '%s'...\n", WIFI_SSID);
    ui_update_network_status(false, "MENCARI WIFI");
}

// ================= LOOP NETWORK (CORE 0 FREERTOS TASK) =================
void network_loop() {
    wl_status_t status = WiFi.status();

    if (status == WL_CONNECTED) {
        if (!wifiConnected) {
            wifiConnected = true;
            String localIp = WiFi.localIP().toString();
            Serial.printf("[WiFi] Berhasil terkoneksi ke '%s'! IP: %s\n", WIFI_SSID, localIp.c_str());

            if (!mdnsStarted) {
                if (MDNS.begin("esp32s3-passenger")) {
                    Serial.println("[mDNS] Responder siap: esp32s3-passenger.local");
                    mdnsStarted = true;
                }
            }
        }

        // Handle WebSocket ke Master
        if (!wsConnected) {
            if (millis() - lastWsRetry > 2000) {
                lastWsRetry = millis();
                connectWebSocket();
            }
        } else {
            // SILENCE WATCHDOG (3.5s)
            if (millis() - lastWsMessageTime > 3500) {
                Serial.println("[WS-WATCHDOG] Master hening > 3.5 detik! Memutus zombie connection...");
                wsConnected = false;
                wsClient.close();
                ui_update_network_status(false, "DISCONNECTED");
                masterIp = "";
                lastWsRetry = millis();
            } else {
                wsClient.poll();
            }
        }
    } else {
        if (wifiConnected) {
            wifiConnected = false;
            wsConnected = false;
            masterIp = "";
            Serial.println("[WiFi] Koneksi WiFi terputus!");
            ui_update_network_status(false, "WIFI TERPUTUS");
        }

        if (millis() - lastWifiRetry > 8000) {
            lastWifiRetry = millis();
            Serial.printf("[WiFi] Menghubungkan ulang ke '%s'...\n", WIFI_SSID);
            WiFi.disconnect();
            if (strlen(WIFI_PASS) > 0) {
                WiFi.begin(WIFI_SSID, WIFI_PASS);
            } else {
                WiFi.begin(WIFI_SSID);
            }
        }
    }
}
