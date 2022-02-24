// Multipurpose_Electronic_Scales
//
// Arduino_UNO_Rev3 ATmega328P controlled multipurpose electronic scale.
//
// This version of the code implements (not yet):
// - Calibrating and store the scale factor into the EEPROM
// - Automatically enter weighing mode after start-up
// - Setup menu by long pressing the Enter button
// - Advanced function to control other pins (main purpose of this project)
// - Buzzer
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
//
// 2022 by Ted Liu
// Project Files (Github): https://github.com/tedLiu0518/Arduino_UNO_multipurpose_electronic_scale
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
#define  VERSION              "v0.1"

//Pins
#define  HX711_DT_PIN            "2"        // HX711 module Data IO Connection
#define  HX711_SCK_PIN           "3"        // HX711 module Serial Clock Input
#define  BTN_ENTER_PIN           "6"        // Enter button Input
#define  BTN_UP_PIN              "7"        // Up button Input
#define  BTN_DOWN_PIN            "8"        // Down button Input
#define  BTN_ESC_PIN             "9"        // ESC button Input
#define  BUZZER_PIN             "10"        // Buzzer

// Default values
#define  DEFAULT_SCALE_FACTOR   2080        // Default scale factor of DYLT-101 "S" Type load cell (20.80)
#define  GRAVITY_ACCELERATION    981        // Default gravitational acceleration (9.81)
#define  BEEP_ENABLE            true        // enable/disable buzzer

// Default measurement unit
#define  DEFAULT_UNIT              3        // 1 for kgf, 2 for gf, 3 for N, 4 for kg, 5 for g

// EEPROM identifier
#define  EEPROM_IDENT         0xE76B        // to identify if EEPROM was written by this program

// Timing variables
uint32_t  buttonmillis, currentmillis;

//Default values that can be changed by the user and store in the EEPROM 
float    scaleFacter               =  (float)DEFAULT_SCALE_FACTOR / 100.00;
float    gravityAcceleration       =  (float)GRAVITY_ACCELERATION / 100.00;
uint8_t  units                     =  DEFAULT_UNIT;
bool     beepEnable                =  BEEP_ENABLE;

// Variables for temperature control
uint8_t  SetUnit;
float    SetScaleFactor, SetGravityAcc;
bool     SetBeepEnable;

// Menu items
const char *MainScreenItems[]      =  { "Measurement", "Setting", "Information", "Advance" };
const char *SureItems[]            =  { "Are you sure ?", "No", "Yes" };
const char *SettingItems[]         =  { "Calibration", "Gravity setting", "Set scale factor" };
const char *InformationItems[]     =  { "Scale factor", "Gravity acceleration", "Firmware version" };
const char *StoreItems[]           =  { "Store Settings ?", "No", "Yes" };
const char *UnitsItems[]           =  { "kgf", "gf", "N", "kg", "g" };
const char *AdvanceItems[]         =  { "Comming Soon" };

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

  // read and set current value
  SetScaleFactor = scaleFacter;
  SetGravityAcc = gravityAcceleration;
  SetUnit = units;
  SetBeepEnable = beepEnable;
}

// reads user settings from EEPROM; if EEPROM values are invalid, write defaults
void getEEPROM() {
  uint16_t identifier = (EEPROM.read(0) << 8) | EEPROM.read(1);
  if (identifier == EEPROM_IDENT) {
    scaleFacter           = (EEPROM.read(2) << 8) | EEPROM.read(3);
    gravityAcceleration   = (EEPROM.read(4) << 8) | EEPROM.read(5);
    units                 = EEPROM.read(6);
    beepEnable            = EEPROM.read(7);
  }
  else {
    EEPROM.update(0, EEPROM_IDENT >> 8); EEPROM.update(1, EEPROM_IDENT & 0xFF);
    updateEEPROM();
  }
}

