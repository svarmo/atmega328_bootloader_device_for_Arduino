/*
 *  Copyright (c) 2017 Svarmo
 */

#ifndef Constant_h
#define Constant_h

#include "bootloader_lilypad328.h"

const unsigned long kb = 1024;
const byte NO_FUSE = 0xFF;

enum {
      LOW_FUSE,
      HIGH_FUSE,
      EXT_FUSE,
      LOCK_FUSE,
      CALIBRATION_FUSE
};

enum {
    progamEnable = 0xAC,

    // writes are preceded by progamEnable
    chipErase = 0x80,
    writeLockByte = 0xE0,
    writeLowFuseByte = 0xA0,
    writeHighFuseByte = 0xA8,
    writeExtendedFuseByte = 0xA4,
    //
    pollReady = 0xF0,

    programAcknowledge = 0x53,

    readSignatureByte = 0x30,
    readCalibrationByte = 0x38,
    readLowFuseByte = 0x50,       readLowFuseByteArg2 = 0x00,
    readExtendedFuseByte = 0x50,  readExtendedFuseByteArg2 = 0x08,
    readHighFuseByte = 0x58,      readHighFuseByteArg2 = 0x08,
    readLockByte = 0x58,          readLockByteArg2 = 0x00,
    //
    readProgramMemory = 0x20,
    writeProgramMemory = 0x4C,
    loadExtendedAddressByte = 0x4D,
    loadProgramMemory = 0x40
};  // end of enum

// structure to hold signature and other relevant data about each chip
typedef struct {
   byte sig [3];                // chip signature
   char desc [14];              // fixed array size keeps chip names in PROGMEM
   unsigned long flashSize;     // how big the flash is (bytes)
   unsigned int baseBootSize;   // base bootloader size (others are multiples of 2/4/8)
   unsigned long pageSize;      // flash programming page size (bytes)
   byte fuseWithBootloaderSize; // ie. one of: lowFuse, highFuse, extFuse
   bool timedWrites;            // true if pollUntilReady won't work by polling the chip
} signatureType;

const signatureType signature PROGMEM = {
    { 0x1E, 0x95, 0x0F }, // Signature
    "ATmega328P",         // Description
    32 * kb,              // Flash size
    512,                  // Bootloader size
    128,                  // Flash page size
    HIGH_FUSE,            // Fuse to change
    false                 // Timed writes
};

// structure to hold signature and other relevant data about each bootloader
typedef struct {
    byte sig [3];                // chip signature
    unsigned long loaderStart;   // start address of bootloader (bytes)
    const byte * bootloader;     // address of bootloader hex data
    unsigned int loaderLength;   // length of bootloader hex data (bytes)
    byte lowFuse, highFuse, extFuse, lockByte;  // what to set the fuses, lock bits to.
} bootloaderType;

const bootloaderType atMega328Bootloader PROGMEM = {
    { 0x1E, 0x95, 0x0F },                         // Signature
    0x7800,                                       // start address
    ATmegaBOOT_168_atmega328_pro_8MHz_hex,        // loader image
    sizeof ATmegaBOOT_168_atmega328_pro_8MHz_hex, // size of bootloader image
    0xE2,         // fuse low byte: internal 8 MHz oscillator, max start-up time
    0xDA,         // fuse high byte: SPI enable, boot into bootloader, 2048 byte bootloader
    0xFD,         // fuse extended byte: brown-out detection at 2.7V
    0xEF          // lock bits: SPM is not allowed to write to the Boot Loader section.
};

#endif
