#include <LiquidCrystal_I2C.h> // library for LCD

#include "phoneticAlphabet.h"

const byte ledPin = 13;
const byte estPin = 2;
const byte interruptPin = 5;

// action states for interrupts
volatile byte state = LOW;
volatile boolean actionState = LOW;

// variable for adjustable time delay
const int tdelayadjustable = 30;


// declare variables for decoding
volatile bool Q1_state;
volatile bool Q2_state;
volatile bool Q3_state;
volatile bool Q4_state;

volatile int decimal_value;

volatile String digit_string;






LiquidCrystal_I2C lcd(0x27,16,2);  // I2C address of LCD chip is 0x27. LCD is 16 chars and 2 lines.

void speak(char* msg) {
  Serial.write(0xFD);
  Serial.write((byte)0x0);
  Serial.write(2 + strlen(msg));
  Serial.write(0x01);
  Serial.write((byte)0x0);
  Serial.write(msg);
}

void busy() {
  state = !state;
  digitalWrite(ledPin, state);
}



void setup() {
  // set input pins
  pinMode(3, INPUT);     // Q1 from DTMF chip
  pinMode(estPin, INPUT);     // ESt from DTMF chip
  pinMode(4, INPUT);     // Q2 from DTMF chip
  pinMode(7, INPUT);     // Q3 from DTMF chip
  pinMode(8, INPUT);     // Q4 from DTMF chip

  pinMode(9, OUTPUT);   // enable transmit relay

  pinMode(ledPin, OUTPUT);
  pinMode(interruptPin, INPUT_PULLUP);

  // attach interrupt to speech synthesis chip pin
  //attachInterrupt(digitalPinToInterrupt(interruptPin), busy, CHANGE);

  // attach interrupt to ESt pin for DTMF decoding
  attachInterrupt(digitalPinToInterrupt(estPin), decodeDTMF, RISING);

  
  Serial.begin(4800);
  
  // set output pins
  //pinMode(6, OUTPUT);    // Speaker
  //pinMode(7, OUTPUT);    // GREEN LED indicator
  //pinMode(8, OUTPUT);    // RED LED indicator

  // LCD setup
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on
  
  // Print a message on both lines of the LCD.
  lcd.setCursor(2,0);   // Set cursor to character 2 on line 0
  lcd.print("Press START");
  lcd.setCursor(2,1);   // Move cursor to character 2 on line 1
  lcd.print("to begin.");
}

void decodeDTMF() {
//      // change action state of interrupt for decodeDTMF
        actionState != actionState;
//      
//      // DTMF Decoding Code
//
//      // wait the adjustable time delay
//      delay(tdelayadjustable);
//      
//
//      //bool ESt_state = digitalRead(3);
//
//      Q1_state = digitalRead(2);
//      Q2_state = digitalRead(4);
//      Q3_state = digitalRead(7);
//      Q4_state = digitalRead(8);
//
//      // Calculate the decimal value
//      decimal_value = 0;
//      if (Q4_state) decimal_value += 8; // Q4 represents 8
//      if (Q3_state) decimal_value += 4; // Q3 represents 4
//      if (Q2_state) decimal_value += 2; // Q2 represents 2
//      if (Q1_state) decimal_value += 1; // Q1 represents 1
//
//      // Convert the decimal value to a string and send it over serial
//      digit_string = String(decimal_value);
//
//      // LCD code, print the string of digits onto the LCD display

      
}

void loop() {

//      // DTMF Decoding Code
//
//      // wait the adjustable time delay
//      // delay(tdelayadjustable);
//      
//
//        bool ESt_state = digitalRead(3);
//
//      bool Q1_state = digitalRead(2);
//      bool Q2_state = digitalRead(4);
//      bool Q3_state = digitalRead(7);
//      bool Q4_state = digitalRead(8);
//
//      // Calculate the decimal value
//      int decimal_value = 0;
//      if (Q4_state) decimal_value += 8; // Q4 represents 8
//      if (Q3_state) decimal_value += 4; // Q3 represents 4
//      if (Q2_state) decimal_value += 2; // Q2 represents 2
//      if (Q1_state) decimal_value += 1; // Q1 represents 1
//
//      // Convert the decimal value to a string and send it over serial
//      String digit_string = String(decimal_value);
//
//      // LCD code, print the string of digits onto the LCD display
//
//      lcd.clear();
//      lcd.setCursor(0,0);
//      lcd.print(digit_string);
      lcd.setCursor(0, 1);
      //lcd.print(ESt_state);

      lcd.print(actionState);
      
      delay(100); // pause for 0.2 s (200 ms)

      

      // Speech Synthesis Chip code

      // digitalWrite(9, HIGH);
      // speak("[x0][t6][v8][s6][m51][g2][h2][n1]K 3 A U K Testing.");
      // delay(3000);
      // digitalWrite(9, LOW);
      // delay(5000);

      if (actionState == HIGH) {

        lcd.clear();
        lcd.setCursor(0,0);
        //lcd.print(digit_string);
        lcd.setCursor(0,1);
        lcd.print("Interrupt detected!");
        delay(2000); // pause for 2 s (2000 ms)

      }

      
}
