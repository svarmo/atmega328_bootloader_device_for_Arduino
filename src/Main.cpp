/*
 *  Copyright (c) 2017 Svarmo
 */

#include "Arduino.h"
#include <SPI.h>
#include <avr/pgmspace.h>

#include "Signatures.h"
#include "Constants.h"
#include "bootloader_lilypad328.h"
#include "bootloader_atmega328.h"
#include "Programmer.h"

#define BAUD_RATE 9600

// #define CLOCKOUT 9
// #define RESET 10
// #define SCK 13
#define SELECT_BUTTON_PIN 2

int foundSig = -1;
signatureType currentSignature;
byte lastAddressMSB = 0;

/******************************/
// ICSP Utils
/******************************/
// programming commands to send via SPI to the chip
// enum {
//     progamEnable = 0xAC,
//
//     // writes are preceded by progamEnable
//     chipErase = 0x80,
//     writeLockByte = 0xE0,
//     writeLowFuseByte = 0xA0,
//     writeHighFuseByte = 0xA8,
//     writeExtendedFuseByte = 0xA4,
//     //
//     pollReady = 0xF0,
//
//     programAcknowledge = 0x53,
//
//     readSignatureByte = 0x30,
//     readCalibrationByte = 0x38,
//     readLowFuseByte = 0x50,       readLowFuseByteArg2 = 0x00,
//     readExtendedFuseByte = 0x50,  readExtendedFuseByteArg2 = 0x08,
//     readHighFuseByte = 0x58,      readHighFuseByteArg2 = 0x08,
//     readLockByte = 0x58,          readLockByteArg2 = 0x00,
//     //
//     readProgramMemory = 0x20,
//     writeProgramMemory = 0x4C,
//     loadExtendedAddressByte = 0x4D,
//     loadProgramMemory = 0x40
// };  // end of enum

void clearPage(); // Declared in Programing_Utils

// byte program(const byte b1, const byte b2 = 0, const byte b3 = 0, const byte b4 = 0);
// byte program(const byte b1, const byte b2, const byte b3, const byte b4) {
//     noInterrupts();
//
//     SPI.transfer(b1);
//     SPI.transfer(b2);
//     SPI.transfer(b3);
//     byte b = SPI.transfer(b4);
//
//     interrupts();
//     return b;
// }

// poll the target device until it is ready to be programmed
void pollUntilReady() {
    if (currentSignature.timedWrites) {
        delay (10);  // at least 2 x WD_FLASH which is 4.5 mS
    } else {
        while ((program(pollReady) & 1) == 1) {}  // wait till ready
    }
}

byte readFuse (const byte which) {
    switch (which) {
        case LOW_FUSE:         return program(readLowFuseByte, readLowFuseByteArg2);
        case HIGH_FUSE:        return program(readHighFuseByte, readHighFuseByteArg2);
        case EXT_FUSE:         return program(readExtendedFuseByte, readExtendedFuseByteArg2);
        case LOCK_FUSE:        return program(readLockByte, readLockByteArg2);
        case CALIBRATION_FUSE: return program(readCalibrationByte);
    }
   return 0;
}

const byte fuseCommands [4] = { writeLowFuseByte, writeHighFuseByte, writeExtendedFuseByte, writeLockByte };
void writeFuse(const byte newValue, const byte whichFuse) {
    if (newValue == 0) {
        return;  // ignore
    }

    program(progamEnable, fuseCommands[whichFuse], 0, newValue);
    pollUntilReady();
}

// void stopProgramming() {
//     digitalWrite(RESET, LOW);
//     pinMode(RESET, INPUT);
//
//     SPI.end();
//     // turn off pull-ups, if any
//     digitalWrite(SCK, LOW);
//     digitalWrite(MOSI, LOW);
//     digitalWrite(MISO, LOW);
//
//     // set everything back to inputs
//     pinMode(SCK, INPUT);
//     pinMode(MOSI, INPUT);
//     pinMode(MISO, INPUT);
//     Serial.println (F("Programming mode off."));
// }