// writes user settings to EEPROM using updade function to minimize write cycles
void updateEEPROM() {
  uint16_t SCALE_FACTOR_INSTORE = scaleFacter * 100;
  uint16_t GRAVITY_ACC_INSTORE = gravityAcceleration * 100;
  EEPROM.update( 2, SCALE_FACTOR_INSTORE >> 8);
  EEPROM.update( 3, SCALE_FACTOR_INSTORE & 0xFF);
  EEPROM.update( 4, GRAVITY_ACC_INSTORE >> 8);
  EEPROM.update( 5, GRAVITY_ACC_INSTORE & 0xFF);
  EEPROM.update( 6, units);
  EEPROM.update( 7, beepEnable);
}

 void loop() {
  ButtonCheck();
}

// void MainScreen(){


// }



// void updateMenu(char Items[], menu){
//     lcd.clear();
//     lcd.setCursor(0, 0);
//     lcd.print(Items[menu]);
//     lcd.setCursor(0, 1);
//     if(menu != Items.length()){
//         lcd.print(Items[menu+1])
//     }
//     else{
//         lcd.print("none")
//     }

// }


// int drawMenu(char Items[]){
//     int menu = 1;
//     while(!digitalRead(enterButton)||!digitalRead(exeButton)){
//     if(!digitalRead(downButton)){
//         while(!digitalRead(downButton));
//         if(menu != Items.length()){
//     menu++;
//     }
//     updateMenu( Items[],  menu); 
//     delay(100); 
//   }
//   if(!digitalRead(upButton)){
//       while(!digitalRead(upButton));
//       if(menu != 0){
//     menu--;
//       }
//     updateMenu(Items[],  menu); 
//     delay(100);
//   }
//     }
//     if(!digitalRead(enterButton)){
//         while(!digitalRead(enterButton));
//         return menu
//     }
//     if(!digitalRead(escButton)){
//         while(!digitalRead(escButton));
//         return 0
//     }
//   }
    
// }

// Check the ENTER button; enter setup menu accordingly
void ButtonLongPress() {
  buttonmillis = millis();
  while( (!digitalRead(BTN_ENTER_PIN)) && ((millis() - buttonmillis) < 500) );
  if ((millis() - buttonmillis) >= 500) SetupScreen();
}

void ButtonShortPress(int BUTTON_PIN){
    buttonmillis = millis();
    while(!digitalRead(BUTTON_PIN));

}

void SetupScreen() {
  uint8_t selection = 0;
  bool repeat = true;
  while (repeat) {
    selection = MenuScreen(SetupItems, sizeof(SetupItems), selection);
    switch (selection) {
      case 0:   TipScreen(); repeat = false; break;
      case 1:   TempScreen(); break;
      case 2:   TimerScreen(); break;
      case 3:   PIDenable = MenuScreen(ControlTypeItems, sizeof(ControlTypeItems), PIDenable); break;
      case 4:   MainScrType = MenuScreen(MainScreenItems, sizeof(MainScreenItems), MainScrType); break;
      case 5:   beepEnable = MenuScreen(BuzzerItems, sizeof(BuzzerItems), beepEnable); break;
      case 6:   InfoScreen(); break;
      default:  repeat = false; break;
    }
  }  
  updateEEPROM();
  SetTemp = SaveSetTemp;
  setRotary(TEMP_MIN, TEMP_MAX, TEMP_STEP, SetTemp);
}

// menu screen
uint8_t MenuScreen(const char *Items[], uint8_t numberOfItems, uint8_t selected) {
  selected = 0
  while(digitalRead(BTN_ENTER_PIN)&&digitalRead(BTN_ESC_PIN)){
      checkbuttonup&BTN_DOWN_PIN
          if BTN_DOWN_PIN
          selected++
          if BTN_UP_PIN
          selected--
  }
  uint8_t lastselected = selected;
  int8_t  arrow = 0;
  
  return selected;
}



int selection(const char *Items[], uint8_t numberOfItems,){
    selected = 0;
    while(btncheck(enter||esc)){
      menuscreen(const char *Items[], uint8_t numberOfItems, uint8_t selected)
      if(buttonCheck(up)&&selected!= 0) selected--
      if(buttonCheck(down)&&selected!=numberOfItems) selected++
    }
    return selection
}

bool buttonCheck(int buttonPin, uint32_t timeSetted){
    currentmillis = millis();
    while(!digitalRead(buttonPin)){
        buttonmillis = millis();
        if(buttonmillis >= timeSetted&&digitalRead(buttonPin)) return true;
    }
    return false;
}