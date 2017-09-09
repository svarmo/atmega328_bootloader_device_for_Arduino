/*
 *  Copyright (c) 2017 Svarmo
 */
#include "Programmer.h"
#include "Constants.h"

void initPins() {
    // set up 8 MHz timer on pin 9
    pinMode(CLOCKOUT, OUTPUT);
    // set up Timer 1
    TCCR1A = bit (COM1A0);              // toggle OC1A on Compare Match
    TCCR1B = bit (WGM12) | bit (CS10);  // CTC, no prescaling
    OCR1A =  0;                         // output every cycle
}

byte program(const byte b1, const byte b2, const byte b3, const byte b4) {
    noInterrupts();

    SPI.transfer(b1);
    SPI.transfer(b2);
    SPI.transfer(b3);
    byte b = SPI.transfer(b4);

    interrupts();
    return b;
}

bool startProgramming() {
    Serial.print(F("Attempting to enter ICSP programming mode ..."));

    pinMode(RESET, OUTPUT);
    digitalWrite(RESET, HIGH);  // ensure SS stays high for now
    SPI.begin();
    SPI.setClockDivider (SPI_CLOCK_DIV64);
    pinMode (SCK, OUTPUT);

    unsigned int timeout = 0;
    byte confirm;
    do {
        delay(100);     // regrouping pause
        noInterrupts(); // ensure SCK low
        digitalWrite(SCK, LOW);

        // then pulse reset, see page 309 of datasheet
        digitalWrite(RESET, HIGH);
        delayMicroseconds(10);  // pulse for at least 2 clock cycles
        digitalWrite(RESET, LOW);
        interrupts();
        delay(25);  // wait at least 20 mS
        noInterrupts();

        SPI.transfer(progamEnable);
        SPI.transfer(programAcknowledge);
        confirm = SPI.transfer(0);
        SPI.transfer(0);
        interrupts();

        if (confirm != programAcknowledge) {
            Serial.print (".");
            if (timeout++ >= ENTER_PROGRAMMING_ATTEMPTS) {
                Serial.println(F("\nFailed to enter programming mode. Double-check wiring!"));
                return false;
            }
        }
    } while (confirm != programAcknowledge);
    Serial.println(F("ok"));
    return true;
}

void stopProgramming() {
    digitalWrite(RESET, LOW);
    // pinMode(RESET, INPUT);

    SPI.end();
    // turn off pull-ups, if any
    digitalWrite(SCK, LOW);
    digitalWrite(MOSI, LOW);
    digitalWrite(MISO, LOW);

    // set everything back to inputs
    pinMode(SCK, INPUT);
    pinMode(MOSI, INPUT);
    pinMode(MISO, INPUT);
    Serial.println (F("Programming mode off."));
}
