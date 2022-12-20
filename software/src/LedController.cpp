//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#include <LedController.h>

uint8_t LedController::brightnessTransBuffer[TRANSITION_BUFFER_SIZE];
uint8_t LedController::warmLedValTransBuffer[TRANSITION_BUFFER_SIZE];
uint8_t LedController::coldLedValTransBuffer[TRANSITION_BUFFER_SIZE];

uint8_t LedController::brightnessTarget = 0; // 0 = 0%  |  255 = 100%
uint8_t LedController::warmLedValTarget = 0;
uint8_t LedController::coldLedValTarget = 0;

uint8_t LedController::brightness = 0; // 0 = 0%  |  255 = 100%
uint8_t LedController::warmLedVal = 255;
uint8_t LedController::coldLedVal = 255;
char LedController::displayBuffer[6]{};
bool LedController::updateCold{true};
bool LedController::updateWarm{true};
bool LedController::updateBrightness{true};
bool LedController::updateCold_LCD{true};
bool LedController::updateWarm_LCD{true};
bool LedController::updateBrightness_LCD{true};
unsigned long LedController::displayTimer{0};

void LedController::init() {
#ifdef DEBUG_MODE
    Serial.println("LC_I");
#endif
    pinMode(Cold_Led_Pin, OUTPUT);
    pinMode(Warm_Led_Pin, OUTPUT);
    writeLED();
}

void LedController::setBrightnessWTransition(bool newVal, uint8_t val) {
    static uint8_t currentVal = 0;
    static uint8_t idx = 0;
    static uint32_t lastCall = 0;
    uint16_t avg = 0;
    if (newVal) {
        currentVal = val;
        setBrightnessTarget(val);
    } else if (millis() - lastCall < TRANSITION_TIME_DIFF_MS) return;
    lastCall = TRANSITION_TIME_DIFF_MS;
    brightnessTransBuffer[idx] = currentVal;
    idx++;
    idx %= TRANSITION_BUFFER_SIZE;
    avg = 0;
    for (uint8_t i: brightnessTransBuffer) {
        avg += i;
    }
    avg /= TRANSITION_BUFFER_SIZE;
    if (avg == brightness) return;
    brightness = avg;
    updateBrightness = true;
}

void LedController::setColdLedWTransition(bool newVal, uint8_t val) {
    static uint8_t currentVal = 0;
    static uint8_t idx = 0;
    static uint32_t lastCall = 0;
    uint16_t avg = 0;
    if (newVal) {
        currentVal = val;
        setColdLedValTarget(val);
    } else if (millis() - lastCall < TRANSITION_TIME_DIFF_MS) return;
    lastCall = TRANSITION_TIME_DIFF_MS;
    coldLedValTransBuffer[idx] = currentVal;
    idx++;
    idx %= TRANSITION_BUFFER_SIZE;
    avg = 0;
    for (uint8_t i: coldLedValTransBuffer) {
        avg += i;
    }
    avg /= TRANSITION_BUFFER_SIZE;
    if (avg == coldLedVal) return;
    coldLedVal = avg;
    updateCold = true;
}

void LedController::setWarmLedWTransition(bool newVal, uint8_t val) {
    static uint8_t currentVal = 0;
    static uint8_t idx = 0;
    static uint32_t lastCall = 0;
    uint16_t avg = 0;
    if (newVal) {
        currentVal = val;
        setWarmLedValTarget(val);
    } else if (millis() - lastCall < TRANSITION_TIME_DIFF_MS) return;
    lastCall = TRANSITION_TIME_DIFF_MS;
    warmLedValTransBuffer[idx] = currentVal;
    idx++;
    idx %= TRANSITION_BUFFER_SIZE;
    avg = 0;
    for (uint8_t i: warmLedValTransBuffer) {
        avg += i;
    }
    avg /= TRANSITION_BUFFER_SIZE;
    if (avg == warmLedVal) return;
    warmLedVal = avg;
    updateWarm = true;
}


uint8_t LedController::getBrightness() {
#ifdef DEBUG_MODE
    Serial.println("LC_GB");
#endif
    return brightness;
}

uint8_t LedController::getWarmLedVal() {
#ifdef DEBUG_MODE
    Serial.println("LC_GWLV");
#endif
    return warmLedVal;
}

uint8_t LedController::getColdLedVal() {
#ifdef DEBUG_MODE
    Serial.println("LC_GCLV");
#endif
    return coldLedVal;
}

void LedController::setBrightness(uint8_t val) {
#ifdef DEBUG_MODE
    Serial.println("LC_SB");
#endif
    setBrightnessWTransition(true, val);
    PerfectLightsCDController::analogReaderSync();
//
//    if (brightness != val) {
//        updateBrightness = true;
//        brightness = val;
//    }
}

void LedController::setWarmLedVal(uint8_t val) {
#ifdef DEBUG_MODE
    Serial.println("LC_SWLV");
#endif
    setWarmLedWTransition(true, val);
//    if (warmLedVal != val) {
//        updateWarm = true;
//        warmLedVal = val;
//    }
}

void LedController::setColdLedVal(uint8_t val) {
#ifdef DEBUG_MODE
    Serial.println("LC_SCLV");
#endif
    setColdLedWTransition(true, val);
//    if (coldLedVal != val) {
//        updateCold = true;
//        coldLedVal = val;
//    }
}

uint8_t LedController::applyBrightness(uint8_t val) {
#ifdef DEBUG_MODE
    Serial.println("LC_AB");
#endif
    return (val * brightness) / 255;
}

