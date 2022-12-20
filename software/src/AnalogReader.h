//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef LED_PANEL_ANALOG_READER_H
#define LED_PANEL_ANALOG_READER_H

#include <Settings.h>
#include <Arduino.h>
#include <LedController.h>
#include <ShiftRegisterLCD.h>

class AnalogReader {
private:
    static bool locked;
    static int pot1, pot2, ntc, button;
    static unsigned long timer, lockTimer, ntcTimer;
    static double lastCalculatedNTC;
    static bool updateLCD_NTC;
    static char displayBuffer[9];


    static void allPinsLow();

    static int readSensor(uint8_t);

    static int getColdLedValue(int);

    static int getWarmLedValue(int);


    [[nodiscard]]static uint8_t convTo8Bit(int);

public:


    [[maybe_unused]] [[nodiscard]]static int getPot1() { return pot1; }

    [[maybe_unused]] [[nodiscard]]static int getPot2() { return pot2; }

    [[maybe_unused]] [[nodiscard]]static int getNTC() { return ntc; }

    [[nodiscard]]static double calculateNTC();

    static void updateLed();

    [[maybe_unused]] [[nodiscard]]static int calculatePercent(int);

    static void init();

    static void loop();

    static void lock();

    static void updateLCD(bool ignoreTimer = false);

    /* return 1 if the button pressed, otherwise 0*/
    static bool getButton() {
        return button < 500;
    }

};


#endif //LED_PANEL_ANALOG_READER_H
