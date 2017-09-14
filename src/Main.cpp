/*
 *  Copyright (c) 2017 Svarmo
 */

// 0x3C

#include "Arduino.h"
#include <Wire.h>

#define BAUD_RATE 9600

byte getOledScreenAddress() {
    byte address = 0x00;
    for(address = 1; address < 127; address++) {
        Wire.beginTransmission(address);
        byte error = Wire.endTransmission();

        if (error == 0) {
            Serial.print("Screen address 0x");
            Serial.println(address, HEX);
            return address;
        }
    }

    Serial.println("No address found");
    return 0x00;
}

void setup() {
    Serial.begin (BAUD_RATE);
    Wire.begin();
    byte screenAddress = getOledScreenAddress();

    Serial.println("Started");
}


void loop() {}
