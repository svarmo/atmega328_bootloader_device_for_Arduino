#BOARD_TAG = micro
#BOARD_TAG = uno
BOARD_TAG = pro
BOARD_SUB = 8MHzatmega328

ARDUINO_LIBS = Wire SPI Adafruit_GFX Adafruit_SSD1306
ARDUINO_VERSION = 166
ARDUINO_DIR = ../../env/Arduino
ARDMK_DIR = ../..//env/ArduinoMake
USER_LIB_PATH = ../../env/lib
include $(ARDMK_DIR)/Arduino.mk
