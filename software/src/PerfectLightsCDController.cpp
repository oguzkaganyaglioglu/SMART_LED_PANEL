//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#include "PerfectLightsCDController.h"

uint32_t PerfectLightsCDController::analogReaderLastUpdate = 0;
bool PerfectLightsCDController::analogReaderUpdated = false;
PLCustomDevices *PerfectLightsCDController::myDevice = nullptr;
WiFiClient *PerfectLightsCDController::myWifiClient = nullptr;
const char *PerfectLightsCDController::userID = PERFECT_LIGHTS_CD_DEVICE_USER_ID;    // The user ID
const char *PerfectLightsCDController::clientID = PERFECT_LIGHTS_CD_DEVICE_ID;       // The ID (device ID) of your custom device
const char *PerfectLightsCDController::username = PERFECT_LIGHTS_CD_MQTT_USERNAME;   // The MQTT username of your custom device
const char *PerfectLightsCDController::pass = PERFECT_LIGHTS_CD_MQTT_PASSWORD;       // The MQTT password of your custom device
const char *PerfectLightsCDController::mqttHost = PERFECT_LIGHTS_CD_MQTT_HOST;       // The MQTT host
const uint16_t PerfectLightsCDController::mqttPort = PERFECT_LIGHTS_CD_MQTT_PORT;    // The MQTT port


void PerfectLightsCDController::init(WiFiClient &wifiClient) {
    myWifiClient = &wifiClient;
    myDevice = new PLCustomDevices(myWifiClient, mqttHost, &mqttPort);
    myDevice->setUserID(userID);
    myDevice->setAuthInfos(clientID, username, pass);
    myDevice->init();
    myDevice->setJSONCallback(callback, true);
}

void PerfectLightsCDController::callback(JsonDocument &_doc) {

    Serial.printf("C1\n");
    serializeJsonPretty(_doc, Serial);
    Serial.flush();

    if (_doc["command"] == "synk") {
        _doc.clear();
        _doc["online"] = true;
        // if brightness equal to 0 we assume device is off
        _doc["on"] = LedController::getBrightnessTarget() != 0;
        _doc["brightness"] = map(LedController::getBrightnessTarget(), 0, 255, 0, 100);
        _doc["color"]["temperatureK"] = LedController::getKelvinVal(true);

        Serial.printf("RESPONSE: %d", myDevice->sendResponse());
    } else if (_doc["command"] == "action.devices.commands.OnOff") {
        // {"command":"","params":{"on":false}}
        // apply incoming request
        // turn on request = set brightness to 20%
        // turn off request set brightness to 0%
        AnalogReader::lock();
        LedController::setBrightness(_doc["params"]["on"] == true ? 51 : 0);
        // create the response
        _doc.clear();
        _doc["online"] = true;
        // if brightness equal to 0 we assume device is off
        _doc["on"] = LedController::getBrightnessTarget() != 0;
        _doc["brightness"] = map(LedController::getBrightnessTarget(), 0, 255, 0, 100);
        Serial.printf("RESPONSE: %d", myDevice->sendResponse());
        myDevice->sendSyncReq();

    } else if (_doc["command"] == "action.devices.commands.BrightnessAbsolute") {
        // {"command":"","params":{"brightness":87}}
        // apply incoming request
        AnalogReader::lock();
//        LedController::setBrightness(map(_doc["params"]["brightness"], 0, 100, 0, 255));
        LedController::setBrightnessWTransition(true, map(_doc["params"]["brightness"], 0, 100, 0, 255));
        // create the response
        _doc.clear();
        _doc["online"] = true;
        _doc["brightness"] = map(LedController::getBrightnessTarget(), 0, 255, 0, 100);
        Serial.printf("RESPONSE: %d", myDevice->sendResponse());
        myDevice->sendSyncReq();
    } else if (_doc["command"] == "action.devices.commands.ColorAbsolute") {
        // {"command":"","params":{"color":{"name":"ivory","temperature":6000}}}
        // apply incoming request
        AnalogReader::lock();
        LedController::setKelvinVal(_doc["params"]["color"]["temperature"]);
        // create the response
        _doc.clear();
        _doc["online"] = true;
        _doc["color"]["temperatureK"] = LedController::getKelvinVal(true);
        Serial.printf("RESPONSE: %d", myDevice->sendResponse());
        myDevice->sendSyncReq();
    }
}

void PerfectLightsCDController::loop() {
    if (myDevice == nullptr) return;
    if (analogReaderUpdated) {
        if (millis() - analogReaderLastUpdate > TIME_BETWEEN_SYNC_REQUESTS_MS) {
            analogReaderUpdated = !(myDevice->sendSyncReq());
        }
    }
    myDevice->loop();
}

void PerfectLightsCDController::analogReaderSync() {
    analogReaderLastUpdate = millis();
    analogReaderUpdated = true;
}

bool PerfectLightsCDController::mqttConnected() {
    return myDevice->mqttConnected();
}

bool PerfectLightsCDController::mqttDisconnect() {
    return myDevice->mqttDisconnect();
}
