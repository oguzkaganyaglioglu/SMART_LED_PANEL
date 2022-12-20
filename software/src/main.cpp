/*
SMART LED PANEL by Oğuz Kağan Yağlıoğlu
v0.2.0
*/

#include <Settings.h>
#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi_Manager.h>
#include <Async_Server.h>
#include <WsDataUpdater.h>
#include <LedController.h>
#include <AnalogReader.h>
#include <ShiftRegisterLCD.h>
#include <PerfectLightsCDController.h>


DNSServer dnsServer;
Async_Server myServer(80, "/ws");
WsDataUpdater wsUpdater(&myServer);
WiFiClient myWifiClient;

PerfectLightsCDController PLCD;


bool buttonState = false;
bool LCD_DISABLE_WRITE = false;
uint8_t menuIndex = 0;


void setup() {
    analogWriteFreq(26000);
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n");
    Serial.println("We are on!!!");
    Serial.printf("\nResetReason: %u\nResetExccause: %u\n", system_get_rst_info()->reason,
                  system_get_rst_info()->exccause);
    WiFi_Manager::init();

    dnsServer.start(53, "*", WiFi.softAPIP());
    myServer.attachWsUpdater(&wsUpdater);

    Serial.print(AP_SSID);
    Serial.print(" : ");
    Serial.println(WiFi.softAPIP());
    ShiftRegisterLCD::setMutex(LCD_DISABLE_WRITE);
    ShiftRegisterLCD::init();
    myServer.init();
    LedController::init();
    AnalogReader::init();
    PLCD.init(myWifiClient);
    Serial.println("Initialization Completed!!!");


//    ShiftRegisterLCD::clearDisplay();
//    ShiftRegisterLCD::goTo(0, 0);
//
//    ShiftRegisterLCD::write("Access Point IP");
//    ShiftRegisterLCD::goTo(1, 0);
//    ShiftRegisterLCD::write(WiFi.softAPIP().toString());
//
//    delay(5000);
//
//    ShiftRegisterLCD::clearDisplay();
//    ShiftRegisterLCD::goTo(0, 0);
//
//    ShiftRegisterLCD::write(WiFi.SSID().substring(0, 15));
//    ShiftRegisterLCD::goTo(1, 0);
//    ShiftRegisterLCD::write(WiFi.localIP().toString());
//    delay(5000);
//
//    ShiftRegisterLCD::clearDisplay();
//    ShiftRegisterLCD::drawTemplate();

}

unsigned long _t = millis();
unsigned long _t2 = millis();
unsigned long menuLastPageChangeTime = millis();

void loop() {
    dnsServer.processNextRequest();
    myServer.loop();
    wsUpdater.loop();
//    if (WiFi.isConnected()) {
////        Serial.print("IP address:\t");
////        Serial.println(WiFi.localIP());
//    }

    if (millis() - _t > 10) {
        /*100Hz loop*/
        AnalogReader::loop();

        LedController::loop();
        ShiftRegisterLCD::loop();
        PerfectLightsCDController::loop();
        _t = millis();

//        Serial.printf("FH: %u\n", system_get_free_heap_size());
    }

    if (millis() - _t2 > 200) {
        /*5Hz loop*/
        if (AnalogReader::getButton()) {
            if (!buttonState) {
                buttonState = true;
                LCD_DISABLE_WRITE = false;
                menuIndex++;

                switch (menuIndex) {
                    case 1:
                        /*page 2: Access Point IP*/
                        ShiftRegisterLCD::clearDisplay();
                        ShiftRegisterLCD::goTo(0, 0);

                        ShiftRegisterLCD::write("Access Point IP");
                        ShiftRegisterLCD::goTo(1, 0);
                        ShiftRegisterLCD::write(WiFi.softAPIP().toString());
                        break;
                    case 2:
                        /*page 3: Connected WifiName and IP */
                        ShiftRegisterLCD::clearDisplay();

                        if (WiFi.isConnected()) {
                            ShiftRegisterLCD::goTo(0, 0);
                            ShiftRegisterLCD::write(WiFi.SSID().substring(0, 15));
                            ShiftRegisterLCD::goTo(1, 0);
                            ShiftRegisterLCD::write(WiFi.localIP().toString());
                        } else {
                            ShiftRegisterLCD::goTo(0, 0);
                            ShiftRegisterLCD::write("WIFI STATUS");
                            ShiftRegisterLCD::goTo(1, 0);
                            ShiftRegisterLCD::write("DISCONNECTED");
                        }
                        break;

                    case 3:
                        /*page 4: MQTT STATUS */
                        ShiftRegisterLCD::clearDisplay();
                        ShiftRegisterLCD::goTo(0, 0);

                        ShiftRegisterLCD::write("MQTT STATUS");
                        ShiftRegisterLCD::goTo(1, 0);
                        ShiftRegisterLCD::write(PerfectLightsCDController::mqttConnected() ?
                                                "CONNECTED" : "DISCONNECTED");
                        break;
                    default:
                        /*page 1: info screen */
                        menuIndex = 0;
                        ShiftRegisterLCD::clearDisplay();
                        ShiftRegisterLCD::drawTemplate();
                        LedController::updateLCD(true);
                        AnalogReader::updateLCD(true);
                        break;

                }

                menuLastPageChangeTime = millis();

                if (menuIndex == 0) {
                    // allow writing to the lcd screen only on the first page
                    LCD_DISABLE_WRITE = false;
                } else LCD_DISABLE_WRITE = true;
            }
        } else {
            if (menuIndex > 0 && (millis() - menuLastPageChangeTime > 5000)) {
                /*page 1: info screen */
                LCD_DISABLE_WRITE = false;
                menuIndex = 0;
                ShiftRegisterLCD::clearDisplay();
                ShiftRegisterLCD::drawTemplate();
                LedController::updateLCD(true);
                AnalogReader::updateLCD(true);
            }
            buttonState = false;
        }
        _t2 = millis();
    }


}
