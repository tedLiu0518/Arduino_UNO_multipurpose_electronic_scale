// Libraries
#include "LiquidCrystal_I2C.h"              // https://github.com/johnrickman/LiquidCrystal_I2C
#include "HX711.h"                          // https://github.com/bogde/HX711

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
#define  LONG_PRESS             1000
#define  SHORT_PRESS              10
#define REFRESH_RATE             100

// Menu items
const String MainScreenItems[]     =  { "Measure", "Setting", "Information", "Advance" };

// Timing && selection variables
uint32_t  buttonMillis, currentMillis, lcdMillis;
uint8_t   selected;

// Default values that can be changed by the user and store in the EEPROM 
float     scaleFacter              =  (float)DEFAULT_SCALE_FACTOR / 100.00;

// Create lcd && scale object
LiquidCrystal_I2C lcd(0x27, 16, 2);
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

    // initialize the lcd && scale
    lcd.init();     
    lcd.backlight();  
    lcd.print("LCD is running..");
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.power_down();	
}

void checkRelay(float value) {
    if(value >= 1500) {
        digitalWrite(RELAY_1_PIN, LOW);
        digitalWrite(RELAY_2_PIN, LOW);
    }
    else{
        digitalWrite(RELAY_2_PIN, HIGH);
        digitalWrite(RELAY_1_PIN, HIGH);
    }
}

void measureScreen(float value) {
    if(currentMillis - lcdMillis >= REFRESH_RATE) {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print(value);
        lcd.setCursor(0,1);
        lcd.print("gram(g)");
        lcdMillis = millis();
    }      
}

void Measure() {
    scale.power_up();
    scale.set_scale(scaleFacter);
    scale.tare();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("measuring..");
    lcdMillis = millis();
    while(!buttonCheck(BTN_ESC_PIN, LONG_PRESS)){
        float value = scale.get_units(10);
        currentMillis = millis();
        measureScreen(value);
        checkRelay(value);
    }
    scale.power_down();
    digitalWrite(RELAY_2_PIN, HIGH);
    digitalWrite(RELAY_1_PIN, HIGH);
}

void mainScreen() {
    int selectedFunc = selection(MainScreenItems, 4);
    switch (selectedFunc) {
        case 0:   Measure();    break;
        // case 1:   Setting();        break;
        // case 2:   Information();    break;
        // case 3:   Advance();        break;
        default:                       break; //break or exit
    }
}

int selection(String Items[], uint8_t numberOfItems) {
    selected = 0;
    lcdMillis = millis();
    while(1) {
        currentMillis = millis();
        if(currentMillis - lcdMillis >= 100){
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
        if(buttonCheck(BTN_ESC_PIN, SHORT_PRESS)) {
            return numberOfItems;
        }
    }
}

bool buttonCheck(int buttonPin, uint32_t timeSetted) {
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
    mainScreen();
}
