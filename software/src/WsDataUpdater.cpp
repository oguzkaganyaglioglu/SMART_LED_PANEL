//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#include <WsDataUpdater.h>

WsDataUpdater::WsDataUpdater(Async_Server *myServer) {
#ifdef DEBUG_MODE
    Serial.println("WDU_WDU");
#endif
    this->myServer = myServer;
}

void WsDataUpdater::updateWsClients(const String &_loadingTitle) {
#ifdef DEBUG_MODE
    Serial.println("WDU_UWC");
#endif
    updateWsData(_loadingTitle);
    this->myServer->sendMSG(this->responseBuffer, this->responseLen);
}

//void WsDataUpdater::updateWsClients(const String &_loadingTitle) {
//    bool _isTimeout = false;
//    if (_loadingTitle != "") {
//        if (isTimeout() ||
//            (_loadingTitle == "Disconnecting" && (WiFi_Manager::status() == 7 || WiFi_Manager::status() == 0)) ||
//            (_loadingTitle.indexOf("Connecting to") == 0 && WiFi_Manager::status() != 7)) {
//            this->setLoadingTitle("");
//            _isTimeout = isTimeout();
//            setTimeout(0);
//        }
//    }
//
//    this->doc.clear();
//    this->doc["NTC"]["temp"] = AnalogReader::calculateNTC();
//    this->doc["LED"]["COLD"] = LedController::getColdLedVal();
//    this->doc["LED"]["WARM"] = LedController::getWarmLedVal();
//    this->doc["LED"]["BRIGHTNESS"] = LedController::getBrightness();
//    this->doc["WIFI"]["STATUS"] = WiFi_Manager::status();
//    this->doc["WIFI"]["CONNECTED"]["RSSI"] = WiFi.RSSI();
//    this->doc["WIFI"]["CONNECTED"]["SSID"] = WiFi.SSID();
//    this->doc["WIFI"]["CONNECTED"]["IP"] = WiFi.localIP();
//
//    this->doc["WIFI"]["SCAN"]["STATUS"] = WiFi_Manager::scanInProgress();
//    this->doc["WIFI"]["SCAN"]["DONE"] = WiFi_Manager::scanDone();
//
//    for (int i = 0; i < WiFi.scanComplete(); i++) {
//        this->doc["WIFI"]["SCAN"]["RESULT"][i]["SSID"] = WiFi.SSID(i);
//        this->doc["WIFI"]["SCAN"]["RESULT"][i]["CHANNEL"] = WiFi.channel(i);
//        this->doc["WIFI"]["SCAN"]["RESULT"][i]["RSSI"] = WiFi.RSSI(i);
//        this->doc["WIFI"]["SCAN"]["RESULT"][i]["OPEN"] = WiFi.encryptionType(i) == ENC_TYPE_NONE;
//    }
//
//    this->doc["LOADING"]["OPEN"] = _loadingTitle != "";
//    this->doc["LOADING"]["TITLE"] = _loadingTitle;
//    this->doc["TIMEOUT"] = _isTimeout;
//
//    size_t len = serializeJson(this->doc, this->responseBuffer);
//
//    this->myServer->sendMSG(this->responseBuffer, len);
//#ifdef PRINT_WEBSOCKET_HIGH_DEBUG
//    Serial.println("CLIENTS DATA UPDATE SENT");
//#endif
//    this->lastUpdate = millis();
//}

void WsDataUpdater::loop() {
#ifdef DEBUG_MODE
    Serial.println("WDU_L");
#endif
    if (this->_startLoop)
        if (millis() - this->startTimer > this->startTimerMS) {
            this->_startLoop = false;
            this->_pauseLoop = false;
            this->startTimer = 0;
            this->startTimerMS = 0;
        }

    if (this->_pauseLoop) return;

    if (millis() - lastUpdate > WS_UPDATE_EVERY_X_MILLIS)
        this->updateWsData(this->loadingTitle);

    while (this->updateToSendList) {
        this->myServer->sendMSGtoClient(this->updateToSendList->value, this->responseBuffer, this->responseLen);
        auto temp = this->updateToSendList;
        this->updateToSendList = this->updateToSendList->next;
        delete temp;
    }
}

void WsDataUpdater::pauseLoop() {
#ifdef DEBUG_MODE
    Serial.println("WDU_PL");
#endif
    this->_startLoop = false;
    this->_pauseLoop = true;
}

void WsDataUpdater::resumeLoop(unsigned long _ms) {
#ifdef DEBUG_MODE
    Serial.println("WDU_RL");
#endif
    this->_startLoop = true;
    this->startTimer = millis();
    this->startTimerMS = _ms;
}

