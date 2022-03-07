// Multipurpose_Electronic_Scales
//
// Arduino_UNO_Rev3 ATmega328P && Arduino_Nano ATmega328 controlled multipurpose electronic scale.
//
// This version of the code implements (not yet):
// - Calibrating and storing the scale factor into the EEPROM
// - Automatically enter measure mode after start-up
// - Setup menu by long-pressing the Esc button
// 
// Power supply should be in the range of 7-12V according to the
// Arduino_UNO_Rev3 tech specs.
//
// For calibration you need a known weight load that close to the 
// maximum range of your load cell.
//
// Board: Arduino UNO Rev3 && Arduino Nano
// Microcontroller: ATmega328P && ATmega328
// Clock speed: 16 MHz
//
// Contributions & Special thanks:
// The program structure refers to the following project:
// - Stefan Wagner, https://github.com/wagiminator/ATmega-Soldering-Station
// Contributors:
// - 
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
#include "math.h"                           // https://github.com/avrdudes/avr-libc

// Firmware version
#define  VERSION              "v0.3"

// Pins
#define  HX711_DT_PIN              2        // HX711 module Data IO Connection
#define  HX711_SCK_PIN             3        // HX711 module Serial Clock Input
#define  BTN_ENTER_PIN             4        // Enter button Input
#define  BTN_UP_PIN                5        // Up button Input
#define  BTN_DOWN_PIN              6        // Down button Input
#define  BTN_ESC_PIN               7        // ESC button Input

// Default values                   
#define  DEFAULT_SCALE_FACTOR   2080        // Default scale factor of DYLT-101 "S" Type load cell (20.80)

// EEPROM identifier
#define  EEPROM_IDENT         0xE76A        // to identify if EEPROM was written by this program

// Define long press and short press time set (ms)
#define  LONG_PRESS             1000
#define  SHORT_PRESS              10

// Define the refresh time set of the LCD (1/ms)
#define REFRESH_RATE             100

// Timing variables
uint32_t  buttonMillis, currentMillis, lcdMillis;

// Selection variables
uint8_t selected;

// Default values that can be changed by the user and store in the EEPROM 
float    scaleFacter               =  (float)DEFAULT_SCALE_FACTOR / 100.00;

// Variables for running
// uint8_t  SetUnit;
// float    SetScaleFactor, SetGravityAcc;

// Menu items
const String MainScreenItems[]     =  { "Measurement", "Setting", "Information" };
const String SureItems[]           =  { "Are you sure ?", "No", "Yes" };
const String SettingItems[]        =  { "Calibration", "Set scale factor" };
const String InformationItems[]    =  { "Scale factor", "Firmware version" };
const String StoreItems[]          =  { "Store Settings ?", "No", "Yes" };

// Create hx711 object
HX711 hx711;

// Create lcd object
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    // set the buttons pin modes
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_ESC_PIN, INPUT_PULLUP);
  
    // initialize the HX711 
    hx711.begin(HX711_DT_PIN, HX711_SCK_PIN);
    hx711.power_down();	

    // initialize the lcd
    lcd.init();     
    lcd.backlight();  
    lcd.print("LCD is running..");
    delay(REFRESH_RATE);

    // get default values from EEPROM
    getEEPROM();
    
    Measurement();
}

void loop() {
    mainMenu();
}

// reads user settings from EEPROM; if EEPROM values are invalid, write defaults
void getEEPROM() {
    uint16_t IntscaleFacter = 0;
    uint16_t identifier = (EEPROM.read(0) << 8) | EEPROM.read(1);
    if (identifier == EEPROM_IDENT) {
        IntscaleFacter = (EEPROM.read(2) << 8) | EEPROM.read(3);
        scaleFacter = (float)IntscaleFacter / 100.00;
    }
    else {
        EEPROM.update(0, EEPROM_IDENT >> 8); EEPROM.update(1, EEPROM_IDENT & 0xFF);
        updateEEPROM();
    }
}

// writes user settings to EEPROM using updade function to minimize write cycles
void updateEEPROM() {
    uint16_t SCALE_FACTOR_INSTORE = scaleFacter * 100;
    EEPROM.update( 2, SCALE_FACTOR_INSTORE >> 8);
    EEPROM.update( 3, SCALE_FACTOR_INSTORE & 0xFF);
}

