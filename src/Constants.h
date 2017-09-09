/*
 *  Copyright (c) 2017 Svarmo
 */

#ifndef Constant_h
#define Constant_h

// number of items in an array
#define NUMITEMS(arg) ((unsigned int) (sizeof (arg) / sizeof (arg [0])))

// enum {
//       LOW_FUSE,
//       HIGH_FUSE,
//       EXT_FUSE,
//       LOCK_FUSE,
//       CALIBRATION_FUSE
// };

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

#endif
