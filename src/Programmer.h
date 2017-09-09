/*
 *  Copyright (c) 2017 Svarmo
 */

#ifndef Programmer_h
#define Programmer_h

#include "Arduino.h"
#include "SerialUtility.h"

// const byte fuseCommands [4] = { writeLowFuseByte, writeHighFuseByte, writeExtendedFuseByte, writeLockByte };

void clearPage();
void pollUntilReady();
byte readFuse (const byte which);
void writeFuse(const byte newValue, const byte whichFuse);
void eraseMemory();
void writeFlash(unsigned long addr, const byte data);
byte readFlash(unsigned long addr);
void readSignature(byte sig [3]);
void commitPage (unsigned long addr, bool showMessage);
void getSignature();

#endif
