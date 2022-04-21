#include "Wire.h"
#include "U8g2lib.h"
#include "HX711.h"
#include "EEPROM.h"

// Pins define
// Define keyboard pins
int scanPin[] = {2, 3};
int receivePin[] = {14, 15};
// Define scale pins
#define HX711_DT_PIN 4
#define HX711_SCK_PIN 5
// Define motor pins
int motorPin1 = 6;
int motorPin2 = 7;
int ENA = 10;
// Battery pin
#define batteryPin 16

// Values define
// Default values
#define DEFAULT_SCALE_FACTOR 2080
// EEPROM identifier
#define EEPROM_IDENT 0xE76A
float scaleFacter = (float)DEFAULT_SCALE_FACTOR / 100.00;
// Keyboard Pins len
#define scanPin_len 2
#define receivePin_len 2
// Define keyMap
const String keyMap[scanPin_len][receivePin_len] = 
{
    {"UP", "DOWN"},
    {"ENTER", "ESC"}
};
// Menu Items
const String menuItems[] = {"Measure", "Calibrate"};

// Variables
// Place to store keyState
uint8_t btn[scanPin_len][receivePin_len];
// Measure value
float value = 0;
uint8_t batteryValue = 0;
// Menu selection
uint8_t selection = 0;
bool Exit = 0;

// Objects
// Create u8g2 && scale object
U8G2_SSD1306_128X64_NONAME_2_SW_I2C u8g2(U8G2_R0, /* clock=*/ SCL, /* data=*/ SDA, /* reset=*/ U8X8_PIN_NONE);
HX711 scale;

void setup() {
    // setup keyboard
    for(int i = 0; i < scanPin_len; i++) {
        pinMode(scanPin[i], OUTPUT);  
    }
    for(int i = 0; i < receivePin_len; i++) {
        pinMode(receivePin[i], INPUT);  
    }
    for(int i = 0; i < scanPin_len; i++) {
        for(int j = 0; j < receivePin_len; j++) {
            btn[i][j] = 0;  
        }  
    }

    // Set motor pins
    pinMode(motorPin1, OUTPUT);
    pinMode(motorPin2, OUTPUT);

    // get default values from EEPROM
    getEEPROM();

    // initialize the display && scale
    u8g2.begin();
    u8g2.setFont(u8g2_font_ncenB12_tr);
    scale.begin(HX711_DT_PIN, HX711_SCK_PIN);
    scale.power_up();
    scale.set_scale(scaleFacter);
    scale.tare();
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

void updateEEPROM() {
    uint16_t SCALE_FACTOR_INSTORE = scaleFacter * 100;
    EEPROM.update( 2, SCALE_FACTOR_INSTORE >> 8);
    EEPROM.update( 3, SCALE_FACTOR_INSTORE & 0xFF);
}

void scanKeyboard() {
    for(int i = 0; i < scanPin_len; i++) {
        digitalWrite(scanPin[i], HIGH);
        for(int j = 0; j < receivePin; j++) {
              if(digitalRead(receivePin[j]) == HIGH && btn[i][j] < 255) {
                  btn[i][j] ++;  
              }
              else if(btn[i][j] == 255) {
                  ;  
              }
              else {
                  btn[i][j] = 0;
              }
        }
        digitalWrite(scanPin[i], LOW);
    }
}

void measure() {
    value = scale.get_units(1);
}

void updateScreen() {
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

void motorControl() {
    if(btn[0][0] > 0) {
        digitalWrite(motorPin1, HIGH); 
        digitalWrite(motorPin2, LOW);
        analogWrite(ENA, 200);
    }
    else if(btn[0][1] > 0) {
        digitalWrite(motorPin2, HIGH); 
        digitalWrite(motorPin1, LOW);
        analogWrite(ENA, 200);
    }
    else {
        digitalWrite(motorPin1, LOW); 
        digitalWrite(motorPin2, LOW);
        analogWrite(ENA, 0);
    }
}

void menuCheck() {
    if(btn[1][1] > 254) {
        digitalWrite(motorPin1, LOW); 
        digitalWrite(motorPin2, LOW);
        analogWrite(ENA, 0);
        menu();
    }
}

void menu() {
    do {
        scanKeyboard();
        updateSelection();
        menuScreen();
        checkExecute();
    } while(Exit);
    Exit = 0;
}

void checkExecute() {
    if(btn[1][0] == 1) {
        switch(selection) {
            case 0: Calibration();  break;
            default: Exit = 1;      break;
        }
    }  
}

void Calibration() {
    ;
}

void menuScreen() {
    if(selection == 0) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 15);
            u8g2.print(">" + menuItems[0]);
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 40);
            u8g2.print(menuItems[1]);
        } while ( u8g2.nextPage() );
    }
    else if(selection == 1) {
        u8g2.firstPage();
        do {
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 15);
            u8g2.print(menuItems[0]);
            u8g2.setFont(u8g2_font_ncenB12_tr);
            u8g2.setCursor(0, 40);
            u8g2.print(">" + menuItems[1]);
        } while ( u8g2.nextPage() );
    }
}

void updateSelection() {
    if(btn[0][0] == 1 && selection > 0) {
        selection --;
    }
    else if(btn[0][1] == 1 && selection < 1) {
        selection ++;  
    } 
}

void batteryCheck() {
    batteryValue = analogRead(batteryPin) / 1024 * 100;
}

void loop() {
    scanKeyboard();
    measure();
    updateScreen();
    motorControl();
    batteryCheck();
    menuCheck();
}
