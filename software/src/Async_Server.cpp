//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#include <Async_Server.h>

bool Async_Server::reboot = false;
AsyncEventSource Async_Server::eventLog = AsyncEventSource("/eventLog");
AsyncWebSocket Async_Server::wsLog = AsyncWebSocket("/wsLog");

Async_Server::Async_Server(uint16_t port, const char *wsUrl) {
#ifdef DEBUG_MODE
    Serial.println("AS_");
#endif
    this->server = new AsyncWebServer(port);
    this->ws = new AsyncWebSocket(wsUrl);
}

Async_Server::~Async_Server() = default;

void Async_Server::attachWsUpdater(WsDataUpdater *_wsUpdater) {
#ifdef DEBUG_MODE
    Serial.println("AS_AWU");
#endif
    this->wsUpdater = _wsUpdater;
}

void Async_Server::init() {
#ifdef DEBUG_MODE
    Serial.println("AS_I");
#endif
    if (!LittleFS.begin()) {
#ifdef PRINT_LITTLEFS_DEBUG
        Serial.println("An Error has occurred while mounting LittleFS");
#endif
        return;
    }

    this->ws->onEvent([this](auto &&PH1, auto &&PH2, auto &&PH3, auto &&PH4, auto &&PH5, auto &&PH6) {
        onEvent(std::forward<decltype(PH1)>(PH1), std::forward<decltype(PH2)>(PH2), std::forward<decltype(PH3)>(PH3),
                std::forward<decltype(PH4)>(PH4), std::forward<decltype(PH5)>(PH5), std::forward<decltype(PH6)>(PH6));
    });

    eventLog.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.printf("Client reconnected! Last message ID that it gat is: %u\n", client->lastId());
        }
        //send event with message "hello!", id current millis
        // and set reconnect delay to 1 second
        client->send("hello!", nullptr, millis(), 1000);
    });

    this->server->addHandler(&wsLog);

    this->server->addHandler(&eventLog);
    this->server->addHandler(this->ws);

    this->server->on("/", HTTP_GET,
                     [](AsyncWebServerRequest *request) {
#ifdef LOGIN_REQUIRED_CONTROL_PANEL
                         if (loginRequired(request)) return;
#endif
                         sendCompressedIfSupported(request, "/WEBSITE/", "controlPanel.html");

                     });
    this->server->on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef LOGIN_REQUIRED_CONTROL_PANEL
        if (loginRequired(request)) return;
#endif
        sendCompressedIfSupported(request, "/WEBSITE/", "style.css", "text/css");
    });

    this->server->on("/updateStyle.css", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef LOGIN_REQUIRED_CONTROL_PANEL
        if (loginRequired(request)) return;
#endif
        sendCompressedIfSupported(request, "/WEBSITE/", "updateStyle.css", "text/css");
    });

    this->server->on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef LOGIN_REQUIRED_CONTROL_PANEL
        if (loginRequired(request)) return;
#endif
        sendCompressedIfSupported(request, "/WEBSITE/", "script.js", "text/javascript");

    });
    this->server->on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request) {
        sendCompressedIfSupported(request, "/WEBSITE/", "favicon.ico", "image/png");

    });

    this->server->serveStatic("/fonts/", LittleFS, "/WEBSITE/fonts/");
//    this->server->on("/fonts/Montserrat-Regular.ttf", HTTP_GET, [](AsyncWebServerRequest *request) {
//        request->send(LittleFS, "/WEBSITE/fonts/Montserrat-Regular.ttf", "font/ttf");
//    }).setFilter(ON_AP_FILTER);
//    this->server->on("/fonts/Montserrat-Medium.ttf", HTTP_GET, [](AsyncWebServerRequest *request) {
//        request->send(LittleFS, "/WEBSITE/fonts/Montserrat-Medium.ttf", "font/ttf");
//    }).setFilter(ON_AP_FILTER);
//    this->server->on("/fonts/Montserrat-SemiBold.ttf", HTTP_GET, [](AsyncWebServerRequest *request) {
//        request->send(LittleFS, "/WEBSITE/fonts/Montserrat-SemiBold.ttf", "font/ttf");
//    }).setFilter(ON_AP_FILTER);
//    this->server->on("/fonts/RobotoMono-SemiBold.ttf", HTTP_GET, [](AsyncWebServerRequest *request) {
//        request->send(LittleFS, "/WEBSITE/fonts/RobotoMono-SemiBold.ttf", "font/ttf");
//    }).setFilter(ON_AP_FILTER);

    this->server->on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(401);
    });

    this->server->on("/update", HTTP_GET, [](AsyncWebServerRequest *request) {
#ifdef LOGIN_REQUIRED_UPDATE
        if (loginRequired(request)) return;
#endif
        sendCompressedIfSupported(request, "/WEBSITE/", "update.html");

    });
    this->server->on("/update", HTTP_POST, [this](AsyncWebServerRequest *request) {
#ifdef LOGIN_REQUIRED_UPDATE
        if (loginRequired(request)) return;
#endif

        Async_Server::reboot = !Update.hasError();
        AsyncWebServerResponse *response = request->beginResponse(200, "text/plain",
                                                                  Async_Server::reboot ? "OK" : "FAIL");
        response->addHeader("Connection", "close");
        request->send(response);
        ShiftRegisterLCD::updateDone(Async_Server::reboot);
        if (!Async_Server::reboot) {
            this->wsUpdater->setLoadingTitle("");
            this->wsUpdater->updateWsClients();
        }

    }, [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len,
              bool final) {
        if (!index) {
            this->wsUpdater->setLoadingTitle("Updating");
            this->wsUpdater->updateWsClients();
            Serial.printf("Update Start: %s\n", filename.c_str());
            ShiftRegisterLCD::updateStart();
            Update.runAsync(true);
            if (!Update.begin((EspClass::getFreeSketchSpace() - 0x1000) & 0xFFFFF000)) {
                Update.printError(Serial);
            }
        }
        if (!Update.hasError()) {
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
            }
        }
        if (final) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %uB\n", index + len);
            } else {
                Update.printError(Serial);

            }
        }
    });

    this->server->onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("/");
    });
    this->server->begin();
}

