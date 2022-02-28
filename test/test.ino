// Libraries
#include "LiquidCrystal_I2C.h"              // https://github.com/johnrickman/LiquidCrystal_I2C

// Pins
#define  BTN_ENTER_PIN             4        // Enter button Input
#define  BTN_UP_PIN                5        // Up button Input
#define  BTN_DOWN_PIN              6        // Down button Input
#define  BTN_ESC_PIN               7        // ESC button Input

// Menu items
const String MainScreenItems[]      =  { "Measurement", "Setting", "Information", "Advance" };

// Timing variables
uint32_t  buttonMillis, currentMillis, lcdMillis;

// Selection variables
uint8_t selected;

// Create lcd object
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
    // set the buttons pin modes
    pinMode(BTN_ENTER_PIN, INPUT_PULLUP);
    pinMode(BTN_UP_PIN, INPUT_PULLUP);
    pinMode(BTN_DOWN_PIN, INPUT_PULLUP);
    pinMode(BTN_ESC_PIN, INPUT_PULLUP);

    // initialize the lcd
    lcd.init();     
    lcd.backlight();  
    lcd.print("LCD is running..");
}

void mainMenu() {
    int selectedFunc = selection(MainScreenItems, 4);
    switch (selectedFunc) {
        // case 0:   Measurement();    break;
        // case 1:   Setting();        break;
        // case 2:   Information();    break;
        // case 3:   Advance();        break;
        default:                       break; //break or exit
    }
}

int selection(String Items[], uint8_t numberOfItems) {
    selected = 0;
    lcdMillis = millis();
    while(!buttonCheck(BTN_ESC_PIN, 1)) {
        currentMillis = millis();
        if(currentMillis - lcdMillis >= 100){
            updateScreen(Items, numberOfItems, selected);
            lcdMillis = millis();
        }
        if(buttonCheck(BTN_UP_PIN, 1) && (selected != 0)) {
            selected --;
        }
        if(buttonCheck(BTN_DOWN_PIN, 1) && (selected != (numberOfItems - 1))) {
            selected ++;
        }
        if(buttonCheck(BTN_ENTER_PIN, 1)) {
            return selected;
        }
    }
    return numberOfItems;
}

bool buttonCheck(int buttonPin, uint32_t timeSetted) {
    currentMillis = millis();
    while(!digitalRead(buttonPin)) {
        buttonMillis = millis();
        if((buttonMillis - currentMillis >= timeSetted) && digitalRead(buttonPin)) return true;
    }
    return false;
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

void loop(){
    mainMenu();
}


