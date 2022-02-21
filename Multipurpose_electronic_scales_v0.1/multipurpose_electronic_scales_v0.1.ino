// Multipurpose_Electronic_Scales
//
// Arduino_UNO_Rev3 ATmega328P controlled multipurpose electronic scale.
//
// This version of the code implements (not yet):
// - Calibrating and store the scale factor into the EEPROM
// - Automatically enter weighing mode after start-up
// - Setup menu by long pressing the Enter button
// - Buzzer
// - Advanced function to control other pins (main purpose of this project)
// 
// Power supply should be in the range of 7-12V according to the
// Arduino_UNO_Rev3 tech specs.
//
// For calibration you need a known weight load that close to the 
// maximun range of your load cell.
//
// Board: Arduino UNO Rev3
// Microcontroller: ATmega328P
// Clock speed: 16 MHz
//
// Contributions & Special thanks:
// The program structure refers to the following project:
// - Stefan Wagner, https://github.com/wagiminator/ATmega-Soldering-Station
// Contributers:
//
// 2022 by Ted Liu
// Project Files (Github): 
// 
// License: 
// - Creative Commons Attribution-ShareAlike 3.0 License (CC BY-SA 3.0)
// - https://creativecommons.org/licenses/by-sa/3.0/
// 
// Libraries
#include "HX711.h"                          // https://github.com/bogde/HX711
#include "LiquidCrystal_I2C.h"              // https://github.com/johnrickman/LiquidCrystal_I2C
#include "EEPROM.h"                         // https://github.com/PaulStoffregen/EEPROM

// Firmware version
#define  VERSION                "v0.1"

//Pins
#define  HX711_DT_PIN           "2"         // HX711 module Data IO Connection
#define  HX711_SCK_PIN          "3"         // HX711 module Serial Clock Input
#define  BTN_ENTER_PIN          "6"         // Enter button Input
#define  BTN_UP_PIN             "7"         // Up button Input
#define  BTN_DOWN_PIN           "8"         // Down button Input
#define  BTN_ESC_PIN            "9"         // ESC button Input
#define  BUZZER_PIN             "10"        // Buzzer

// Default values
#define  DEFAULT_SCALE_FACTOR   2080        // Default scale factor of DYLT-101 "S" Type load cell (20.80)
#define  GRAVITY_ACCELERATION    981        // Default gravitational acceleration (9.81)
#define  BEEP_ENABLE            true        // enable/disable buzzer

// Default measurement unit
#define  DEFAULT_UNIT           0003        // 0001 for kgf, 0002 for gf, 0003 for N, 0004 for kg, 0005 for g

// EEPROM identifier
#define  EEPROM_IDENT           0xE76B      // to identify if EEPROM was written by this program

//Default values that can be changed by the user and store in the EEPROM 
float    scaleFacter            = DEFAULT_SCALE_FACTOR / 100;
float    gravityAcceleration    = GRAVITY_ACCELERATION / 100;
char     units                  = DEFAULT_UNIT;
bool     beepEnable             = BEEP_ENABLE;

// Menu items
const char *MainScreenItems[]   = { "Measurement", "Setting", "Information", "Advance" };
const char *SureItems[]         = { "Are you sure ?", "No", "Yes" };
const char *SettingItems[]      = { "Calibration", "Gravity setting", "Set scale factor" };
const char *InformationItems[]  = { "Scale factor", "Gravity acceleration", "Firmware version" };
const char *StoreItems[]        = { "Store Settings ?", "No", "Yes" };
const char *UnitsItems[]        = { "kgf", "gf", "N", "kg", "g" };
const char *AdvanceItems[]      = { "Comming Soon" };

// Timing variables

// Create hx711 object
HX711 hx711;

// Create lcd object
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  // set the buttons and buzzer pin modes
  pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
  pinMode(BTN_UP_PIN, INPUT_PULLUP);
  pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
  pinMode(BTN_ESC_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // initialize the HX711 
  hx711.begin(HX711_DT_PIN, HX711_SCK_PIN);

  // initialize the lcd
  lcd.init();     
  lcd.backlight();  
  lcd.print("LCD is running..");

  // must be LOW when buzzer not in use
  digitalWrite(BUZZER_PIN, LOW); 

  // get default values from EEPROM
  getEEPROM();
  
}

void loop() {
  
}

// reads user settings from EEPROM; if EEPROM values are invalid, write defaults
void getEEPROM() {
  uint16_t identifier = (EEPROM.read(0) << 8) | EEPROM.read(1);
  if (identifier == EEPROM_IDENT) {
    scaleFacter           = (EEPROM.read(2) << 8) | EEPROM.read(3);
    gravityAcceleration   = (EEPROM.read(4) << 8) | EEPROM.read(5);
    units                 = EEPROM.read(6);
    beepEnable            = EEPROM.read(6);
  }
  else {
    EEPROM.update(0, EEPROM_IDENT >> 8); EEPROM.update(1, EEPROM_IDENT & 0xFF);
    updateEEPROM();
  }
}

// writes user settings to EEPROM using updade function to minimize write cycles
void updateEEPROM() {
  
  EEPROM.update( 2, scaleFacter >> 8);
  EEPROM.update( 3, scaleFacter & 0xFF);
  EEPROM.update( 4, gravityAcceleration >> 8);
  EEPROM.update( 5, gravityAcceleration & 0xFF);
  EEPROM.update( 6, units);
  EEPROM.update( 7, beepEnable);
}
