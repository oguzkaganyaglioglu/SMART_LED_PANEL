//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#ifndef LedController_h
#define LedController_h

#include <Settings.h>
#include <Arduino.h>
#include <ShiftRegisterLCD.h>
#include <PerfectLightsCDController.h>

#define TRANSITION_BUFFER_SIZE 10
#define TRANSITION_TIME_DIFF_MS 100

class LedController {


private:
    static uint8_t brightnessTarget;
    static uint8_t warmLedValTarget;
    static uint8_t coldLedValTarget;

    static uint8_t brightnessTransBuffer[TRANSITION_BUFFER_SIZE];
    static uint8_t warmLedValTransBuffer[TRANSITION_BUFFER_SIZE];
    static uint8_t coldLedValTransBuffer[TRANSITION_BUFFER_SIZE];

    static uint8_t brightness;
    static uint8_t warmLedVal;
    static uint8_t coldLedVal;
    static unsigned long displayTimer;
    static bool updateCold_LCD, updateWarm_LCD, updateBrightness_LCD;

    static bool updateCold, updateWarm, updateBrightness;

    static char displayBuffer[6];

    static void writeCOLD_LED();

    static void writeWARM_LED();

    static void writeLED();

    static uint8_t applyBrightness(uint8_t);

    static double calculatePosition(bool returnTarget = false);

    static void displayUpdateKelvin();

    static void displayUpdateCWB(uint8_t value, uint8_t col);

    static void displayUpdate(int value, uint8_t row, uint8_t col, const char *format);

public:

    static int getKelvinVal(bool returnTarget = false);

    static void setKelvinVal(int kelvin);

    static uint8_t getBrightness();

    static void updateLed(uint8_t cold, uint8_t warm, uint8_t brightness);

    static uint8_t getWarmLedVal();

    static uint8_t getColdLedVal();

    void static setBrightness(uint8_t);

    static void setWarmLedVal(uint8_t);

    static void setColdLedVal(uint8_t);

    static void init();

    static void loop();

    static void setBrightnessWTransition(bool newVal, uint8_t val);

    static void setWarmLedWTransition(bool newVal, uint8_t val);

    static void setColdLedWTransition(bool newVal, uint8_t val);

    static uint8_t getBrightnessTarget();

    static void setBrightnessTarget(uint8_t brightnessTarget);

    static uint8_t getWarmLedValTarget();

    static void setWarmLedValTarget(uint8_t warmLedValTarget);

    static uint8_t getColdLedValTarget();

    static void setColdLedValTarget(uint8_t coldLedValTarget);

    static void updateLCD(bool ignoreTimer = false);
};

#endif