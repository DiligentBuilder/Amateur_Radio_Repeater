#include <LiquidCrystal_I2C.h> // library for LCD

//#include "phoneticAlphabet.h"

using namespace std;
#include "String.h"

#include <EEPROM.h>

// Define char arrays to hold old password and password entered by the user

char old_password_chars_change_callsign[6];

char old_password_entry_chars_change_callsign[6];

// Define Morse Code callsign
String morseCode = "-.- ...-- .- ..- -.-";

const int sphChpBaudRate = 4800;  //baud rate for speech chip

// Define Morse code timing parameters
const int dotDuration = 50; // in milliseconds
const int dashDuration = 3 * dotDuration;
const int interSymbolDelay = dotDuration;
const int interCharacterDelay = 3 * dotDuration;
const int interWordDelay = 4 * dotDuration;
const int morseCodeFreq = 700;  // Morse code frequency in Hz


const byte ledPin = 13;
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

// making a new string to store characters for the new callsign for the change callsign command
String new_callsign_string;

unsigned long lastCallsignTX;  // keeps track of time of last callsign transmission
unsigned long lastDetectionTime;  // keeps track of last time audio from receiver radio is detected
bool detectionState;  // keeps track of whether input audio was detected within the gateDelayTime
const int callsignDelayTime = 300000;  // number of milliseconds to delay after each callsign transmission  (5 mins = 300000 ms)
const int gateDelayTime = 1000;  // number of milliseconds to delay after each receiver detection
bool txCallsignState;  // keeps track of whether we are transmitting the callsign currently

int micThresh; // define microphone threshold for input detection

// action states for interrupts
volatile byte state = LOW;
volatile boolean actionState = LOW;

// variable for adjustable time delay
const int tdelayadjustable = 200;

String old_password_entry = "      ";
String old_password = "      ";
String new_password = "      ";

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

void addCharToNewCallsignString(String charToAdd) {
  if (charToAdd == "#") new_callsign_string = "";

  new_callsign_string = new_callsign_string + charToAdd;
   
}

