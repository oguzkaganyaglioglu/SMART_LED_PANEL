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


bool buttonState = false;
bool LCD_DISABLE_WRITE = false;
uint8_t menuIndex = 0;


void setup() {
    /* set pwm frequency */
    analogWriteFreq(26000);

    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n");
    Serial.println("We are on!!!");
    Serial.printf("\nResetReason: %u\nResetExccause: %u\n", system_get_rst_info()->reason,
                  system_get_rst_info()->exccause);

    /* initialize Wi-Fi manager */
    WiFi_Manager::init();

    /* start DNS server */
    dnsServer.start(53, "*", WiFi.softAPIP());
    myServer.attachWsUpdater(&wsUpdater);

    Serial.print(AP_SSID);
    Serial.print(" : ");
    Serial.println(WiFi.softAPIP());

    /* initialize LCD */
    ShiftRegisterLCD::setMutex(LCD_DISABLE_WRITE);
    ShiftRegisterLCD::init();

    /* initialize server */
    myServer.init();

    /* initialize LED Controller */
    LedController::init();

    /* initialize Analog Reader */
    AnalogReader::init();

    /* initialize Perfect Lights Custom Smart Home Controller  */
    PerfectLightsCDController::init(myWifiClient);

    Serial.println("Initialization Completed!!!");
}

unsigned long t = millis();
unsigned long t2 = millis();
unsigned long menuLastPageChangeTime = millis();

void loop() {
    /* Process DNS requests */
    dnsServer.processNextRequest();

    /* Cleanup old clients & reboot if needed */
    myServer.loop();

    /* Send the latest values to the ws clients periodically */
    wsUpdater.loop();

    if (millis() - t > 10) {
        /*100Hz loop*/

        /* Read POTs, NTC and the button sensors */
        AnalogReader::loop();

        /* Write the latest values to the LEDs */
        LedController::loop();

        /* Refresh display periodically */
        ShiftRegisterLCD::loop();

        /* Process Perfect Light Custom Smart Home requests */
        PerfectLightsCDController::loop();
        t = millis();
    }

    if (millis() - t2 > 200) {
        /*5Hz loop*/

        /* check if the button pressed */
        if (AnalogReader::getButton()) {

            /* check if the button state changed */
            if (!buttonState) {
                /* change the menu page */
                buttonState = true;

                /* unlock the display */
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
                    /* allow writing to the lcd screen only on the first page */
                    LCD_DISABLE_WRITE = false;
                } else LCD_DISABLE_WRITE = true;
            }
        } else {
            /* open the first page of the menu if a page other than the first page of the menu
             * remains open for more than 5 seconds */
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
        t2 = millis();
    }


}
