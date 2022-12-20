//
// Created by Oguz Kagan YAGLIOGLU
// www.oguzkagan.xyz
//

#include "ShiftRegisterLCD.h"

#define MODE_4BIT_2_LINE B0010100
#define CLEAR_DISPLAY B00000001
#define RETURN_HOME B00000010
#define DISPLAY_ON_CURSOR_OFF B00001100
#define DISPLAY_ON_CURSOR_ON_BLINK B00001111
#define CURSOR_SHIFT_RIGHT B00010100
#define CURSOR_SHIFT_LEFT B00010000

uint8_t ShiftRegisterLCD::cursorPos = 0;
unsigned long ShiftRegisterLCD::timer = millis();
unsigned long ShiftRegisterLCD::backupTimer = millis();
unsigned long ShiftRegisterLCD::unlockTime = 0;
bool ShiftRegisterLCD::lockScreen = false;
bool *ShiftRegisterLCD::disableWrite = nullptr;
uint8_t ShiftRegisterLCD::backup[32]{0};

void ShiftRegisterLCD::send8bit(uint8_t d) {
    // D7654 E RW RS
    //  0000 0 0  0  0

#ifdef DEBUG_MODE
    Serial.println("SRLCD_S8B");
#endif

#ifdef DISABLE_DISPLAY
    return;
#endif
    EspClass::wdtFeed();
    if (lockScreen) return;
    if (disableWrite != nullptr && *disableWrite) return;

    digitalWrite(ST_CP, LOW);
//    shiftOut(DS, SH_CP, MSBFIRST, d);
    SPI.transfer(d);
    digitalWrite(ST_CP, HIGH);
}

void ShiftRegisterLCD::sendCommand(uint8_t c, bool RS) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_SC");
#endif
    if (disableWrite != nullptr && *disableWrite) return;

    send4bit((c >> 0x4), RS);
    send4bit((c & 0xF), RS);
}

void ShiftRegisterLCD::send4bit(uint8_t x, bool RS) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_S4B");
#endif
    if (disableWrite != nullptr && *disableWrite) return;

    short t = RS ? B010 : 0;
    x = x << 3;

    send8bit(x | (B000 | t));
    delayMicroseconds(1);
    send8bit(x | (B100 | t));
    delayMicroseconds(1);
    send8bit(x | (B000 | t));
    delayMicroseconds(100);
}

void ShiftRegisterLCD::init() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_I");
#endif

    if (disableWrite != nullptr && *disableWrite) return;


    SPI.begin();
    SPI.beginTransaction(SPISettings(10000000, LSBFIRST, SPI_MODE0));
    pinMode(ST_CP, OUTPUT);
//    pinMode(SH_CP, OUTPUT);
//    pinMode(DS, OUTPUT);

    for (int i = 0; i < 3; i++) {
        sendCommand(MODE_4BIT_2_LINE, false);
    }

    for (int i = 0; i < 3; i++) {
        clearDisplay();
    }

    for (int i = 0; i < 3; i++) {
        returnHome();
    }
    for (int i = 0; i < 3; i++) {
        sendCommand(DISPLAY_ON_CURSOR_OFF, false);
    }
    clearDisplay();
    goTo(0, 0);
    write(" Perfect Lights ");
    goTo(1, 0);
    write("       by       ");
    delay(1000);
    goTo(0, 0);
    write("   Oguz Kagan   ");
    goTo(1, 0);
    write("   YAGLIOGLU    ");
    delay(1000);
    clearDisplay();
    drawTemplate();


}

void ShiftRegisterLCD::drawTemplate() {
    //| 9000K    100 C |
    //| 100c 100w 100b |

#ifdef DEBUG_MODE
    Serial.println("SRLCD_DT");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    goTo(0, 5);
    write('K');

    goTo(0, 13);
    write(B11011111);//Celsius sign
    write('C');

    goTo(1, 4);
    write('C');

    goTo(1, 9);
    write('W');

    goTo(1, 14);
    write('B');
}

void ShiftRegisterLCD::clearDisplay(bool clearBackup) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_CD");
#endif

    if (disableWrite != nullptr && *disableWrite) return;
    if (clearBackup) for (unsigned char &i: backup) i = ' ';
    sendCommand(CLEAR_DISPLAY, false);
    delayMicroseconds(2000);
    cursorPos = 0;
}

void ShiftRegisterLCD::write(uint8_t c, bool _backup) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_WRT");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    sendCommand(c, true);
    if (_backup && cursorPos <= 55) {
        if (cursorPos >= 40) backup[cursorPos - 24] = c;
        else backup[cursorPos] = c;
    }
    ++cursorPos;
    backupTimer = millis();
}

void ShiftRegisterLCD::write(const String &str, bool _backup) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_WRTS");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    for (char c: str) {
        write(c, _backup);
    }
}

void ShiftRegisterLCD::returnHome() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_RH");
#endif

    if (disableWrite != nullptr && *disableWrite) return;


    sendCommand(RETURN_HOME, false);
    delayMicroseconds(2000);
    cursorPos = 0;
}

void ShiftRegisterLCD::goTo(uint8_t row, uint8_t col) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_GT");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    if (row < 0 || row > 1) return;
    auto x = (int8_t) ((row * 40) + col - cursorPos);
    if (x == 0) return;
    if (x > 0) for (uint8_t i = 0; i < x; ++i) cursorShiftRight();
    else for (int8_t i = 0; i > x; --i) cursorShiftLeft();
}

void ShiftRegisterLCD::cursorShiftLeft() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_CSL");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    sendCommand(CURSOR_SHIFT_LEFT, false);
    --cursorPos;
}

void ShiftRegisterLCD::cursorShiftRight() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_CSR");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    sendCommand(CURSOR_SHIFT_RIGHT, false);
    ++cursorPos;
}

void ShiftRegisterLCD::updateStart() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_US");
#endif
    lockScreen = false;
    clearDisplay(false);
    goTo(0, 0);
    write("    UPDATING    ", false);
    goTo(1, 0);
    write("  PLEASE WAIT!  ", false);
    lockScreen = true;
    setUnlockTimer(0);
}

void ShiftRegisterLCD::updateDone(bool result) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_UD");
#endif
    lockScreen = false;
    clearDisplay(false);
    goTo(0, 0);
    write("     UPDATE     ", false);
    goTo(1, 0);
    if (result)
        write("   SUCCESSFUL   ", false);
    else
        write("     FAILED     ", false);
    lockScreen = true;
    setUnlockTimer(5000);
}

void ShiftRegisterLCD::setUnlockTimer(unsigned long t) {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_SUT");
#endif
    timer = millis();
    unlockTime = t;
}

void ShiftRegisterLCD::loop() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_L");
#endif
    if (unlockTime != 0 && millis() - timer > unlockTime) {
        lockScreen = false;
        unlockTime = 0;
        clearDisplay(false);
        drawBackup();
    }


#ifdef DISPLAY_REDRAW_INTERVAL
    if (millis() - backupTimer > DISPLAY_REDRAW_INTERVAL) {
        drawBackup();
        backupTimer = millis();
    }
#endif
}

void ShiftRegisterLCD::drawBackup() {
#ifdef DEBUG_MODE
    Serial.println("SRLCD_DB");
#endif

    if (disableWrite != nullptr && *disableWrite) return;

    goTo(0, 0);
    for (unsigned int i = 0; i < 16; ++i) {
        write(backup[i], false);
    }
    goTo(1, 0);
    for (unsigned int i = 16; i < 32; ++i) {
        write(backup[i], false);
    }
}

void ShiftRegisterLCD::setMutex(bool &mutex) {
    disableWrite = &mutex;
}


