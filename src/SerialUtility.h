/*
 *  Copyright (c) 2017 Svarmo
 */

#ifndef SerialUtility_h
#define SerialUtility_h

#include "Arduino.h"
#include <SPI.h>

#define CLOCKOUT 9
#define RESET 10
#define SCK 13
#define ENTER_PROGRAMMING_ATTEMPTS 50

/* Functions */
void initPins();
byte program(const byte b1, const byte b2 = 0, const byte b3 = 0, const byte b4 = 0);
void stopProgramming();
bool startProgramming();

#endif