void WsDataUpdater::applyIncoming(uint32_t id, uint8_t *cmd, size_t len) {
#ifdef DEBUG_MODE
    Serial.println("WDU_AI");
#endif
    deserializeJson(this->docInput, cmd, len);
    if (!this->docInput["LED"].isNull()) {
        if (!this->docInput["LED"]["BRIGHTNESS"].isNull()) {
#ifdef PRINT_WEBSOCKET_HIGH_DEBUG
            Serial.println("WS BRIGHTNESS UPDATE");
#endif
            AnalogReader::lock();
            LedController::setBrightness((uint8_t) (this->docInput["LED"]["BRIGHTNESS"]));
            this->doc["LED"]["BRIGHTNESS"] = LedController::getBrightnessTarget();

        }

        if (!this->docInput["LED"]["COLD"].isNull() && !this->docInput["LED"]["WARM"].isNull()) {
#ifdef PRINT_WEBSOCKET_HIGH_DEBUG
            Serial.println("WS COLOR KELVIN UPDATE");
#endif
            AnalogReader::lock();
            LedController::setColdLedVal((uint8_t) (this->docInput["LED"]["COLD"]));
            LedController::setWarmLedVal((uint8_t) (this->docInput["LED"]["WARM"]));
            this->doc["LED"]["COLD"] = LedController::getColdLedValTarget();
            this->doc["LED"]["WARM"] = LedController::getWarmLedValTarget();
        }
//        this->updateWsData(this->loadingTitle);
    }

    if (!this->docInput["WIFI"].isNull()) {
        if (!this->docInput["WIFI"]["SCAN"].isNull()) {
            //Serial.println("Scanning...");
            WiFi_Manager::startScan();
        }

        if (!this->docInput["WIFI"]["SCAN_DEL"].isNull()) {
            //Serial.println("Scan deleting...");
            this->doc["WIFI"]["SCAN"]["RESULT"].clear();
            WiFi_Manager::scanDelete();
        }

        if (!this->docInput["WIFI"]["CONNECT"].isNull()) {
            //Serial.printf("Connect to %s\n", ((String) this->docInput["WIFI"]["CONNECT"]["SSID"]).c_str());
            this->setLoadingTitle("Connecting to<br/>" + (String) this->docInput["WIFI"]["CONNECT"]["SSID"]);
            WiFi_Manager::connect(((String) this->docInput["WIFI"]["CONNECT"]["SSID"]).c_str(),
                                  ((String) this->docInput["WIFI"]["CONNECT"]["PASS"]).c_str());
            setTimeout(20000);
        }

        if (!this->docInput["WIFI"]["DISCONNECT"].isNull()) {
            //Serial.printf("Disconnecting \n");
            this->setLoadingTitle("Disconnecting");
            WiFi_Manager::disconnect();
            setTimeout(3000);
        }
//        this->updateWsData(this->loadingTitle);
        this->lastUpdate = 0;
    }

    if (!this->docInput["EXECUTE"].isNull()) {
        if (this->docInput["EXECUTE"] == "UPDATE") {
#ifdef PRINT_WEBSOCKET_HIGH_DEBUG
            Serial.println("UPDATE REQUEST");
#endif

            if (this->updateToSendList)
                this->updateToSendList = new _LinkedList<uint32_t>(id, this->updateToSendList->next);
            else this->updateToSendList = new _LinkedList<uint32_t>(id);
//            updateWsClients(loadingTitle); //THIS SHOULD BE RUN LAST
        }
    }
}

void WsDataUpdater::setLoadingTitle(const String &_loadingTitle) {
#ifdef DEBUG_MODE
    Serial.println("WDU_SLT");
#endif
    //Serial.printf("%s  ->  %s\n", this->loadingTitle.c_str(), _loadingTitle.c_str());
    this->loadingTitle = _loadingTitle;
}

void WsDataUpdater::setTimeout(unsigned long _timeout) {
#ifdef DEBUG_MODE
    Serial.println("WDU_STO");
#endif
    this->timeout = _timeout;
    this->timeoutSet = millis();
}

bool WsDataUpdater::isTimeout() {
#ifdef DEBUG_MODE
    Serial.println("WDU_ITO");
#endif
    return this->timeout != 0 && millis() - timeoutSet > timeout;
}

void WsDataUpdater::updateWsData(const String &_loadingTitle) {
#ifdef DEBUG_MODE
    Serial.println("WDU_UWD");
#endif
//    return; //todo find the issue why NODEMCU resets in here, try to use nested objects
    bool _isTimeout = false;
    if (_loadingTitle != "") {
        if (isTimeout() ||
            (_loadingTitle == "Disconnecting" && (WiFi_Manager::status() == 7 || WiFi_Manager::status() == 0)) ||
            (_loadingTitle.indexOf("Connecting to") == 0 && WiFi_Manager::status() != 7)) {
            this->setLoadingTitle("");
            _isTimeout = isTimeout();
            setTimeout(0);
        }
    }

    this->doc.clear();
    this->doc["NTC"]["temp"] = AnalogReader::calculateNTC();
    this->doc["LED"]["COLD"] = LedController::getColdLedValTarget();
    this->doc["LED"]["WARM"] = LedController::getWarmLedValTarget();
    this->doc["LED"]["BRIGHTNESS"] = LedController::getBrightnessTarget();
    this->doc["WIFI"]["STATUS"] = WiFi_Manager::status();
    this->doc["WIFI"]["CONNECTED"]["RSSI"] = WiFi.RSSI();
    this->doc["WIFI"]["CONNECTED"]["SSID"] = WiFi.SSID();
    this->doc["WIFI"]["CONNECTED"]["IP"] = WiFi.localIP();

    this->doc["WIFI"]["SCAN"]["STATUS"] = WiFi_Manager::scanInProgress();
    this->doc["WIFI"]["SCAN"]["DONE"] = WiFi_Manager::scanDone();
    for (int i = 0; i < WiFi.scanComplete(); i++) {
        this->doc["WIFI"]["SCAN"]["RESULT"][i]["SSID"] = WiFi.SSID(i);
        this->doc["WIFI"]["SCAN"]["RESULT"][i]["CHANNEL"] = WiFi.channel(i);
        this->doc["WIFI"]["SCAN"]["RESULT"][i]["RSSI"] = WiFi.RSSI(i);
        this->doc["WIFI"]["SCAN"]["RESULT"][i]["OPEN"] = WiFi.encryptionType(i) == ENC_TYPE_NONE;
    }

    this->doc["LOADING"]["OPEN"] = _loadingTitle != "";
    this->doc["LOADING"]["TITLE"] = _loadingTitle;
    this->doc["TIMEOUT"] = _isTimeout;
    this->responseLen = serializeJson(this->doc, this->responseBuffer);
    this->lastUpdate = millis();
}

