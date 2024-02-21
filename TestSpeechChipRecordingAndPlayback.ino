#include <LiquidCrystal_I2C.h> // library for LCD
#include <SoftwareSerial.h>

const byte ledPin = 13;

SoftwareSerial mySerial(2, 1); // RX, TX



LiquidCrystal_I2C lcd(0x27,16,2);  // I2C address of LCD chip is 0x27. LCD is 16 chars and 2 lines.



void setup() {
  // put your setup code here, to run once:

  pinMode(ledPin, OUTPUT);

  

  // Open serial communications and wait for port to open

  mySerial.begin(9600);

  

}

void loop() {
  // put your main code here, to run repeatedly:

  lcd.print("Recording Now");

  // send command to record

  byte cmd1[] = {0xFD, 0x00, 0x04, 0x41, 0x01, 0x05, 0x08};

  int i1;

  for (i1 = 0; i1 < sizeof(cmd1); i1++) {
    mySerial.write(cmd1[i1]);
  }

  // wait for a few seconds

  // stop recording

  delay(10000);

  lcd.print("Stopping Recording");

  byte cmd2[] = {0xFD, 0x00, 0x01, 0x44};

  int i2;

  for (i2 = 0; i2 < sizeof(cmd2); i2++) {
    mySerial.write(cmd2[i2]);
  }

  delay(5000);

  

  // play back our recording

  lcd.print("Starting Playback");

  byte cmd3[] = {0xFD, 0x00, 0x04, 0x41, 0x01, 0x05, 0x05};

  int i3;

  for (i3 = 0; i3 < sizeof(cmd3); i3++) {
    mySerial.write(cmd3[i3]);
  }



  // stop the decoding

  lcd.print("Stopping Playback");

  delay(10000);

  byte cmd4[] = {0xFD, 0x00, 0x01, 0x44};

  int i4;

  for (i4 = 0; i4 < sizeof(cmd2); i4++) {
    mySerial.write(cmd4[i4]);
  }

  delay(5000);

}
