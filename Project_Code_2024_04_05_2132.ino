#include <LiquidCrystal_I2C.h> // library for LCD

using namespace std;
#include "String.h"

#include <EEPROM.h>

// Define char arrays to hold old password and password entered by the user

// Define Morse Code callsign
String morseCode;

String spkStr = "[x0][t6][v8][s6][m51][g2][h2][n1]";
String morseInstructions = "After the beep, enter your callsign in Morse code, using 1 for dot, 2 for dash, 3 for space, pound to clear, and star to finish.";

const int sphChpBaudRate = 4800;  //baud rate for speech chip

// Define Morse code timing parameters
const int dotDuration = 50; // in milliseconds
const int dashDuration = 3 * dotDuration;
const int interSymbolDelay = dotDuration;
const int interCharacterDelay = 3 * dotDuration;
const int morseCodeFreq = 700;  // Morse code frequency in Hz

// define pin names
const byte pwrLED = 13;  // white LED, always on with power
const byte rxLED = 12;  // blue LED, on when receiving
const byte txLED = 11;  // red LED, on when transmitting
const byte txRLY = 9;  // enable transmit relay
const byte estPin = 2;
const byte interruptPin = 5;
const byte pwmAudioPin = 6;

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
bool detectionState;  // keeps track of whether input audio was detected within the gateDelayTime
const unsigned long callsignDelayTime = 300000;  // number of milliseconds to delay after each callsign transmission  (5 mins = 300000 ms)
const int gateDelayTime = 1000;  // number of milliseconds to delay after each receiver detection
bool txCallsignState;  // keeps track of whether we are transmitting the callsign currently

int micThresh; // define microphone threshold for input detection

// action states for interrupts
volatile byte state = LOW;
volatile boolean actionState = LOW;

// variable for adjustable time delay
const int tdelayadjustable = 200;


volatile int decimal_value;

String digit_string;

// define variable for buffer

char buffer[bufferSize];

// the buffer position starts at 0, and it gets incremented every time a character is added to the buffer

int bufferPosition = 0;

// software reboot (used to reset the program and clean up RAM)
void software_reboot() {
  asm volatile (" jmp 0");
}

LiquidCrystal_I2C lcd(0x27,16,2);  // I2C address of LCD chip is 0x27. LCD is 16 chars and 2 lines.

void speak(char* msg) {
  Serial.write(0xFD);
  Serial.write((byte)0x0);
  Serial.write(2 + strlen(msg));
  Serial.write(0x01);
  Serial.write((byte)0x0);
  Serial.write(msg);
}

String DTMFget() {
  lcd.backlight();  // enable LCD backlight
        
        // clear the LCD screen
        lcd.clear();

        // wait the adjustable time delay
        delay(tdelayadjustable);

        // obtain states from DTMF chip binary outputs
        bool Q1_state = digitalRead(Q1_pin);
        bool Q2_state = digitalRead(Q2_pin);
        bool Q3_state = digitalRead(Q3_pin);
        bool Q4_state = digitalRead(Q4_pin);

        // Calculate the decimal value
        decimal_value = 0;
        if (Q4_state) decimal_value += 8; // Q4 represents 8
        if (Q3_state) decimal_value += 4; // Q3 represents 4
        if (Q2_state) decimal_value += 2; // Q2 represents 2
        if (Q1_state) decimal_value += 1; // Q1 represents 1

        // Convert the decimal value to a string
        digit_string = String(decimal_value);

        // overwrite string for the non-standard cases of 0, *, and #
        if(digit_string == "10") digit_string = "0";
        else if(digit_string == "11") digit_string = "*";
        else if(digit_string == "12") digit_string = "#";

        // print digit to LCD on bottom line
        lcd.setCursor(0, 1);
        lcd.print(digit_string);

        return digit_string;
}

void addCharToBuffer(String charToAdd) {

  if (charToAdd == "#") buffer_string = "";  // clear buffer string if # is entered
  if(buffer_string.length() > 36) buffer_string = "";  // clear buffer string if too long (>36 chars)

  buffer_string = buffer_string + charToAdd;  // append entered character to buffer string

}

void enableTX() { // enable transmission
  digitalWrite(txLED, HIGH);
  digitalWrite(txRLY, HIGH);
}

