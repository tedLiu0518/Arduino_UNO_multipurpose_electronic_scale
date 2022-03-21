// Multipurpose_Electronic_Scales
//
// Arduino_UNO_Rev3 ATmega328P && Arduino_Nano ATmega328 controlled multipurpose electronic scale.
//
// This version of the code implements:
// - Calibrating and storing the scale factor into the EEPROM
// - Automatically enter measure mode after start-up
// - Setup menu by long-pressing the Esc button
// - Two Relay pin to control motor or etc
// - New SSD1306oled display
// 
// Power supply should be in the range of 7-12V according to the
// Arduino_UNO_Rev3 && Arduino_Nano tech specs.
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
#include "Wire.h"                           // https://github.com/esp8266/Arduino/blob/master/libraries/Wire
#include "U8g2lib.h"                        // https://github.com/olikraus/u8g2
#include "HX711.h"                          // https://github.com/bogde/HX711
#include "EEPROM.h"                         // https://github.com/PaulStoffregen/EEPROM

// Firmware version
#define  VERSION              "v0.6"

// Pins
#define  HX711_DT_PIN              2        // HX711 module Data IO Connection
#define  HX711_SCK_PIN             3        // HX711 module Serial Clock Input
#define  BTN_ENTER_PIN             4        // Enter button Input
#define  BTN_UP_PIN                5        // Up button Input
#define  BTN_DOWN_PIN              6        // Down button Input
#define  BTN_ESC_PIN               7        // ESC button Input
#define  RELAY_1_PIN               8        // RELAY 1 PIN
#define  RELAY_2_PIN               9        // RELAY 2 PIN

// Default values                   
#define  DEFAULT_SCALE_FACTOR   2080        // Default scale factor of DYLT-101 "S" Type load cell (20.80)
#define  LONG_PRESS             1000        // Default time for long press (millisecond)
#define  SHORT_PRESS              10        // Default time for short press (millisecond)
#define  REFRESH_RATE            100        // Refresh time for display (millisecond)
#define  RELAY_VALUE             100        // Default value for toggle Relays (gram)

// EEPROM identifier
#define  EEPROM_IDENT         0xE76A        // to identify if EEPROM was written by this program

// Menu items
const String MainScreenItems[]           =  { "Measure", "Calibrate", "Information" };
const String PutSampleScreenItems[]      =  { "Calibrate", "Put Sample", "Press ENTER" };
const String ClearLoadScreenItems[]      =  { "Calibrate", "Clear Load", "Press ENTER" };
const String TareItems[]                 =  { "Measure", "Tare: ENTER", "" };

// Timing && selection variables
uint32_t     buttonMillis, currentMillis, displayMillis;
uint8_t      selected;

// Default values that can be changed by the user and store in the EEPROM 
float        scaleFacter           =  (float)DEFAULT_SCALE_FACTOR / 100.00;

// Create u8g2 && scale object
U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
HX711 scale;

void setup() {
    // set the buttons pin modes
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_ESC_PIN, INPUT_PULLUP);
    pinMode(RELAY_1_PIN, OUTPUT);
    pinMode(RELAY_2_PIN, OUTPUT);

    // set default relay output
    digitalWrite(RELAY_1_PIN, HIGH);
    digitalWrite(RELAY_2_PIN, HIGH);

    // initialize the display && scale
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB14_tr);
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.power_down();	

    // get default values from EEPROM
    getEEPROM();

    Measure();
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

// writes user settings to EEPROM using update function to minimize write cycles
void updateEEPROM() {
    uint16_t SCALE_FACTOR_INSTORE = scaleFacter * 100;
    EEPROM.update( 2, SCALE_FACTOR_INSTORE >> 8);
    EEPROM.update( 3, SCALE_FACTOR_INSTORE & 0xFF);
}

// when the load is greater than setted, toggle relay
void checkRelay(uint16_t value) {
    if (!digitalRead(BTN_DOWN_PIN)) {
        digitalWrite(RELAY_1_PIN, HIGH);
        digitalWrite(RELAY_2_PIN, LOW);
    }
    else {
        if (value >= RELAY_VALUE) {
            digitalWrite(RELAY_1_PIN, HIGH);
        }
        else {
            if(!digitalRead(BTN_UP_PIN)) {
                digitalWrite(RELAY_2_PIN, HIGH);
                digitalWrite(RELAY_1_PIN, LOW);
            }
        }
    }
}

void ResetRelay() {
    if (!digitalRead(BTN_UP_PIN) || !digitalRead(BTN_DOWN_PIN));
    else {
        digitalWrite(RELAY_2_PIN, HIGH);
        digitalWrite(RELAY_1_PIN, HIGH);
    }
}

void measureScreen(uint16_t value) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 15);
            u8g2.print("Measure");
            u8g2.setFont(u8g2_font_ncenB18_tr);
            u8g2.setCursor(0, 41);
            u8g2.print(value);
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 58);
            u8g2.print("gram(g)");
        } while(u8g2.nextPage());     
}

