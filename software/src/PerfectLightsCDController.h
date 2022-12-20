//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef LED_PANEL_PERFECTLIGHTSCDCONTROLLER_H
#define LED_PANEL_PERFECTLIGHTSCDCONTROLLER_H

#include <Arduino.h>
#include <Settings.h>
#include <PLCustomDevices.h>
#include <LedController.h>
#include <AnalogReader.h>


class PerfectLightsCDController {
public:
    static void init(WiFiClient &wifiClient);

    static void analogReaderSync();

    static void loop();

    static bool mqttConnected();

    static bool mqttDisconnect();

public:

private:
    static const char *userID;      // The user ID
    static const char *clientID;    // The ID (device ID) of your custom device
    static const char *username;   // The MQTT username of your custom device
    static const char *pass;       // The MQTT password of your custom device
    static const char *mqttHost;       // The MQTT host
    static const uint16_t mqttPort;    // The MQTT port

    static uint32_t analogReaderLastUpdate;
    static bool analogReaderUpdated;

    static void callback(JsonDocument &_doc);

    static PLCustomDevices *myDevice;
    static WiFiClient *myWifiClient;
public:

};


#endif //LED_PANEL_PERFECTLIGHTSCDCONTROLLER_H