void disableTX() { // disable transmission
  digitalWrite(txLED, LOW);
  digitalWrite(txRLY, LOW);
}

void transmitMorseCode(const char* message) {
  for (int i = 0; message[i] != '\0'; i++) {
    switch (message[i]) {
      case '1':
        playDot();
        break;
      case '2':
        playDash();
        break;
      case '3':
        delay(interCharacterDelay);
        break;
    }
    delay(interSymbolDelay);
  }
}

void speakCallsign(String morse) {
  int charStart = 0;
  int charEnd;

  String morseCodeSpc = morse + "3";

  for(int i = 0; i < morseCodeSpc.length(); i++) {
    if (morseCodeSpc.charAt(i) == '3') {

      String mseToSpk;
      char charToSpk;

      charEnd = i;
      mseToSpk = morseCodeSpc.substring(charStart, charEnd);
      
      delay(250);

      charStart = i + 1;

      String letToSpk;
      
      if (mseToSpk == "12") letToSpk = "A";
      else if (mseToSpk == "2111") letToSpk = "B";
      else if (mseToSpk == "2121") letToSpk = "C";
      else if (mseToSpk == "211") letToSpk = "dee";  //D
      else if (mseToSpk == "1") letToSpk = "E";
      else if (mseToSpk == "1121") letToSpk = "F";
      else if (mseToSpk == "221") letToSpk = "G";
      else if (mseToSpk == "1111") letToSpk = "H";
      else if (mseToSpk == "11") letToSpk = "I";
      else if (mseToSpk == "1222") letToSpk = "J";
      else if (mseToSpk == "212") letToSpk = "K";
      else if (mseToSpk == "1211") letToSpk = "L";
      else if (mseToSpk == "22") letToSpk = "M";
      else if (mseToSpk == "21") letToSpk = "N";
      else if (mseToSpk == "222") letToSpk = "O";
      else if (mseToSpk == "1221") letToSpk = "P";
      else if (mseToSpk == "2212") letToSpk = "Q";
      else if (mseToSpk == "121") letToSpk = "R";
      else if (mseToSpk == "111") letToSpk = "S";
      else if (mseToSpk == "2") letToSpk = "T";
      else if (mseToSpk == "112") letToSpk = "you";  //U
      else if (mseToSpk == "1112") letToSpk = "V";
      else if (mseToSpk == "122") letToSpk = "W";
      else if (mseToSpk == "2112") letToSpk = "X";
      else if (mseToSpk == "2122") letToSpk = "why";  //Y
      else if (mseToSpk == "2211") letToSpk = "Z";
      else if (mseToSpk == "12222") letToSpk = "1";
      else if (mseToSpk == "11222") letToSpk = "2";
      else if (mseToSpk == "11122") letToSpk = "3";
      else if (mseToSpk == "11112") letToSpk = "4";
      else if (mseToSpk == "11111") letToSpk = "5";
      else if (mseToSpk == "21111") letToSpk = "6";
      else if (mseToSpk == "22111") letToSpk = "7";
      else if (mseToSpk == "22211") letToSpk = "8";
      else if (mseToSpk == "22221") letToSpk = "9";
      else if (mseToSpk == "22222") letToSpk = "0";

      speak((spkStr + letToSpk).c_str());

      
      //lcd.clear();
      //lcd.setCursor(0, 0);
      //lcd.print(i);
      //lcd.setCursor(0, 1);
      //lcd.print(String(charToSpk));
      //lcd.backlight();
      delay(500);
      //lcd.noBacklight();

      

    }
  }
}

void playDot() {
  tone(pwmAudioPin, morseCodeFreq, dotDuration);
  delay(dotDuration);
  noTone(pwmAudioPin);
}

void playDash() {
  tone(pwmAudioPin, morseCodeFreq, dashDuration);
  delay(dashDuration);
  noTone(pwmAudioPin);
}

// function to check if the buffer's contents is the correct sequence of characters for the change password command
bool checkContentsChangePassword(String s) {
    // check if the first character in the string is a #
    if (s[0] != '#') {
      return false;
    }

    // check if the next six characters are numbers from 0-9
    for (int i = 1; i <= 6; i++) {
      if (!isDigit(s[i])) {
        return false;
      }
    }

    if (s[7] != '*') {
      return false;
    }

    // check if the next six characters are numbers from 0-9
    for (int i = 8; i < 13; i++) {
      if (!isDigit(s[i])) {
        return false;
      }
    }

    if (s[14] != '*') {
      return false;
    }

    return true;   
}