// bool startProgramming() {
//     Serial.print(F("Attempting to enter ICSP programming mode ..."));
//
//     pinMode(RESET, OUTPUT);
//     digitalWrite(RESET, HIGH);  // ensure SS stays high for now
//     SPI.begin();
//     SPI.setClockDivider (SPI_CLOCK_DIV64);
//     pinMode (SCK, OUTPUT);
//
//     unsigned int timeout = 0;
//     byte confirm;
//     do {
//         delay(100);     // regrouping pause
//         noInterrupts(); // ensure SCK low
//         digitalWrite(SCK, LOW);
//
//         // then pulse reset, see page 309 of datasheet
//         digitalWrite(RESET, HIGH);
//         delayMicroseconds(10);  // pulse for at least 2 clock cycles
//         digitalWrite(RESET, LOW);
//         interrupts();
//         delay(25);  // wait at least 20 mS
//         noInterrupts();
//
//         SPI.transfer(progamEnable);
//         SPI.transfer(programAcknowledge);
//         confirm = SPI.transfer(0);
//         SPI.transfer(0);
//         interrupts();
//
//         if (confirm != programAcknowledge) {
//             Serial.print (".");
//             if (timeout++ >= ENTER_PROGRAMMING_ATTEMPTS) {
//                 Serial.println(F("\nFailed to enter programming mode. Double-check wiring!"));
//                 return false;
//             }
//         }
//     } while (confirm != programAcknowledge);
//     Serial.println(F("ok"));
//     return true;
// }

void eraseMemory() {
    program(progamEnable, chipErase);  // erase it
    delay(20);
    pollUntilReady();
    clearPage();  // clear temporary page
}

void writeFlash(unsigned long addr, const byte data) {
    byte high = (addr & 1) ? 0x08 : 0;  // set if high byte wanted
    addr >>= 1;  // turn into word address
    program(loadProgramMemory | high, 0, lowByte (addr), data);
}

byte readFlash(unsigned long addr) {
    byte high = (addr & 1) ? 0x08 : 0;  // set if high byte wanted
    addr >>= 1;  // turn into word address

    // set the extended (most significant) address byte if necessary
    byte MSB = (addr >> 16) & 0xFF;
    if (MSB != lastAddressMSB) {
        program(loadExtendedAddressByte, 0, MSB);
        lastAddressMSB = MSB;
    }

    return program(readProgramMemory | high, highByte (addr), lowByte (addr));
}

void readSignature(byte sig [3]) {
    for (byte i = 0; i < 3; i++) {
        sig [i] = program(readSignatureByte, 0, i);
    }

    // make sure extended address is zero to match lastAddressMSB variable
    program(loadExtendedAddressByte, 0, 0);
    lastAddressMSB = 0;
}

// void initPins() {
//     // set up 8 MHz timer on pin 9
//     pinMode (CLOCKOUT, OUTPUT);
//     // set up Timer 1
//     TCCR1A = bit (COM1A0);              // toggle OC1A on Compare Match
//     TCCR1B = bit (WGM12) | bit (CS10);  // CTC, no prescaling
//     OCR1A =  0;                         // output every cycle
// }

// commit page to flash memory
void commitPage (unsigned long addr, bool showMessage) {
    // if (showMessage) {
    //     Serial.print (F("Committing page starting at 0x"));
    //     Serial.println (addr, HEX);
    // }
    // else {
    //     // showProgress ();
    // }

    addr >>= 1;  // turn into word address

    // set the extended (most significant) address byte if necessary
    byte MSB = (addr >> 16) & 0xFF;
    if (MSB != lastAddressMSB) {
        program (loadExtendedAddressByte, 0, MSB);
        lastAddressMSB = MSB;
    }

    program(writeProgramMemory, highByte (addr), lowByte (addr));
    pollUntilReady();

    clearPage();  // clear ready for next page full
}


/******************************/
// Programing Utils stuff
/******************************/
// clear entire temporary page to 0xFF in case we don't write to all of it
void clearPage() {
    unsigned int len = currentSignature.pageSize;
    for (unsigned int i = 0; i < len; i++) {
        writeFlash(i, 0xFF);
    }
}  // end of clearPage

/************************/


