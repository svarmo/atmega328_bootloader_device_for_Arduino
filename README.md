# Overview
The objective of this project is to create a fully functional Arduino Bootloader for the ATMega328 MCU.

This project will include all the sources to manufacture the programmer device, it includes the 3D printed model, the PCB manufacturing files as well as the source code for the device.

# License
This project is licenses under an MIT license, view attached file for more detail.

The MCU is works using Arduino and **HEAVILY** relies on code provided by Nick Gammon's Arduino_sketch code: https://github.com/nickgammon/arduino_sketches (Also MIT license)

All the hardware is designed by me and falls under **Open Hardware** spec so feel free to use, modify as you see fit.

## Special contributors
* Ricardo Rodriguez
* Nick Gammon

# Usage
TODO: Populate how to use

# Notes
At this point, a good write looks like this:

```
Started
Waiting 2 seconds
Attempting to enter ICSP programming mode ...ok

Getting signature
Processor = ATmega328P

*CURRENT FUSE VALUES*
Low_Fuse = E2
High_fuse = DA
Ext_Fuse = FD
Lock_Fuse = EF
Clock_Calibration_Fuse = 96

Using Lilypad 8 MHz loader.

Bootloader address = 7800
Bootloader length = 1932

*NEW FUSE VALUES*
Low_Fuse = E2
High_Fuse = DA
Ext_Fuse = 5
Lock_Fuse = 2F
Erasing chip ...
Writing bootloader ...
Written.
Verifying ...
No errors found.

Writing fuses ...

*CURRENT FUSE VALUES*
Low_Fuse = E2
High_fuse = DA
Ext_Fuse = FD
Lock_Fuse = EF
Clock_Calibration_Fuse = 96

Done.
Programming mode off.
```
