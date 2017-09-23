/*
 *  Copyright (c) 2017 Svarmo
 */

// 0x3C

#include "Arduino.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BAUD_RATE 9600
#define OLED_RESET 4
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

Adafruit_SSD1306 display;
static const unsigned char PROGMEM logo16_glcd_bmp[] =
{ B00000000, B11000000,
  B00000001, B11000000,
  B00000001, B11000000,
  B00000011, B11100000,
  B11110011, B11100000,
  B11111110, B11111000,
  B01111110, B11111111,
  B00110011, B10011111,
  B00011111, B11111100,
  B00001101, B01110000,
  B00011011, B10100000,
  B00111111, B11100000,
  B00111111, B11110000,
  B01111100, B11110000,
  B01110000, B01110000,
  B00000000, B00110000 };

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// byte getOledScreenAddress() {
//     byte address = 0x00;
//     for(address = 1; address < 127; address++) {
//         Serial.print("*");
//         Wire.beginTransmission(address);
//         byte error = Wire.endTransmission();
//
//         if (error == 0) {
//             Serial.print("Screen address 0x");
//             Serial.println(address, HEX);
//             return address;
//         }
//     }
//
//     Serial.println("No address found");
//     return 0x00;
// }

void setupOledScreen() {
    Serial.println("Screen sequence");
    display.begin(0x3C);

    display.display();
    delay(2000);

    // Clear the buffer.
    display.clearDisplay();

    // draw a white circle, 10 pixel radius
    display.fillCircle(display.width()/2, display.height()/2, 10, WHITE);
    display.display();
    delay(2000);
    display.clearDisplay();

    // text display tests
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Hello, world!");
    display.setTextColor(BLACK, WHITE); // 'inverted' text
    display.println(3.141592);
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.print("0x"); display.println(0xDEADBEEF, HEX);
    display.display();
    delay(2000);
    display.clearDisplay();
}

void setup() {
    Serial.begin (BAUD_RATE);
    Serial.println("Starting");
    //Wire.begin();
    setupOledScreen();

    Serial.println("Started");
}


void loop() {}
