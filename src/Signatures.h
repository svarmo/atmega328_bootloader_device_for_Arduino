// Signatures.h
//
// Signatures and extra information for known chips
//
// Author: Nick Gammon

/* ----------------------------------------------------------------------------
NOTE: This file should only be modified in the Atmega_Hex_Uploader directory.
Copies in other directories are hard-linked to this one.
After modifying it run the shell script:
  fixup_links.sh
This script needs to be run in the directories:
  Atmega_Board_Programmer and Atmega_Board_Detector
That will ensure that those directories now are using the same file.
------------------------------------------------------------------------------ */


// copy of fuses/lock bytes found for this processor
byte fuses [5];

// meaning of positions in above array
enum {
      LOW_FUSE,
      HIGH_FUSE,
      EXT_FUSE,
      LOCK_FUSE,
      CALIBRATION_FUSE
};

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

const unsigned long kb = 1024;
const byte NO_FUSE = 0xFF;


// see Atmega datasheets
const signatureType signatures [] PROGMEM =
  {
//     signature        description   flash size   bootloader  flash  fuse     timed
//                                                     size    page    to      writes
//                                                             size   change
  // Atmega328 family
  { { 0x1E, 0x92, 0x0A }, "ATmega48PA",   4 * kb,         0,    64,  NO_FUSE,  false  },
  { { 0x1E, 0x93, 0x0F }, "ATmega88PA",   8 * kb,       256,   128,  EXT_FUSE,  false },
  { { 0x1E, 0x94, 0x0B }, "ATmega168PA", 16 * kb,       256,   128,  EXT_FUSE,  false },
  { { 0x1E, 0x95, 0x0F }, "ATmega328P",  32 * kb,       512,   128,  HIGH_FUSE, false },
  { { 0x1E, 0x95, 0x14 }, "ATmega328",   32 * kb,       512,   128,  HIGH_FUSE, false },

  };  // end of signatures