void LedController::updateLed(uint8_t cold, uint8_t warm, uint8_t bright) {
#ifdef DEBUG_MODE
    Serial.println("LC_UL");
#endif

    setWarmLedVal(warm);
    setColdLedVal(cold);
    setBrightness(bright);
}

void LedController::writeCOLD_LED() {
#ifdef DEBUG_MODE
    Serial.println("LC_WCL");
#endif
    analogWrite(Cold_Led_Pin, applyBrightness(coldLedVal));
}

void LedController::writeWARM_LED() {
#ifdef DEBUG_MODE
    Serial.println("LC_WWL");
#endif
    analogWrite(Warm_Led_Pin, applyBrightness(warmLedVal));
}

void LedController::writeLED() {
#ifdef DEBUG_MODE
    Serial.println("LC_WL");
#endif
    writeCOLD_LED();
    writeWARM_LED();
}

void LedController::displayUpdateKelvin() {
#ifdef DEBUG_MODE
    Serial.println("LC_DUK");
#endif
    displayUpdate(getKelvinVal(), 0, 1, "%4d");
}

void LedController::displayUpdateCWB(uint8_t value, uint8_t col) {
#ifdef DEBUG_MODE
    Serial.println("LC_DUCWB");
#endif
    displayUpdate(map(value, 0, 255, 0, 100), 1, col, "%3d");
}

void LedController::displayUpdate(int value, uint8_t row, uint8_t col, const char *format) {
#ifdef DEBUG_MODE
    Serial.println("LC_DU");
#endif
    sprintf(displayBuffer, format, value);
    ShiftRegisterLCD::goTo(row, col);
    ShiftRegisterLCD::write(displayBuffer);
}

double LedController::calculatePosition(bool returnTarget) {
#ifdef DEBUG_MODE
    Serial.println("LC_CP");
#endif
    if (returnTarget) {
        if (coldLedValTarget < 255) return double(coldLedValTarget / 2.0);
        return double(((255.0 - warmLedValTarget) + 255.0) / 2.0);
    }

    if (coldLedVal < 255) return double(coldLedVal / 2.0);
    return double(((255.0 - warmLedVal) + 255.0) / 2.0);
}

void LedController::loop() {

#ifdef DEBUG_MODE
    Serial.println("LC_L");
#endif

    if (updateCold) {
        writeCOLD_LED();
        updateCold_LCD = true;
        updateCold = false;
    }
    if (updateWarm) {
        writeWARM_LED();
        updateWarm_LCD = true;
        updateWarm = false;
    }

    if (AnalogReader::calculateNTC() > HEAT_PROTECTION_MAX_TEMP) {
        setBrightness(0);
    }

    if (updateBrightness) {
        writeLED();
        updateBrightness_LCD = true;
        updateBrightness = false;
    }


    setBrightnessWTransition(false, 0);
    setWarmLedWTransition(false, 0);
    setColdLedWTransition(false, 0);

    updateLCD();

}

int LedController::getKelvinVal(bool returnTarget) {
    if (returnTarget) return int(((((calculatePosition(true) / 255.0) * (COLD_K - WARM_K)) + WARM_K) + 25) / 50) * 50;

    static int preVal = 0;
    static int newVal = 0;
    newVal = int(((((calculatePosition() / 255.0) * (COLD_K - WARM_K)) + WARM_K) + 25) / 50) * 50;
    if (newVal != preVal) {
        PerfectLightsCDController::analogReaderSync();
        preVal = newVal;
    }
    return newVal;
}

void LedController::setKelvinVal(int kelvin) {
    // separate cold and warm led values from given kelvin value
    int pos = int((510.0 * (kelvin - WARM_K)) / (COLD_K - WARM_K));
    // extract cold led value
    int temp = map(pos, 0, 255, 0, 255);
    temp = min(temp, 255);
    temp = max(temp, 0);
    setColdLedVal(temp);

    // extract warm led value
    temp = map(pos, 510, 255, 0, 255);
    temp = min(temp, 255);
    temp = max(temp, 0);
    setWarmLedVal(temp);
}

uint8_t LedController::getBrightnessTarget() {
    return brightnessTarget;
}

void LedController::setBrightnessTarget(uint8_t brightnessTarget) {
    LedController::brightnessTarget = brightnessTarget;
}


uint8_t LedController::getWarmLedValTarget() {
    return warmLedValTarget;
}

void LedController::setWarmLedValTarget(uint8_t warmLedValTarget) {
    LedController::warmLedValTarget = warmLedValTarget;
}

uint8_t LedController::getColdLedValTarget() {
    return coldLedValTarget;
}

void LedController::setColdLedValTarget(uint8_t coldLedValTarget) {
    LedController::coldLedValTarget = coldLedValTarget;
}

void LedController::updateLCD(bool ignoreTimer) {
    if (ignoreTimer || millis() - displayTimer > LED_DISPLAY_UPDATE_INTERVAL) {
        if (updateCold_LCD || ignoreTimer) {
            displayUpdateCWB(coldLedVal, 1);
        }

        if (updateWarm_LCD || ignoreTimer) {
            displayUpdateCWB(warmLedVal, 6);
        }

        if (updateBrightness_LCD || ignoreTimer) {
            displayUpdateCWB(brightness, 11);
            updateBrightness_LCD = false;
        }

        if (updateWarm_LCD || updateCold_LCD || ignoreTimer) {
            displayUpdateKelvin();
            updateWarm_LCD = false;
            updateCold_LCD = false;
        }
        displayTimer = millis();
    }
}
