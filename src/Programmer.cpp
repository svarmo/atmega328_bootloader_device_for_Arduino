
/*
 *  Copyright (c) 2017 Svarmo
 */
#include "Programmer.h"
#include "Constants.h"

int foundSig = -1;
signatureType currentSignature;
byte lastAddressMSB = 0;

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

// commit page to flash memory
void commitPage (unsigned long addr, bool showMessage) {
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

// clear entire temporary page to 0xFF in case we don't write to all of it
void clearPage() {
    unsigned int len = currentSignature.pageSize;
    for (unsigned int i = 0; i < len; i++) {
        writeFlash(i, 0xFF);
    }
}  // end of clearPage

void getSignature() {
    // TODO: Just make sure it matches the expected signature
    // Serial.println(F("\nGetting signature"));
    // byte sig[3];
    // readSignature(sig);
    // // foundSig = -1;
    // // for (int j = 0; j < NUMITEMS (signatures); j++) {
    // //     memcpy_P(&currentSignature, &signatures[j], sizeof currentSignature);
    // //
    // //     if (memcmp(sig, currentSignature.sig, sizeof sig) == 0) {
    // //         foundSig = j;
    // //         Serial.print(F("Processor = "));
    // //         Serial.println(currentSignature.desc);
    // //         return;
    // //     }
    // // }
    // Serial.println(F("Unrecogized signature."));
}