// function to check if the buffer's contents is the correct sequence of characters for the change callsign command
bool checkContentsChangeCallsign(String s) {
    // check if the first character in the string is a #
    if (s[0] != '#') {
      return false;
    }

    // check if the next six characters are numbers from 0-9
    for (int i = 1; i <= 6; i++) {
      if (!isDigit(s[i])) {
        return false;
      }
    }

    if (s[7] != '*') {
      return false;
    }

    // check if the next three characters are the numbers 001
    if (s[8] != '0') {
      return false;
    }
    if (s[9] != '0') {
      return false;
    }
    if (s[10] != '1') {
      return false;
    }

    // check if the 11th character is a *

    if (s[11] != '*') {
      return false;
    }

    return true;     
}

// function to check if the buffer's contents is the correct sequence of characters for the transmit current callsign command
bool checkContentsTransmitCurrentCallsign(String s) {
  // check if the first character in the string is a #
  if (s[0] != '#') {
    return false;
  }


  // check if the next six characters are numbers from 0-9
  for (int i = 1; i <= 6; i++) {
    if (!isDigit(s[i])) {
      return false;
    }
  }

  if (s[7] != '*') {
    return false;
  }

  // check if the next three characters are the numbers 000
  if (s[8] != '0') {
    return false;
  }
  if (s[9] != '0') {
    return false;
  }
  if (s[10] != '0') {
    return false;
  }

  // check if the 11th character is a *

  if (s[11] != '*') {
    return false;
  }

  return true;   
}

void writeStringToEEPROM(int addrOffset, const String &strToWrite)
{
  byte len = strToWrite.length();
  EEPROM.write(addrOffset, len);
  for (int i = 0; i < len; i++)
  {
    EEPROM.write(addrOffset + 1 + i, strToWrite[i]);
  }
}

String readStringFromEEPROM(int addrOffset)
{
  int newStrLen = EEPROM.read(addrOffset);
  char data[newStrLen + 1];
  for (int i = 0; i < newStrLen; i++)
  {
    data[i] = EEPROM.read(addrOffset + 1 + i);
  }
  data[newStrLen] = '\0'; // !!! NOTE !!! Remove the space between the slash "/" and "0" (I've added a space because otherwise there is a display bug)
  return String(data);
}

