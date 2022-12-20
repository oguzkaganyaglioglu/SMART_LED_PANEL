//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//


#include <WiFi_Manager.h>

wl_status_t WiFi_Manager::status() {
#ifdef DEBUG_MODE
    Serial.println("WM_S");
#endif
    //Serial.printf("WM::Status: %u %u\n", (uint8_t) WiFi.status(), (uint8_t) WiFi.getSleepMode());
    return WiFi.status();
}

void WiFi_Manager::connect(const char *ssid, const char *pass, int32_t channel, uint8_t *bssid) {
#ifdef DEBUG_MODE
    Serial.println("WM_C");
#endif
    scanDelete();
    disconnect();
    WiFi.begin(ssid, pass, channel, bssid);
    ESP8266WiFiClass::persistent(true);
}

void WiFi_Manager::disconnect() {
#ifdef DEBUG_MODE
    Serial.println("WM_D");
#endif
    //Serial.println("WM::Disconnecting...");
    /*FixMe: WiFi.disconnect() cause wdt reset*/
    //WiFi.disconnect();
    PerfectLightsCDController::mqttDisconnect();
}

void WiFi_Manager::startScan() {
#ifdef DEBUG_MODE
    Serial.println("WM_SS");
#endif
    WiFi.scanDelete();
    WiFi.scanNetworks(true);
}

void WiFi_Manager::scanDelete() {
#ifdef DEBUG_MODE
    Serial.println("WM_SD");
#endif
    WiFi.scanDelete();
}

bool WiFi_Manager::scanInProgress() {
#ifdef DEBUG_MODE
    Serial.println("WM_SIP");
#endif
    return WiFi.scanComplete() == -1;
}

bool WiFi_Manager::scanDone() {
#ifdef DEBUG_MODE
    Serial.println("WM_SDONE");
#endif
    return WiFi.scanComplete() >= 0;
}

void WiFi_Manager::init() {
#ifdef DEBUG_MODE
    Serial.println("WM_I");
#endif
    WiFi.hostname("PLCD_SMART_LED_PANEL");
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    WiFi.softAP(AP_SSID, AP_PASS);
    WiFi.setAutoConnect(true);
    WiFi.setAutoReconnect(true);
    WiFi.begin();
}
