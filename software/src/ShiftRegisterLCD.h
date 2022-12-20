//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef LED_PANEL_SHIFTREGISTERLCD_H
#define LED_PANEL_SHIFTREGISTERLCD_H

#include <Settings.h>
#include <Arduino.h>
#include <SPI.h>

class ShiftRegisterLCD {
private:
    static unsigned long timer, backupTimer;
    static unsigned long unlockTime;
    static bool lockScreen;
    static uint8_t cursorPos, backup[32];

    static bool *disableWrite;

    static void send8bit(uint8_t);

    static void sendCommand(uint8_t, bool);

    static void send4bit(uint8_t, bool);

    static void cursorShiftLeft();

    static void cursorShiftRight();


    static void setUnlockTimer(unsigned long t);

    static void drawBackup();

public:
    static void init();

    static void returnHome();

    static void setMutex(bool &mutex);

    static void write(const String &, bool = true);

    static void write(uint8_t, bool = true);

    static void clearDisplay(bool = true);

    static void goTo(uint8_t row, uint8_t col);

    static void drawTemplate();

    static void updateStart();

    static void updateDone(bool);

    static void loop();

};


#endif //LED_PANEL_SHIFTREGISTERLCD_H