void setup() {

  buffer_string.reserve(40);

  // setting the pinMode of the LED pins to OUTPUT
  pinMode(pwrLED, OUTPUT);
  pinMode(rxLED, OUTPUT);
  pinMode(txLED, OUTPUT);

  // leaving the power LED on all the time (when powered)
  digitalWrite(pwrLED, HIGH);

  // making the password 999999 for debugging purposes
  //writeStringToEEPROM(0, "999999");

  // set the Morse Code callsign to the callsign stored in the EEPROM non-volatile memory
  morseCode = readStringFromEEPROM(7);

  // write default password to EEPROM Memory
  // writeStringToEEPROM(0, "123456");

  // set input pins
  pinMode(A3, INPUT_PULLUP);  // Manual reset password button
  pinMode(3, INPUT);     // Q1 from DTMF chip
  pinMode(estPin, INPUT);     // ESt from DTMF chip
  pinMode(4, INPUT);     // Q2 from DTMF chip
  pinMode(7, INPUT);     // Q3 from DTMF chip
  pinMode(8, INPUT);     // Q4 from DTMF chip

  pinMode(9, OUTPUT);   // enable transmit relay
  pinMode(10, OUTPUT);  // audio source select relay (open = RX radio output, closed = speech chip)

  pinMode(interruptPin, INPUT_PULLUP);
  pinMode(pwmAudioPin, OUTPUT);

  // attach interrupt to speech synthesis chip pin
  //attachInterrupt(digitalPinToInterrupt(interruptPin), busy, CHANGE);

  // attach interrupt to ESt pin for DTMF decoding
  attachInterrupt(digitalPinToInterrupt(estPin), decodeDTMF, RISING);

  
  Serial.begin(sphChpBaudRate);
  
  // set output pins
  //pinMode(6, OUTPUT);    // Speaker
  //pinMode(7, OUTPUT);    // GREEN LED indicator
  //pinMode(8, OUTPUT);    // RED LED indicator

  // LCD setup
  lcd.init();
  lcd.clear();

  lastCallsignTX = 0;  // keeps track of time of last callsign transmission
  lastDetectionTime = 0;  // keeps track of last time audio from receiver radio is detected
  detectionState = false;  // keeps track of whether input audio was detected within the gateDelayTime
  txCallsignState = false;  // keeps track of whether we are transmitting the callsign currently

  micThresh = 600; // define analog threshold for detecting audio input
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
      // check if reset PIN button is depressed
      if(digitalRead(A3) == 0)
      {
        lcd.backlight();  // enable LCD backlight
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Hold for 3 s");
        lcd.setCursor(0, 1);
        lcd.print("To reset PWD");

        unsigned int counter = 0;

        delay(100);  // digital debounce

        while(digitalRead(A3) == 0)
        {
          delay(1);
          counter++;
          if(counter == 3000)
          {
            writeStringToEEPROM(0, "123456");
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Password reset");
            lcd.setCursor(0, 1);
            lcd.print("to 123456");
            delay(3000);
          }
        }

        delay(1000);
        lcd.clear();

        software_reboot();  // reboot to clean up RAM

      }

      // detect audio input from receiver radio
      int micIn = analogRead(0);  // read pin coming from receiver radio audio output
      if(micIn > micThresh) {  // if pin value greater than threshold
        digitalWrite(rxLED, HIGH);  // turn on RX indicator LED
        enableTX();  // enable transmission
        lastDetectionTime = millis();  // set last detection time
        detectionState = true;  // set detection state to TRUE
      }
      
      // disable transmission after gate delay time
      if (( millis() < lastDetectionTime) or ( millis() > lastDetectionTime + gateDelayTime)) {  //first conditional captures overflow case with millis
        lcd.noBacklight();  // disable LCD backlight
        digitalWrite(rxLED, LOW);  // turn off RX indicator LED
        disableTX();  //disable transmission
        detectionState = false;  // set detection state to FALSE
      }

      // DTMF tone interrupt routine (the interrupt sets actionState to HIGH)
      if (actionState == HIGH) {

        // add the digit to the buffer
        addCharToBuffer(DTMFget());

        // print buffer contents to LCD on top line
        lcd.setCursor(0, 0);
        lcd.print(buffer_string);
        
        actionState = LOW;  // reset actionState to LOW

      }

      
      // if buffer string length is greater than 12, a command could have been issued
      if (buffer_string.length() >= 12) {

        bool correctPWD = false;

        String old_password_entry = buffer_string.substring(1, 7);

        String old_password = readStringFromEEPROM(0);

        correctPWD = (old_password.compareTo(old_password_entry) == 0);
        // lcd.clear();
        // we have a string saved in the bufferString variable that is long enough to possibly be a transmit current callsign command or a change callsign command

        // check if command was to transmit current callsign (#XXXXXX*000*)
        bool isTransmitCurrentCallsign = false;
        isTransmitCurrentCallsign = checkContentsTransmitCurrentCallsign(buffer_string);
        // lcd.print(isTransmitCurrentCallsign);
        // delay(3000);
        
        // check if command was to change callsign (#XXXXXX*001*)
        bool isChangeCallsign = false;
        isChangeCallsign = checkContentsChangeCallsign(buffer_string);

        // check if password is correct
        if (correctPWD) {
          if (isTransmitCurrentCallsign) {
            // passwords ARE equal!
            lcd.backlight();  // enable LCD backlight

            //Speech Synthesis Chip code
            enableTX();
            digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
            delay(1000);
            // once we have the Morse code string, we will play the Morse code back through the PWM pin on the ATMega
            transmitMorseCode(morseCode.c_str());

            speakCallsign(morseCode);

            delay(500);

            //Speech Synthesis Chip code
            disableTX();
            digitalWrite(10, LOW);  // switch audio back to input radio (open relay 2)

            // clear buffer so transmit current callsign command doesn't automatically repeat
            addCharToBuffer("#");
          }
        
          if (isChangeCallsign) {

            // check if characters 1-6 matches the old password stored in the EEPROM

            lcd.backlight();
            lcd.clear();
            lcd.print("change callsign");
            lcd.setCursor(0,1);
            lcd.print("process begin");

            delay(1500);

            // starting process of callsign changing

            // prompt the user with speech prompts

              //Speech Synthesis Chip code
              enableTX();  // enable transmission on TX radio (close relay 1)
              digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
              
              speak((spkStr + morseInstructions).c_str());
              lcd.clear();
              lcd.print("Enter callsign");
              delay(5000);

              lcd.clear();              
              lcd.print("1:dot 2:dash");
              lcd.setCursor(0, 1);
              lcd.print("3:space *:finish");
              delay(12000);
              transmitMorseCode("2");

              //Speech Synthesis Chip code
              disableTX();  // disable transmission
              digitalWrite(10, LOW);  // switch audio source back to radio input (open relay 2)

              delay(1000);

              addCharToBuffer("#");

            unsigned int timeOutCnt1 = 0;  // time out counter

            while (1) {
          
              if (actionState == HIGH) {
                timeOutCnt1 = 0;  // reset time out counter

                addCharToBuffer(DTMFget());

                // print buffer contents to LCD on top line
                lcd.setCursor(0, 0);
                
                if(buffer_string.length() > 16){
                  lcd.print(buffer_string.substring(buffer_string.length() - 16));
                }
                else{
                  lcd.print(buffer_string);
                }
                
                actionState = LOW;  // reset actionState to LOW

              }

              // user enters the callsign in Morse code into the buffer that has now been cleared
              // keep on checking to see if a * has been entered by the user
              // check if the substring * exists within the bufferString

              // if it exists, the user has ended, and the user has finished entering their callsign in Morse code
              // now, it is time to 1) play the digits back through the speech chip and 2) play the Morse code back through the PWM pin on the ATMega
              if (buffer_string.indexOf("*") != -1) {
                // play the digits back through the speech chip

                // remove # if user entered #
                if(buffer_string.substring(0,1) == "#") {
                  buffer_string = buffer_string.substring(1);
                }

                // remove *
                buffer_string.remove(buffer_string.length() - 1);

                //Speech Synthesis Chip code
                enableTX();  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                delay(1000);
                speak((spkStr + "To confirm, the digits are").c_str());
                delay(5500);

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("Callsign digits:");
                lcd.setCursor(0, 1);

                for (int i = 0; i < buffer_string.length(); i++) {

                  if ( (i % 15) == 0 ) lcd.clear();  // clear LCD if 16 digits have already been displayed

                  speak(String(buffer_string.charAt(i)).c_str());  // speak digits representing Morse code
                  lcd.print(buffer_string.charAt(i));  // print Morse code digits on lcd

                  delay(1000);
                }

                //Speech Synthesis Chip code
                delay(1000);

                speak((spkStr + "The Morse code is").c_str());

                delay(4000);

                String morse_code_string = buffer_string;

                //Speech Synthesis Chip code
                delay(200);
                // once we have the Morse code string, we will play the Morse code back through the PWM pin on the ATMega
                transmitMorseCode(morse_code_string.c_str());

                // print the morse code on the LCD display

                delay(1000);

                speak((spkStr + "The characters are").c_str());
                delay(3500);
                speakCallsign(morse_code_string);

                delay(1000);

                // clearing buffer
                buffer_string = "";

                // prompt user
                speak((spkStr + "Press star to confirm, pound to cancel").c_str());

                lcd.clear();
                lcd.setCursor(0, 0);
                lcd.print("* Confirm");
                lcd.setCursor(0, 1);
                lcd.print("# Cancel");

                delay(7500);

                disableTX();  // disable transmission
                digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                unsigned int timeOutCnt2 = 0;  // time out counter

                while (1) {

                  // DTMF tone interrupt routine (the interrupt sets actionState to HIGH)
                  if (actionState == HIGH) {

                    // add the digit to the buffer
                    addCharToBuffer(DTMFget());

                    // print buffer contents to LCD on top line
                    lcd.setCursor(0, 0);
                    lcd.print(buffer_string);
                    
                    actionState = LOW;  // reset actionState to LOW

                  }
                  
                  // if confirm selected (user enters "*")
                  if (buffer_string.indexOf("*") != -1) {

                    //Speech Synthesis Chip code
                    enableTX();  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
                    
                    delay(2000);
                    speak((spkStr + "Callsign confirmed.  Saving to memory.").c_str());
                    // speak("[x0][t6][v8][s6][m51][g2][h2][n1]Callsign confirmed.  Saving to memory.");

                    lcd.clear();
                    lcd.print("Callsign confirmed");
                    lcd.setCursor(0, 1);
                    lcd.print("Saving to memory");

                    // writing string to non-volatile memory
                    writeStringToEEPROM(7, morse_code_string);

                    // updating the callsign string variable
                    morseCode = readStringFromEEPROM(7);

                    delay(6000);

                    //Speech Synthesis Chip code
                    disableTX();  // disable transmission
                    digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)


                    

                    delay(1000);

                    lcd.clear();

                    // writing string to non-volatile memory
                    
                    software_reboot();  // reboot to break out of loops and clean up RAM

                  }

                  // if cancel selected (user enters "#") or time out condition occurs
                  if ((buffer_string.indexOf("#") != -1) or (timeOutCnt2 > 20000)) {

                    //Speech Synthesis Chip code
                    enableTX();  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                    delay(2000);
                    speak((spkStr + "Canceling").c_str());
                    // speak("[x0][t6][v8][s6][m51][g2][h2][n1]Canceling");
                    delay(4000);

                    //Speech Synthesis Chip code
                    disableTX();  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                    lcd.clear();
                    lcd.print("Canceling");

                    delay(3000);

                    lcd.clear();

                    software_reboot();  // reboot to break out of loops and clean up RAM
                    
                  }

                  delay(1);  // delay 1 ms
                  timeOutCnt2++;  // increase time out counter

                }
              }

              delay(1);  // delay by 1 ms
              timeOutCnt1++;  // increment time out counter

              // if time out condition occurs
              if(timeOutCnt1 > 20000) {  // 20 s elapsed with no input
                enableTX();  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                delay(1000);
                speak((spkStr + "Time out").c_str());
                delay(2000);

                disableTX();  // disable transmission
                digitalWrite(10, LOW);  // switch audio source back to radio input (open relay 2)

                software_reboot();  // reboot to break out of loops and clean up RAM
              }
              
            }
          }

          if (buffer_string.length() >= 15) {
            // we have a string saved in the bufferString variable that is long enough to possibly be a change password command
            bool isChangePassword = false;
            isChangePassword = checkContentsChangePassword(buffer_string);

            if (isChangePassword) {
              // passwords ARE equal!
              // writes to EEPROM when sequence is detected and the old password in the buffer string matches the old password in the EEPROM memory
              String new_password = buffer_string.substring(8, 14);

              writeStringToEEPROM(0, new_password);

              delay(200);
              lcd.clear();
              lcd.setCursor(0, 0);
              lcd.print("Passwords Equal!");
              lcd.setCursor(0, 1);
              lcd.print("PWD Changed!");

              // TRANSMIT REASSURANCE RESPONSE ("PASSWORD CHANGE SUCCESSFUL")
              enableTX();  // enable transmission on TX radio (close relay 1)
              digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
              delay(500);
              speak((spkStr + "Password change successful").c_str());
              delay(5000);
              disableTX();  // disable transmission
              digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

              software_reboot();  // reboot to clean up RAM
            }

            buffer_string = "";

          } // end if buffer_string.length() >= 14
        } //end if correctPWD
      } // end ifbuffer_string.length() >= 12

      // transmit callsign at specified interval
      if (( millis() < lastCallsignTX) or ( millis() > lastCallsignTX + callsignDelayTime))  //first conditional captures overflow case with millis
      {
        lastCallsignTX = millis();  //capture current time

        // txCallsignState = true;  // set transmit callsign state to TRUE

        //Speech Synthesis Chip code
        enableTX();  // enable transmission on TX radio (close relay 1)
        digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

        //speak("[x0][t6][v8][s6][m51][g2][h2][n1]K 3 A U K");  // send command to speech chip to say callsign
        delay(1000);
        transmitMorseCode(morseCode.c_str());
        delay(250);
        speakCallsign(morseCode);
        delay(500);
        digitalWrite(10, LOW);  // switch audio source back to input radio
      }

}

// decodeDTMF ISR

void decodeDTMF() {
//      // change action state of interrupt for decodeDTMF
        actionState = !actionState;
}