void transmitMorseCode(const char* message) {
  for (int i = 0; message[i] != '\0'; i++) {
    switch (message[i]) {
      case '.':
        playDot();
        break;
      case '-':
        playDash();
        break;
      case ' ':
        delay(interWordDelay);
        break;
    }
    delay(interSymbolDelay);
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

  // setting the pinMode of the LED pins to OUTPUT
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);

  // leaving the LEDs on all the time
  digitalWrite(11, HIGH);
  digitalWrite(12, HIGH);
  digitalWrite(13, HIGH);

  

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

  pinMode(ledPin, OUTPUT);
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
  lcd.backlight();      // Make sure backlight is on

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

      }

      // detect audio input from transmitter radio
      int micIn = analogRead(0);  // read pin coming from transmitter radio audio output
      if(micIn > micThresh) {  // if pin value greater than threshold
        digitalWrite(9, HIGH);  // close transmit relay (enable transmission)
        lastDetectionTime = millis();  // set last detection time
        detectionState = true;  // set detection state to TRUE
      }
      
      // disable transmission after gate delay time
      if (( millis() < lastDetectionTime) or ( millis() > lastDetectionTime + gateDelayTime)) {  //first conditional captures overflow case with millis
        digitalWrite(9, LOW);  // open transmit relay (disable transmission)
        detectionState = false;  // set detection state to FALSE
      }

      // DTMF tone interrupt routine (the interrupt sets actionState to HIGH)
      if (actionState == HIGH) {
        
        // clear the LCD screen
        lcd.clear();

        // wait the adjustable time delay
        delay(tdelayadjustable);

        // obtain states from DTMF chip binary outputs
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

        // Convert the decimal value to a string
        digit_string = String(decimal_value);

        // overwrite string for the non-standard cases of 0, *, and #
        if(digit_string == "10") digit_string = "0";
        else if(digit_string == "11") digit_string = "*";
        else if(digit_string == "12") digit_string = "#";

        // print digit to LCD on bottom line
        lcd.setCursor(0, 1);
        lcd.print(digit_string);

        // add the digit to the buffer
        addCharToBuffer(digit_string);

        // print buffer contents to LCD on top line
        lcd.setCursor(0, 0);
        lcd.print(buffer_string);
        
        actionState = LOW;  // reset actionState to LOW

      }

      // code to check for the change callsign command
      // when the buffer string reaches 12 characters, check to see if a change callsign command was entered

      // check the first twelve characters to see if it is in the correct format for a change callsign command
      // format for change callsign command: # x x x x x x * 0 0 1 * 

      if (buffer_string.length() >= 12) {
        // lcd.clear();
        // we have a string saved in the bufferString variable that is long enough to possibly be a transmit current callsign command or a change callsign command

        bool isTransmitCurrentCallsign = false;
        isTransmitCurrentCallsign = checkContentsTransmitCurrentCallsign(buffer_string);
        // lcd.print(isTransmitCurrentCallsign);
        // delay(3000);

        if (isTransmitCurrentCallsign) {

          lcd.clear();
          lcd.print("a");
          delay(3000);

          // check if characters 1-6 matches the old password stored in the EEPROM

          old_password_entry = buffer_string.substring(1, 7);

          old_password = readStringFromEEPROM(0);

          // check if passwords are equal
          if (old_password.compareTo(old_password_entry) == 0) {
            // passwords ARE equal!

            lcd.clear();
            lcd.print("Passwords equal ");
            lcd.setCursor(0, 1);
            lcd.print("for transmit cmd");
            delay(3000);

            //Speech Synthesis Chip code
            digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
            digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
            delay(200);
            // once we have the Morse code string, we will play the Morse code back through the PWM pin on the ATMega
            transmitMorseCode(morseCode.c_str());

            //Speech Synthesis Chip code
            digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
            digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)


            lcd.clear();
            lcd.print("Current Callsign: ");
            lcd.setCursor(0, 1);
            lcd.print(String(morseCode));
            delay(3000);

            // clear LCD display to declutter
            lcd.clear();

            


            // clear buffer so transmit current callsign command doesn't automatically repeat
            addCharToBuffer("#");

            









          }
        }
        


        bool isChangeCallsign = false;
        isChangeCallsign = checkContentsChangeCallsign(buffer_string);
        //lcd.print(isChangeCallsign);



        if (isChangeCallsign) {

          lcd.clear();

          lcd.print("x");


          // check if characters 1-6 matches the old password stored in the EEPROM

          old_password_entry = buffer_string.substring(1, 7);

          old_password = readStringFromEEPROM(0);


          
          // old_password_entry.toCharArray(old_password_entry_chars_change_callsign, 7);

          
          
          // old_password.toCharArray(old_password_chars_change_callsign, 7);

          
          // lcd.clear();
          // lcd.print(old_password_chars_change_callsign);
          // delay(5000);

          

          // lcd.clear();
          // lcd.print(old_password_entry_chars_change_callsign);
          // delay(5000);

          // lcd.clear();
          // lcd.print(old_password_entry);
          // lcd.print(old_password);

          // delay(5000);

          // lcd.clear();
          // lcd.setCursor(0,0);
          // lcd.print("Old: ");
          // lcd.print(old_password);
          // lcd.print(" ");
          // lcd.print(old_password.length());
          // delay(5000);

          // lcd.setCursor(0,1);
          // lcd.print("Ent: ");
          // lcd.print(old_password_entry);
          // lcd.print(" ");
          // lcd.print(old_password_entry.length());
          // delay(10000);
          // lcd.clear();

          // lcd.print(old_password_entry_chars_change_callsign);

          // delay(3000);

          // lcd.clear();

          // lcd.print(old_password_chars_change_callsign);

          // delay(3000);




          // lcd.clear();
          // lcd.print(strncmp(old_password_entry_chars_change_callsign, old_password_chars_change_callsign, 6));

          // delay(5000);

          // check if passwords are equal
          if (old_password.compareTo(old_password_entry) == 0) {
            // passwords ARE equal!

            lcd.clear();
            lcd.print("passwords equal");

            delay(3000);

            // starting process of callsign changing

            bool breakVar = 0;

            // clearing buffer
            // addCharToBuffer("#");

            // prompt the user with speech prompts

              //Speech Synthesis Chip code
              digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
              digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
              
              speak("[x0][t6][v8][s6][m51][g2][h2][n1]Enter your callsign in Morse code, using 1 for dot, 2 for dash, 3 for space, and star to end. ");
              delay(15000);

              //Speech Synthesis Chip code
              digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
              digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

              lcd.clear();
              lcd.print("Enter callsign");
              delay(3000);

              lcd.clear();              
              lcd.print("1 dot 2 dash 3 space");
              lcd.setCursor(0, 1);
              lcd.print("# to start over");

              delay(1000);



            


            while (1) {
           

            
             

              

              if (actionState == HIGH) {
        
                // clear the LCD screen
                lcd.clear();

                // wait the adjustable time delay
                delay(tdelayadjustable);

                // obtain states from DTMF chip binary outputs
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

                // Convert the decimal value to a string
                digit_string = String(decimal_value);

                // overwrite string for the non-standard cases of 0, *, and #
                if(digit_string == "10") digit_string = "0";
                else if(digit_string == "11") digit_string = "*";
                else if(digit_string == "12") digit_string = "#";

                // print digit to LCD on bottom line
                lcd.setCursor(0, 1);
                lcd.print(digit_string);

                // add the digit to the buffer
                addCharToNewCallsignString(digit_string);

                // print buffer contents to LCD on top line
                lcd.setCursor(0, 0);
                lcd.print(new_callsign_string);
                
                actionState = LOW;  // reset actionState to LOW

              }

              lcd.clear();
              lcd.print(new_callsign_string);
              //delay(5000);

              //lcd.clear();
              //lcd.print(buffer_string.indexOf("*"));
              //delay(5000);




              // user enters the callsign in Morse code into the buffer that has now been cleared

              // keep on checking to see if a * has been entered by the user

              // check if the substring * exists within the bufferString

              // if it exists, the user has ended, and the user has finished entering their callsign in Morse code
              // now, it is time to 1) play the digits back through the speech chip and 2) play the Morse code back through the PWM pin on the ATMega
              if (new_callsign_string.indexOf("*") != -1) {
                // play the digits back through the speech chip

                //Speech Synthesis Chip code
                digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                speak("[x0][t6][v8][s6][m51][g2][h2][n1]To confirm, the digits of your callsign that were detected are: ");
                delay(10000);

                //Speech Synthesis Chip code
                digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                lcd.clear();
                lcd.print("Callsign digits:");

                delay(3000);

                lcd.setCursor(0, 1);
                

                //Speech Synthesis Chip code
                digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                for (int i = 0; i < new_callsign_string.indexOf("*"); i++) {

                  


                  speak(String(new_callsign_string.charAt(i)).c_str());
                  lcd.print(new_callsign_string.charAt(i));
                  
                  delay(1000);

                 
                  
                }

                //Speech Synthesis Chip code
                digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                
                

                delay(3000);

                //Speech Synthesis Chip code
                digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                speak("[x0][t6][v8][s6][m51][g2][h2][n1]To confirm, the Morse code of your callsign that was detected is: ");

                delay(10000);

                //Speech Synthesis Chip code
                digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)


                lcd.clear();
                lcd.print("Callsign Morse code:");

                String morse_code_string = "";

                for (int i = 0; i < new_callsign_string.indexOf("*"); i++) {
                  if (new_callsign_string.charAt(i) == '1') {
                    morse_code_string += ".";
                  }

                  if (new_callsign_string.charAt(i) == '2') {
                    morse_code_string += "-";
                  }

                  if (new_callsign_string.charAt(i) == '3') {
                    morse_code_string += " ";
                  }
                }

                //Speech Synthesis Chip code
                digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
                delay(200);
                // once we have the Morse code string, we will play the Morse code back through the PWM pin on the ATMega
                transmitMorseCode(morse_code_string.c_str());

                //Speech Synthesis Chip code
                digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                // print the morse code on the LCD display
                lcd.setCursor(0, 1);
                lcd.print(morse_code_string);

                delay(3000);



                // clearing buffer
                addCharToBuffer("#");


                // prompt user

                //Speech Synthesis Chip code
                digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                speak("[x0][t6][v8][s6][m51][g2][h2][n1]Press star to confirm, Press 1 to retry, Press 0 to cancel");

                delay(15000);

                //Speech Synthesis Chip code
                digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                lcd.clear();
                lcd.print("* Confirm, 1 Retry, ");
                lcd.setCursor(0, 1);
                lcd.print("0 Cancel");

                delay(3000);



   

                while (1) {

                  // DTMF tone interrupt routine (the interrupt sets actionState to HIGH)
                  if (actionState == HIGH) {
                    
                    // clear the LCD screen
                    lcd.clear();

                    // wait the adjustable time delay
                    delay(tdelayadjustable);

                    // obtain states from DTMF chip binary outputs
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

                    // Convert the decimal value to a string
                    digit_string = String(decimal_value);

                    // overwrite string for the non-standard cases of 0, *, and #
                    if(digit_string == "10") digit_string = "0";
                    else if(digit_string == "11") digit_string = "*";
                    else if(digit_string == "12") digit_string = "#";

                    // print digit to LCD on bottom line
                    lcd.setCursor(0, 1);
                    lcd.print(digit_string);

                    // add the digit to the buffer
                    addCharToBuffer(digit_string);

                    // print buffer contents to LCD on top line
                    lcd.setCursor(0, 0);
                    lcd.print(buffer_string);
                    
                    actionState = LOW;  // reset actionState to LOW

                  }
                  

                  if (buffer_string.indexOf("*") != -1) {

                    //Speech Synthesis Chip code
                    digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
                    
                    speak("[x0][t6][v8][s6][m51][g2][h2][n1]Committing Morse Code to EEProm");

                    delay(6000);

                    //Speech Synthesis Chip code
                    digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)


                    lcd.clear();
                    lcd.print("Committing Morse ");
                    lcd.setCursor(0, 1);
                    lcd.print("Code to EEProm");

                    delay(3000);

                    lcd.clear();

                    // writing string to non-volatile memory
                    writeStringToEEPROM(7, morse_code_string);

                    // updating the callsign string variable
                    morseCode = readStringFromEEPROM(7);

                    

                    // clear the buffer
                    addCharToBuffer("#");

                    // clear the new_callsign_string
                    new_callsign_string = "";

                    // we are done now
                    breakVar = 1;

                    // break out of the infinite loop
                    break;

                  }

                  if (buffer_string.indexOf("1") != -1) {

                    //Speech Synthesis Chip code
                    digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                    speak("[x0][t6][v8][s6][m51][g2][h2][n1]Retrying");
                    delay(4000);

                    //Speech Synthesis Chip code
                    digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                    lcd.clear();
                    lcd.print("Retrying");

                    delay(3000);

                    lcd.clear();

                    // clear the buffer
                    // addCharToBuffer("#");

                    // clear the new_callsign_string
                    new_callsign_string = "";


                    // no break, will continue looping

                    // do not set breakVar = 1

                    // break out of the infinite loop
                    break;

                    
                  }

                  if (buffer_string.indexOf("0") != -1) {

                    //Speech Synthesis Chip code
                    digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

                    speak("[x0][t6][v8][s6][m51][g2][h2][n1]Canceling");
                    delay(4000);

                    //Speech Synthesis Chip code
                    digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
                    digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

                    lcd.clear();
                    lcd.print("Canceling");

                    delay(3000);

                    lcd.clear();

                    // clear the buffer
                    addCharToBuffer("#");

                    // clear the new_callsign_string
                    new_callsign_string = "";

                    
                    

                    // break to cancel and get out of the loop
                    breakVar = 1;

                    // break out of the infinite loop
                    break;
                    
                  }

                
                }

                // break if the breakVar = 1
                if (breakVar == 1) {
                  break;
                }
                
              }

              
            }
          }

           
        }
      }

      // code to change password when the buffer string is of the correct length and has the correct format
      // check if the buffer string is fourteen characters long or longer, if so, 
      // check the first fourteen characters to see if it is in the correct format for a change password command

      if (buffer_string.length() >= 14) {
        // we have a string saved in the bufferString variable that is long enough to possibly be a change password command
        bool isChangePassword = false;
        isChangePassword = checkContentsChangePassword(buffer_string);

        if (isChangePassword) {
          // check if characters 1-6 matches the old password stored in the EEPROM
          
          old_password_entry = buffer_string.substring(1, 7);

          new_password = buffer_string.substring(8, 14);
          
          old_password = readStringFromEEPROM(0);

          // if (strcmp(buffer_string_substring_char_array, old_password_char_array) == 0) {
          if (old_password_entry == old_password) {
            // passwords ARE equal!
            // writes to EEPROM when sequence is detected and the old password in the buffer string matches the old password in the EEPROM memory
            writeStringToEEPROM(0, new_password);

            delay(200);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("Passwords Equal!");
            lcd.setCursor(0, 1);
            lcd.print("PWD Changed!");

            // TRANSMIT REASSURANCE RESPONSE ("PASSWORD CHANGE SUCCESSFUL")
            digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
            digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
            delay(500);
            speak("[x0][t6][v8][s6][m51][g2][h2][n1]Password change successful.");
            delay(5000);
            digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
            digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)
          }
          else {
           // passwords are NOT EQUAL!
            delay(200);
            lcd.clear();
            lcd.setCursor(0, 0);
            lcd.print("PWD No Match");

            // TRANSMIT REASSURANCE RESPONSE ("INCORRECT OLD PASSWORD ...")
            delay(500);
            digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
            digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)
            speak("[x0][t6][v8][s6][m51][g2][h2][n1]Incorrect old password entered. Password change unsuccessful.");
            delay(6500);
            digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
            digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)
          }

          buffer_string = "";

        }
      }



      // transmit callsign at specified interval
      if (( millis() < lastCallsignTX) or ( millis() > lastCallsignTX + callsignDelayTime))  //first conditional captures overflow case with millis
      {
        lastCallsignTX = millis();  //capture current time

        // txCallsignState = true;  // set transmit callsign state to TRUE

        //Speech Synthesis Chip code
        digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
        digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

        speak("[x0][t6][v8][s6][m51][g2][h2][n1]K 3 A U K");  // send command to speech chip to say callsign
        delay(2000);
        transmitMorseCode(morseCode.c_str());
        digitalWrite(10, LOW);  // switch audio source back to input radio
      }

      // // disable transmission after callsign has been played (~ 1 seconds for now)
      // if (detectionState == false){
      //   if (( millis() < lastCallsignTX) or ( millis() > lastCallsignTX + 1000))  //first conditional captures overflow case with millis
      //   {
      //     txCallsignState = false;  // set transmit callsign state to FALSE

      //     // open both relays
      //     digitalWrite(9, LOW);
      //     digitalWrite(10, LOW);

          
      //   }
      // }
}

// decodeDTMF ISR

void decodeDTMF() {
//      // change action state of interrupt for decodeDTMF
        actionState = !actionState;
}
