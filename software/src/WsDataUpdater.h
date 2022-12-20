//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef WsDataUpdater_h
#define WsDataUpdater_h

#include <Settings.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi_Manager.h>
#include <Async_Server.h>
#include <LedController.h>
#include <AnalogReader.h>

class Async_Server;

template<typename T>
struct _LinkedList {
    _LinkedList *next;
    T value;

    _LinkedList(T value) : next(nullptr), value(value) {};

    _LinkedList(T value, _LinkedList *next) : next(next), value(value) {};
};

class WsDataUpdater {
private:
    Async_Server *myServer = NULL;
    unsigned long lastUpdate = 0;
    char responseBuffer[JSON_BUFFER]{};
    StaticJsonDocument<JSON_BUFFER> doc;
    StaticJsonDocument<JSON_IN_BUFFER> docInput;
    _LinkedList<uint32_t> *updateToSendList = nullptr;
    size_t responseLen{0};
    bool _pauseLoop = false;
    bool _startLoop = false;
    unsigned long startTimer = 0;
    unsigned long startTimerMS = 0;
    String loadingTitle = "";
    unsigned long timeout{0}, timeoutSet{0};

    void setTimeout(unsigned long _timeout);

    bool isTimeout();

    void updateWsData(const String &_loadingTitle);


public:
    void setLoadingTitle(const String &_loadingTitle);

    void updateWsClients() {
        updateWsClients(loadingTitle);
    }

    void updateWsClients(const String &loadingTitle);

    void loop();

    void pauseLoop();

    void resumeLoop(unsigned long = 0);

    void applyIncoming(uint32_t id, uint8_t *cmd, size_t len);

    WsDataUpdater(Async_Server *);
};

#endif