//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef WiFi_Manager_h
#define WiFi_Manager_h

#include <Settings.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PerfectLightsCDController.h>

class WiFi_Manager
{
private:
    /* data */
public:
    static wl_status_t status();
    static void startScan();
    static bool scanDone();
    static void scanDelete();
    static bool scanInProgress();
    static void disconnect();
    static void connect(const char *, const char *, int32_t = 0, uint8_t * = nullptr);
    static void init();
    //void loop();
};

#endif