void Async_Server::onEvent([[maybe_unused]] AsyncWebSocket
                           *webSocketServer,
                           AsyncWebSocketClient *client,
                           AwsEventType
                           type,
                           void *arg, uint8_t
                           *data,
                           size_t len
) {
#ifdef DEBUG_MODE
    Serial.println("AS_OE");
#endif
    switch (type) {
        case WS_EVT_CONNECT:
#ifdef PRINT_WEBSOCKET_DEBUG
            Serial.printf("#%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
#endif
            if (this->ws->count() > MAX_WEB_SOCKET_CLIENT) {
                client->text("WS_CLIENT_OVERFLOW");
                client->close();
            }
            break;
        case WS_EVT_DISCONNECT:
#ifdef PRINT_WEBSOCKET_DEBUG
            Serial.printf("#%u disconnected\n", client->id());
#endif
            break;
        case WS_EVT_DATA:

            handleWebSocketMessage(client, arg, data, len
            );
            break;
        case WS_EVT_PONG:
#ifdef PRINT_WEBSOCKET_DEBUG
            Serial.printf("ws[%s][%u] pong[%u]: %s\n", webSocketServer->url(), client->id(), len, (len)?(char*)data:"");
#endif
            break;
        case WS_EVT_ERROR:
#ifdef PRINT_WEBSOCKET_DEBUG
            Serial.printf("ws[%s][%u] error(%u): %s\n", webSocketServer->url(), client->id(), *((uint16_t *) arg), (char *) data);
#endif
            break;
    }
}

void Async_Server::handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
#ifdef DEBUG_MODE
    Serial.println("AS_HWSM");
    Serial.printf("FH: %u\n", system_get_free_heap_size());
#endif
    EspClass::wdtFeed();
    auto *info = (AwsFrameInfo *) arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        this->wsUpdater->pauseLoop();
        this->wsUpdater->applyIncoming(client->id(), data, len);
        this->wsUpdater->resumeLoop(100);
    }
}

void Async_Server::sendMSG(char *message, size_t len) {
#ifdef DEBUG_MODE
    Serial.println("AS_SMSG");
#endif
    this->ws->textAll(message, len);
}

void Async_Server::loop() {
#ifdef DEBUG_MODE
    Serial.println("AS_L");
#endif
    if (Async_Server::reboot) {
        this->wsUpdater->setLoadingTitle("Rebooting");
        this->wsUpdater->updateWsClients();
        Serial.println("Rebooting...");
        delay(100);
        EspClass::restart();
    }
    this->ws->cleanupClients(MAX_WEB_SOCKET_CLIENT);
//    if (millis() - this->timerWsClean > 200) {
//        this->ws->cleanupClients(MAX_WEB_SOCKET_CLIENT);
//        this->timerWsClean = millis();
//    }
}

bool Async_Server::loginRequired(AsyncWebServerRequest *request) {
#ifdef DEBUG_MODE
    Serial.println("AS_LR");
#endif
    bool temp = !request->authenticate(HTTP_USERNAME, HTTP_PASSWORD);
    if (temp) request->requestAuthentication();
    return temp;
}

void
Async_Server::sendCompressedIfSupported(AsyncWebServerRequest *request, const String &path, const String &file,
                                        const String &contentType) {
#ifdef DEBUG_MODE
    Serial.println("AS_SCMPIS");
#endif
    if (request->hasHeader("Accept-Encoding") &&
        request->getHeader("Accept-Encoding")->value().indexOf("gzip") != -1) {
        AsyncWebServerResponse *response = request->beginResponse(LittleFS,
                                                                  path + "gz_" + file,
                                                                  contentType);
        response->addHeader("Content-Encoding", "gzip");
        request->send(response);
    } else {
        request->send(LittleFS, path + file);
    }
}

void Async_Server::sendMSGtoClient(uint32_t id, char *message, size_t len) {
#ifdef DEBUG_MODE
    Serial.println("AS_SMSGTC");
#endif
    this->ws->text(id, message, len);
}
