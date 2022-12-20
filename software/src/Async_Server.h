//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef Async_Server_h
#define Async_Server_h

#include <Settings.h>
#include <Arduino.h>
#include <WiFi_Manager.h>
#include <LittleFS.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WsDataUpdater.h>
#include <ShiftRegisterLCD.h>

class WsDataUpdater;

class Async_Server {
private:
    static bool reboot;
    AsyncWebServer *server = NULL;
    AsyncWebSocket *ws = NULL;
    WsDataUpdater *wsUpdater = NULL;
    unsigned long timerWsClean = 0;

    void onEvent([[maybe_unused]] AsyncWebSocket *, AsyncWebSocketClient *, AwsEventType, void *, uint8_t *, size_t);

    void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len);

    static bool loginRequired(AsyncWebServerRequest *request);

    static void sendCompressedIfSupported(AsyncWebServerRequest *request,
                                          const String &path,
                                          const String &file,
                                          const String &contentType = String());

public:
    static AsyncEventSource eventLog;
    static AsyncWebSocket wsLog;

    Async_Server(uint16_t, const char *);

    ~Async_Server();

    void init();

    void sendMSGtoClient(uint32_t id, char *, size_t);

    void sendMSG(char *, size_t);

    void loop();

    void attachWsUpdater(WsDataUpdater *);
};

#endif