// in this mode btn up and btn down will control the relay 
void Measure() {
    scale.power_up();
    scale.set_scale(scaleFacter);
    ConfirmScreen(TareItems);
    while (1) {
        if (buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
            scale.tare();
            break;
        }
        if (buttonCheck(BTN_ESC_PIN, SHORT_PRESS)) {
            break;
        }
    }
    while (!buttonCheck(BTN_ESC_PIN, LONG_PRESS)) {
        uint16_t value = scale.get_units(10);
        measureScreen(value);
        checkRelay(value);
        ResetRelay();
    }
    scale.power_down();
    digitalWrite(RELAY_2_PIN, HIGH);
    digitalWrite(RELAY_1_PIN, HIGH);
}

// only accept 3 items, first is current task, second is confirm content, third is action
void ConfirmScreen(String Items[]) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 15);
            u8g2.print(Items[0]);
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 32);
            u8g2.print(Items[1]);		
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 50);
            u8g2.print(Items[2]);
        } while ( u8g2.nextPage() );
}

uint16_t numberInput(int numberSize){
    int selectedNum[numberSize] = {0};
    for (int i = 0; i <= numberSize-1 ; i++) {
        CalibrationScreen(selectedNum, numberSize);
        while (!buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
               
            if (buttonCheck(BTN_DOWN_PIN, SHORT_PRESS) && (selectedNum[i] != 0)) {
                selectedNum[i]--;
                 CalibrationScreen(selectedNum, numberSize);
            }
            if (buttonCheck(BTN_UP_PIN, SHORT_PRESS) && (selectedNum[i] != 9)) {
                selectedNum[i]++;
                 CalibrationScreen(selectedNum, numberSize);
            }
        }
    }
    uint16_t numberReturn = 0;
    for (int i = 0; i<= numberSize-1 ; i++) {
        numberReturn = numberReturn + (selectedNum[i]*pow(10,i));
    }
    return numberReturn;
}

void Calibration() {
    scale.power_up();
  ConfirmScreen(ClearLoadScreenItems);
    while (1) {
        if (buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
            break;
        }
        if (buttonCheck(BTN_ESC_PIN, SHORT_PRESS)) {
            mainScreen();
        }
    }
    scale.set_scale();
    scale.tare();
    float sample_weight = (float)numberInput(5)/100.00;
  ConfirmScreen(PutSampleScreenItems);
    while (1) {
        if (buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
            break;
        }
        if (buttonCheck(BTN_ESC_PIN, SHORT_PRESS)) {
            mainScreen();
        }
    }
    float current_weight = scale.get_units(10);
    scaleFacter = (current_weight / sample_weight);
            confirmScale();
    while (1) {
        if (buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
            updateEEPROM();
            break;
        }
        if (buttonCheck(BTN_ESC_PIN, SHORT_PRESS)) {
            mainScreen();
        }
    }
    getEEPROM();
}

void confirmScale() {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 15);
            u8g2.print("Scale factor:");
            u8g2.setFont(u8g2_font_ncenB14_tr);
            u8g2.setCursor(0, 40);
            u8g2.print(scaleFacter);		
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 58);
            u8g2.print("Save: ENTER");
        } while ( u8g2.nextPage() );
}

void CalibrationScreen(int selectedNum[], int numberSize) {
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB12_tr);
        u8g2.setCursor(0, 15);
        u8g2.print("Sample(g):");
        for (int i = 0; i < numberSize; i++) {
            u8g2.setFont(u8g2_font_ncenB14_tr);
            u8g2.setCursor((100-i*10),40);
            u8g2.print(selectedNum[i]);
        } 
        u8g2.setFont(u8g2_font_ncenB12_tr);
        u8g2.setCursor(0, 58);
        u8g2.print("Next: ENTER");
    } while ( u8g2.nextPage() );
}

void mainScreen() {
    int selectedFunc = selection(MainScreenItems, 3);
    switch (selectedFunc) {
        case 0:   Measure();           break;
        case 1:   Calibration();       break;
        // case 2:   Information();    break; //show current scale factor
        default:                       break; //break or exit
    }
}

int selection(String Items[], uint8_t numberOfItems) {
    selected = 0;
    updateScreen(Items, numberOfItems, selected);
    while(1) {
        if(buttonCheck(BTN_UP_PIN, SHORT_PRESS) && (selected != 0)) {
            selected --;
            updateScreen(Items, numberOfItems, selected);
        }
        if(buttonCheck(BTN_DOWN_PIN, SHORT_PRESS) && (selected != (numberOfItems - 1))) {
            selected ++;
            updateScreen(Items, numberOfItems, selected);
        }
        if(buttonCheck(BTN_ENTER_PIN, SHORT_PRESS)) {
            return selected;
        }
        if(buttonCheck(BTN_ESC_PIN, SHORT_PRESS)) {
            return numberOfItems;
        }
    }
}

bool buttonCheck(int buttonPin, uint32_t timeSetted) {
    currentMillis = millis();
    while(!digitalRead(buttonPin)) {
        buttonMillis = millis();
        if(buttonMillis - currentMillis >= timeSetted) {
            while(!digitalRead(buttonPin));
            return true;
        } 
    }
    return false;
}

void updateScreen(String Items[], uint8_t numberOfItems, uint8_t selected) {
    u8g2.firstPage();
    do {
        u8g2.setFont(u8g2_font_ncenB12_tr);
        u8g2.setCursor(0, 15);
        u8g2.print(">" + Items[selected]);
        if(selected <= (numberOfItems-2)) {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 40);
            u8g2.print(Items[selected+1]);
        }
    } while ( u8g2.nextPage() );
}

void loop() {
    mainScreen();
}
