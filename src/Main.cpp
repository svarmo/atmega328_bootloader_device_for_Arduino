/*
 *  Copyright (c) 2017 Svarmo
 */

#include "Arduino.h"
#include <avr/pgmspace.h>
#include <SPI.h>

#include "Constants.h"
#include "SerialUtility.h"
#include "Programmer.h"

#define BAUD_RATE 9600
#define SELECT_BUTTON_PIN 2

// bootloaderType currentBootloader;

void printFuseBytes() {
    byte lowFuseByte = readFuse(LOW_FUSE);
    byte highFuseByte = readFuse(HIGH_FUSE);
    byte extFuseByte = readFuse(EXT_FUSE);
    byte lockFuseByte = readFuse(LOCK_FUSE);
    byte calibrationFuseByte = readFuse(CALIBRATION_FUSE);

    Serial.println(F("\n*CURRENT FUSE VALUES*"));
    Serial.print(F("Low_Fuse = "));
    Serial.println(lowFuseByte, HEX);
    Serial.print(F("High_fuse = "));
    Serial.println(highFuseByte, HEX);
    Serial.print(F("Ext_Fuse = "));
    Serial.println(extFuseByte, HEX);
    Serial.print(F("Lock_Fuse = "));
    Serial.println(lockFuseByte, HEX);
    Serial.print(F("Clock_Calibration_Fuse = "));
    Serial.println(calibrationFuseByte, HEX);
    Serial.println();
}

// burn the bootloader to the target device
void writeBootloader() {
    byte lFuse = readFuse(LOW_FUSE);
    byte newlFuse = atMega328Bootloader.lowFuse;
    byte newhFuse = atMega328Bootloader.highFuse;
    byte newextFuse = atMega328Bootloader.extFuse;
    byte newlockByte = atMega328Bootloader.lockByte;

    unsigned long addr = atMega328Bootloader.loaderStart;
    unsigned int  len = atMega328Bootloader.loaderLength;
    unsigned long pagesize = signature.pageSize;
    unsigned long pagemask = ~(pagesize - 1);

    unsigned long oldPage = addr & pagemask;

    // Current Low Fuse does not match the bootloader's lowFuse setting, re-write it
    if (lFuse != newlFuse) {
        if ((lFuse & 0x80) == 0) {
            Serial.println(F("Clearing 'Divide clock by 8' fuse bit."));
        }

        Serial.print(F("Fixing low fuse setting ..."));
        // writeFuse (newlFuse, LOW_FUSE);
        delay (1000);
        stopProgramming ();  // latch fuse
        if (!startProgramming ()) {
            return;
        }
        delay (1000);
    }

    Serial.println(F("\n*NEW FUSE VALUES*"));
    Serial.print(F("Low_Fuse = "));
    Serial.println(newlFuse, HEX);
    Serial.print(F("High_Fuse = "));
    Serial.println(newhFuse, HEX);
    Serial.print(F("Ext_Fuse = "));
    Serial.println(newextFuse, HEX);
    Serial.print(F("Lock_Fuse = "));
    Serial.println(newlockByte);

    Serial.println(F("Erasing chip ..."));
    eraseMemory();
    Serial.println(F("Writing bootloader ..."));
    for (int i = 0; i < len; i += 2) {
        unsigned long thisPage = (addr + i) & pagemask;
        // page changed? commit old one
        if (thisPage != oldPage) {
            commitPage (oldPage, true);
            oldPage = thisPage;
        }
        writeFlash(addr + i, pgm_read_byte(atMega328Bootloader.bootloader));
        writeFlash(addr + i + 1, pgm_read_byte(atMega328Bootloader.bootloader + i));
    }  // end while doing each word

    // commit final page
    commitPage(oldPage, true);
    Serial.println(F("Written."));
    // }  // end if programming

    Serial.println(F("Verifying ..."));

    // count errors
    unsigned int errors = 0;
    // check each byte
    for (int i = 0; i < len; i++) {
        byte found = readFlash (addr + i);
        byte expected = pgm_read_byte(atMega328Bootloader.bootloader + i);
        if (found != expected) {
            errors++;
        }  // end if error
    }  // end of for

    if (errors == 0) {
        Serial.println(F("No errors found."));
    } else {
        Serial.print(errors, DEC);
        Serial.println(F(" verification error(s)."));
        return;  // don't change fuses if errors
    }  // end if

    Serial.println(F("\nWriting fuses ..."));

    writeFuse(newlFuse, LOW_FUSE);
    writeFuse(newhFuse, HIGH_FUSE);
    writeFuse(newextFuse, EXT_FUSE);
    writeFuse(newlockByte, LOCK_FUSE);

    // confirm them
    printFuseBytes();
    Serial.println(F("Done."));
}



// void bootloadingInterruptCallback() {
//     Serial.println("In Callback");
//     if (millis() - lastInterrupt >= 5000) {
//         lastInterrupt = millis();
//         executeBootloading = true;
//     }
// }

void programBootloader() {
    if (startProgramming()) {
        getSignature();
        printFuseBytes();
        writeBootloader();
        stopProgramming();
    }
}

void setup() {
    Serial.begin (BAUD_RATE);
    Serial.println("Started");

    initPins();

    Serial.println("Waiting 2 seconds");
    delay(2000);
    programBootloader();
}

void loop() {}