// structure to hold signature and other relevant data about each bootloader
typedef struct {
    byte sig [3];                // chip signature
    unsigned long loaderStart;   // start address of bootloader (bytes)
    const byte * bootloader;     // address of bootloader hex data
    unsigned int loaderLength;   // length of bootloader hex data (bytes)
    byte lowFuse, highFuse, extFuse, lockByte;  // what to set the fuses, lock bits to.
} bootloaderType;

// see Atmega328 datasheet page 298
const bootloaderType bootloaders [] PROGMEM = {
    // Only known bootloaders are in this array.
    // If we support it at all, it will have a start address.
    // If not compiled into this particular version the bootloader address will be zero.

    // ATmega328P
    {
        { 0x1E, 0x95, 0x0F },
        0x7E00,               // start address
        atmega328_optiboot,   // loader image
        sizeof atmega328_optiboot,
        0xFF,         // fuse low byte: external clock, max start-up time
        0xDE,         // fuse high byte: SPI enable, boot into bootloader, 512 byte bootloader
        0x05,         // fuse extended byte: brown-out detection at 2.7V
        0x2F          // lock bits: SPM is not allowed to write to the Boot Loader section.
    },

};

bootloaderType currentBootloader;

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
    bool foundBootloader = false;

    for (int j = 0; j < NUMITEMS (bootloaders); j++) {
        memcpy_P(&currentBootloader, &bootloaders [j], sizeof currentBootloader);

        if (memcmp(currentSignature.sig, currentBootloader.sig, sizeof currentSignature.sig) == 0) {
            foundBootloader = true;
            break;
        }  // end of signature found
    }  // end of for each signature

    if (!foundBootloader) {
        Serial.println(F("No bootloader support for this device."));
        return;
    }

    // if in the table, but with zero length, we need to enable a #define to use it.
    if (currentBootloader.loaderLength == 0) {
        Serial.println(F("Bootloader for this device is disabled, edit " __FILE__ " to enable it."));
        return;
    }

    byte lFuse = readFuse(LOW_FUSE);
    byte newlFuse = currentBootloader.lowFuse;
    byte newhFuse = currentBootloader.highFuse;
    byte newextFuse = currentBootloader.extFuse;
    byte newlockByte = currentBootloader.lockByte;

    unsigned long addr = currentBootloader.loaderStart;
    unsigned int  len = currentBootloader.loaderLength;
    unsigned long pagesize = currentSignature.pageSize;
    unsigned long pagemask = ~(pagesize - 1);
    const byte * bootloader = currentBootloader.bootloader;

    // Atmega328P or Atmega328
    if (currentBootloader.sig[0] == 0x1E && currentBootloader.sig[1] == 0x95
        && (currentBootloader.sig[2] == 0x0F || currentBootloader.sig[2] == 0x14)) {

        Serial.println(F("Using Lilypad 8 MHz loader."));
        bootloader = ATmegaBOOT_168_atmega328_pro_8MHz_hex;
        newlFuse = 0xE2;  // internal 8 MHz oscillator
        newhFuse = 0xDA;  //  2048 byte bootloader, SPI enabled
        addr = 0x7800;
        len = sizeof ATmegaBOOT_168_atmega328_pro_8MHz_hex;
    }  // end of being Atmega328P

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


    Serial.print(F("\nBootloader address = "));
    Serial.println(addr, HEX);
    Serial.print(F("Bootloader length = "));
    Serial.println(len);

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
        writeFlash(addr + i, pgm_read_byte(bootloader + i));
        writeFlash(addr + i + 1, pgm_read_byte(bootloader + i + 1));
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
        byte expected = pgm_read_byte(bootloader + i);
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
} // end of writeBootloader

void getSignature() {
    // TODO: Just make sure it matches the expected signature
    Serial.println(F("\nGetting signature"));
    byte sig[3];
    readSignature(sig);
    foundSig = -1;
    for (int j = 0; j < NUMITEMS (signatures); j++) {
        memcpy_P(&currentSignature, &signatures[j], sizeof currentSignature);

        if (memcmp(sig, currentSignature.sig, sizeof sig) == 0) {
            foundSig = j;
            Serial.print(F("Processor = "));
            Serial.println(currentSignature.desc);
            return;
        }
    }
    Serial.println(F("Unrecogized signature."));
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

        // if we found a signature try to write a bootloader
        if (foundSig != -1) {
            writeBootloader();
        }
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