void Measurement(){
    hx711.power_up();
    hx711.set_scale(scaleFacter);
    lcd.clear();
    lcd.print("taring.."); 
    hx711.tare();
    delay(REFRESH_RATE);
    lcd.clear();
    lcd.print("measuring..");
    delay(REFRESH_RATE);
    lcdMillis = millis();
    while(!buttonCheck(BTN_ESC_PIN, LONG_PRESS)){
        float value = hx711.get_units(10);
        delay(100);
        currentMillis = millis();
        if(currentMillis - lcdMillis >= REFRESH_RATE){
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(value);
            lcd.setCursor(0,1);
            lcd.print("gram(g)");
            lcdMillis = millis();
        }     
    }  
    hx711.power_down();
}

uint16_t numberInput(int numberSize){
    int selectedNum[numberSize] = {0};
    for(int i = 0; i <= numberSize-1 ; i++){
        lcdMillis = millis();
        while(!buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)){
            currentMillis = millis();
            if(currentMillis - lcdMillis >= REFRESH_RATE){
                CalibrationScreen(selectedNum, numberSize);
                lcdMillis = millis();
            }
            if(buttonCheck(BTN_DOWN_PIN, SHORT_PRESS) && (selectedNum[i] != 0)) {
                selectedNum[i]--;
            }
            if(buttonCheck(BTN_UP_PIN, SHORT_PRESS) && (selectedNum[i] != 9)) {
                selectedNum[i]++;
            }
        }
    }
    uint16_t numberReturn = 0;
    for(int i = 0; i<= numberSize-1 ; i++){
        numberReturn = numberReturn + (selectedNum[i]*pow(10,i));
    }
    return numberReturn;
}

void CalibrationScreen(int selectedNum[], int numberSize){
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Input sample weight (g)..");
    for(int i = 0; i < numberSize; i++){
        lcd.setCursor((15-i),1);
        lcd.print(selectedNum[i]);
    } 
}

void Calibration(){
    hx711.power_up();
    hx711.set_scale();  //set current scale to 0 
    lcd.clear();
    lcd.print("Nothing on it...");
    delay(REFRESH_RATE);
    hx711.tare();       //set current load to 0
    lcd.clear();
    lcd.print("Set sapmple object(g)"); 
    delay(REFRESH_RATE);
    float sample_weight = (float)numberInput(5)/100.00;
    lcd.clear();
    lcd.print("Put sample object...");
    delay(REFRESH_RATE);
    float current_weight = 0;
    while(!buttonCheck(BTN_ENTER_PIN, LONG_PRESS)){
        current_weight = hx711.get_units(10); 
    }
    float scale_factor = (current_weight/sample_weight);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Scale number:");
    lcd.setCursor(0,1);
    lcd.print(scale_factor); 
    delay(10000);
    storeInput(scale_factor);
}

void storeInput(float scale_factor){
    int selectedFunc = selection(SettingItems, 2);
}

void Setting() {
    int selectedFunc = selection(SettingItems, 2);
    switch (selectedFunc) {
        case 0:   Calibration();    break;
        // case 1:   Set_scale_factor();  break;
        default:  break;
    }
}

void Information(){
       while(1){
    lcd.print("information");
    }
}

void mainMenu() {
    int selectedFunc = selection(MainScreenItems, 3);
    switch (selectedFunc) {
        case 0:   Measurement();    break;
        case 1:   Setting();        break;
        case 2:   Information();    break;
        default:                    break;
    }
}

void updateScreen(String Items[], uint8_t numberOfItems, uint8_t selected) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(">" + Items[selected]);
    if(selected <= (numberOfItems-2)) {
        lcd.setCursor(0, 1);
        lcd.print(Items[selected+1]);
    }
}

int selection(String Items[], uint8_t numberOfItems) {
    selected = 0;
    lcdMillis = millis();
    while(1) {
        currentMillis = millis();
        if(currentMillis - lcdMillis >= REFRESH_RATE){
            updateScreen(Items, numberOfItems, selected);
            lcdMillis = millis();
        }
        if(buttonCheck(BTN_UP_PIN, SHORT_PRESS) && (selected != 0)) {
            selected --;
        }
        if(buttonCheck(BTN_DOWN_PIN, SHORT_PRESS) && (selected != (numberOfItems - 1))) {
            selected ++;
        }
        if(buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
            return selected;
        }
        if(buttonCheck(BTN_ESC_PIN, SHORT_PRESS)){
            return numberOfItems;
        }
    }
}

// Function to check whether a button is pressed and released, when released check the pressing time,
// if the pressing time bigger than timeSetted return true.
// When a button is pressed the task will locked (hope to fix)
bool buttonCheck(uint8_t buttonPin, uint32_t timeSetted) {
    currentMillis = millis();
    while(!digitalRead(buttonPin)) {
        buttonMillis = millis();
        if(buttonMillis - currentMillis >= timeSetted){
            while(!digitalRead(buttonPin));
            return true;
        } 
    }
    return false;
}
