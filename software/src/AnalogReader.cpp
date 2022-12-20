//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#include "AnalogReader.h"
#include "Async_Server.h"

int AnalogReader::pot1 = 0;
int AnalogReader::pot2 = 0;
int AnalogReader::ntc = 0;
int AnalogReader::button = 1000;
bool AnalogReader::locked = false;
bool AnalogReader::updateLCD_NTC = false;
unsigned long AnalogReader::timer = millis();
unsigned long AnalogReader::ntcTimer = millis();
unsigned long AnalogReader::lockTimer = millis();
double AnalogReader::lastCalculatedNTC = 0;
char AnalogReader::displayBuffer[9]{};

void AnalogReader::loop() {
    static uint8_t state = 0;
#ifdef DEBUG_MODE
    Serial.println("AR_L");
#endif
    /*
     * NOTE: Calling analogRead() too frequently causes WiFi to stop working.
     * When WiFi is under operation, analogRead() result may be cached for at least 5ms between effective calls.
     * TODO so change this function to wait at least 5ms between reads
     */
    if (millis() - timer > ANALOG_READ_INTERVAL) {
        ++state;
        if (state == 1)
            pot1 = readSensor(0);
        else if (state == 2)
            pot2 = readSensor(1);
        else if (state == 3) {
            ntc = readSensor(2);
        } else if (state == 4) {
            button = readSensor(3);
            state = 0;
        }
        if (state == 0) {
#ifdef DEBUG_MODE
            char tempBuffer[35];
            sprintf(tempBuffer, "Pot1:%4d \tPot2:%4d \tNTC:%4d", pot1, pot2, ntc);
            Serial.println(tempBuffer);
#endif
            //        Async_Server::wsLog.textAll(tempBuffer);

#ifdef REVERSE_POT_1
            pot1 = abs(pot1 - REVERSE_POT_1);
#endif
#ifdef REVERSE_POT_2
            pot2 = abs(pot2 - REVERSE_POT_2);
#endif
            updateLed();
        }
        timer = millis();
    }

    updateLCD();
}

void AnalogReader::allPinsLow() {
#ifdef DEBUG_MODE
    Serial.println("AR_APL");
#endif
    digitalWrite(ANALOG_MULTIPLEXER_B, LOW);
    digitalWrite(ANALOG_MULTIPLEXER_A, LOW);
    digitalWrite(ANALOG_MULTIPLEXER_INH, LOW);
}

void AnalogReader::init() {
#ifdef DEBUG_MODE
    Serial.println("AR_I");
#endif
    pinMode(A0, INPUT);
    pinMode(ANALOG_MULTIPLEXER_B, OUTPUT);
    pinMode(ANALOG_MULTIPLEXER_A, OUTPUT);
    pinMode(ANALOG_MULTIPLEXER_INH, OUTPUT);
}

int AnalogReader::readSensor(uint8_t switchNumber) {
#ifdef DEBUG_MODE
    Serial.println("AR_RS");
#endif
    digitalWrite(ANALOG_MULTIPLEXER_INH, HIGH);
    digitalWrite(ANALOG_MULTIPLEXER_A, (switchNumber & 1));
    digitalWrite(ANALOG_MULTIPLEXER_B, (switchNumber & 2));
    digitalWrite(ANALOG_MULTIPLEXER_INH, LOW);
    int t = analogRead(ANALOG_INPUT);
    return t;
}

[[maybe_unused]] [[nodiscard]] int AnalogReader::calculatePercent(int x) {
#ifdef DEBUG_MODE
    Serial.println("AR_CP");
#endif
    x = map(x, 50, 900, 0, 100);
    x = min(x, 100);
    x = max(x, 0);
    return x;
}

void AnalogReader::updateLed() {
#ifdef DEBUG_MODE
    Serial.println("AR_UL");
#endif
    static const int size = 10;
    static int lastPot1 = 0, lastPot2 = 0;
    static int lastValuesP1[size]{0};
    static int lastValuesP2[size]{0};
    static int i = 0;
    int p1 = 0, p2 = 0;
    lastValuesP1[i % size] = pot1;
    lastValuesP2[i % size] = pot2;
    ++i;
    for (int j = 0; j < size; ++j) {
        p1 += lastValuesP1[j];
        p2 += lastValuesP2[j];
    }
    p1 = (int) round(double(p1) / size);
    p2 = (int) round(double(p2) / size);

#ifdef ANALOG_READER_DEBUG

    //    Serial.printf("%d,%d\n", convTo8Bit(lastPot1), convTo8Bit(p1));
#endif
#ifdef DISABLE_POT_CONTROL
    return;
#endif
    if (millis() - lockTimer > POT_LOCK_TIME)
        locked = true;

    if (abs(lastPot1 - p1) > POT_UNLOCK_TRIGGER || abs(lastPot2 - p2) > POT_UNLOCK_TRIGGER) {
        locked = false;
        lockTimer = millis();
    }

    if (!locked && (abs(lastPot1 - p1) > POT_UPDATE_TRIGGER || abs(lastPot2 - p2) > POT_UPDATE_TRIGGER)) {
        lastPot1 = p1;
        lastPot2 = p2;
        LedController::updateLed(getColdLedValue(lastPot1), getWarmLedValue(lastPot1), convTo8Bit(lastPot2));
#ifdef ANALOG_READER_DEBUG

        //        Serial.printf("%4dP1     %4dP2\n", lastPot1, lastPot2);
#endif
    }
}

int AnalogReader::getColdLedValue(int x) {
#ifdef DEBUG_MODE
    Serial.println("AR_GCLV");
#endif
    x = map(x, 50, 475, 0, 255);
    x = min(x, 255);
    x = max(x, 0);
    return x;
}

int AnalogReader::getWarmLedValue(int x) {
#ifdef DEBUG_MODE
    Serial.println("AR_GWLV");
#endif
    x = map(x, 900, 475, 0, 255);
    x = min(x, 255);
    x = max(x, 0);
    return x;
}

uint8_t AnalogReader::convTo8Bit(int x) {
#ifdef DEBUG_MODE
    Serial.println("AR_CT8B");
#endif
    x = map(x, 50, 900, 0, 255);
    x = min(x, 255);
    x = max(x, 0);
    return x;
}

double AnalogReader::calculateNTC() {
#ifdef DEBUG_MODE
    Serial.println("AR_CNTC");
#endif
    double x = (ntc * 3.3) / 1024;
    x += 0.18;                     // fix tolerance//todo update stein-hart equation and remove this line
    x = RES_OHM * ((3.3 / x) - 1); // calculating resistance
    x = log(x / RES_OHM);
    x = (1.0f / (val_A + val_B * x + val_C * x * x + val_D * x * x * x)); // stein-hart equation
    x -= 273.15;                                                          // convert to celsius
    if (lastCalculatedNTC != x) {
        updateLCD_NTC = true;
        lastCalculatedNTC = x;
    }
    return x;
}

void AnalogReader::lock() {
#ifdef DEBUG_MODE
    Serial.println("AR_LCK");
#endif
    locked = true;
}

void AnalogReader::updateLCD(bool ignoreTimer) {
    if (ignoreTimer || (updateLCD_NTC && millis() - ntcTimer > NTC_DISPLAY_UPDATE_INTERVAL)) {
        sprintf(displayBuffer, "%3d", (int) round(lastCalculatedNTC));
        ShiftRegisterLCD::goTo(0, 10);
        ShiftRegisterLCD::write(displayBuffer);
        updateLCD_NTC = false;
        ntcTimer = millis();
    }
}
