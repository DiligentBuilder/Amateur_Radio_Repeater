#include <LiquidCrystal_I2C.h> // library for LCD

#include "phoneticAlphabet.h"

using namespace std;
#include "String.h"


const byte ledPin = 13;
const byte estPin = 2;
const byte interruptPin = 5;

const int bufferSize = 10;
const int bufferSizePlusOne = bufferSize + 1;

// defining constants for Q digital pin numbers
const byte Q1_pin = 3;
const byte Q2_pin = 4;
const byte Q3_pin = 7;
const byte Q4_pin = 8;
String buffer_string = "";

unsigned long lastCallsignTX;  // keeps track of time of last callsign transmission
unsigned long lastDetectionTime;  // keeps track of last time audio from receiver radio is detected
const int callsignDelayTime = 30000;  // number of milliseconds to delay after each callsign transmission
const int gateDelayTime = 1000;  // number of milliseconds to delay after each receiver detection

int micThresh; // define microphone threshold for input detection

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

String digit_string;

// define variable for buffer

char buffer[bufferSize];

// the buffer position starts at 0, and it gets incremented every time a character is added to the buffer

int bufferPosition = 0;


// fifteen second counter
// this counter starts at 0 and goes until the count is high enough for 15 seconds
// this counter is for the refreshing of the buffer to clear itself after around 
// 15 seconds to "forget" the user input after a certain amount of time
int fifteenSecondCounter = 0;







LiquidCrystal_I2C lcd(0x27,16,2);  // I2C address of LCD chip is 0x27. LCD is 16 chars and 2 lines.

void speak(char* msg) {
  Serial.write(0xFD);
  Serial.write((byte)0x0);
  Serial.write(2 + strlen(msg));
  Serial.write(0x01);
  Serial.write((byte)0x0);
  Serial.write(msg);
}

// busy ISR

void busy() {
  state = !state;
  digitalWrite(ledPin, state);
}

// functions to implement the buffer
// buffer needs to be cleared 1) after every certain amount of time, and 2) every time the user enters a "#" sign
void clearBuffer() {
  for (int i = 1; i < bufferSize; i++) {
    buffer[i] = '-'; // dash will be the empty character for a character in the buffer

    // reset the buffer position to 0
    bufferPosition = 0;



  }

}

void addCharToBuffer(String charToAdd) {
  // // reset the fifteen second delay counter to 0
  // fifteenSecondCounter = 0;

  // if (charToAdd == "#") {
  //   clearBuffer();
  //   buffer[bufferPosition] = "#";
    
  // }
  // else {
  //   buffer[bufferPosition] = charToAdd;
  // }

  // // increment the buffer position
  // bufferPosition++;
  if (charToAdd == "#") buffer_string = "";
  buffer_string = buffer_string + charToAdd;

  //if (size(buffer_string) > 15)
  //  String.erase(String.begin(), String.begin()+1);

}



void setup() {
  // set input pins
  pinMode(3, INPUT);     // Q1 from DTMF chip
  pinMode(estPin, INPUT);     // ESt from DTMF chip
  pinMode(4, INPUT);     // Q2 from DTMF chip
  pinMode(7, INPUT);     // Q3 from DTMF chip
  pinMode(8, INPUT);     // Q4 from DTMF chip

  pinMode(9, OUTPUT);   // enable transmit relay
  pinMode(10, OUTPUT);  // audio source select relay (open = RX radio output, closed = speech chip)

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

  lastCallsignTX = 0;  // keeps track of time of last callsign transmission

  lastDetectionTime = 0;  // keeps track of last time audio from receiver radio is detected

  micThresh = 600; // define microphone threshold
}

String returnBufferContents() {

  char bufferPlusTerminator[bufferSizePlusOne];

  for (int i = 0; i < bufferSize; i++) {
    bufferPlusTerminator[i] = buffer[i];
  }
  
  bufferPlusTerminator[bufferSize] = '\0';

  return String(bufferPlusTerminator);

}

void loop() {

      // detect audio input
      int micIn = analogRead(0);
      if(micIn > micThresh) {
        digitalWrite(9, HIGH);
        lastDetectionTime = millis();
      }
      
      if (( millis() < lastDetectionTime) or ( millis() > lastDetectionTime + gateDelayTime)) {  //first conditional captures overflow case with millis
        digitalWrite(9, LOW);
      }

      lcd.setCursor(0, 1);
      //lcd.print(ESt_state);

      lcd.print("--");
      
      delay(100); // pause for 0.2 s (200 ms)

      

      

      if (actionState == HIGH) {

        lcd.clear();
        
        

        

        // DTMF Decoding Code

        // wait the adjustable time delay
        delay(tdelayadjustable);
     

        //bool ESt_state = digitalRead(3);

        Q1_state = digitalRead(Q1_pin);
        Q2_state = digitalRead(Q2_pin);
        Q3_state = digitalRead(Q3_pin);
        Q4_state = digitalRead(Q4_pin);

        // Calculate the decimal value
        decimal_value = 0;
        if (Q4_state) decimal_value += 8; // Q4 represents 8
        if (Q3_state) decimal_value += 4; // Q3 represents 4
        if (Q2_state) decimal_value += 2; // Q2 represents 2
        if (Q1_state) decimal_value += 1; // Q1 represents 1

        // Convert the decimal value to a string and send it over serial
        digit_string = String(decimal_value);

        if(digit_string == "10") digit_string = "0";
        else if(digit_string == "11") digit_string = "*";
        else if(digit_string == "12") digit_string = "#";

        // LCD code, print the string of digits onto the LCD display
        
        lcd.setCursor(0,1);
        lcd.print(digit_string);

        // add the digit as a CHAR onto the buffer
        addCharToBuffer(digit_string);

        lcd.setCursor(0,0);
        lcd.print(buffer_string);
        
        actionState = LOW;

      }

      // increment the fifteen second counter by one
      fifteenSecondCounter++;

      // check if the fifteen seconds has passed yet
      // loop delays by 100 ms each time, so the time has reached 15 seconds when the count has reached 150
      if (fifteenSecondCounter >= 150) {
        fifteenSecondCounter = 0;
        clearBuffer();
        
      }

      // transmit callsign at specified interval
      if (( millis() < lastCallsignTX) or ( millis() > lastCallsignTX + callsignDelayTime))  //first conditional captures overflow case with millis
      {
        lastCallsignTX = millis();  //capture current time

        //Speech Synthesis Chip code
        digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
        digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

        speak("[x0][t6][v8][s6][m51][g2][h2][n1]K 3 A U K Testing.");  // send command to speech chip to say callsign
        delay(3000);  // delay 3 seconds as the callsign is read

        // open both relays
        digitalWrite(9, LOW);
        digitalWrite(10, LOW);

      }
}

// decodeDTMF ISR

void decodeDTMF() {
//      // change action state of interrupt for decodeDTMF
        actionState = !actionState;
//      
//      

      
}
