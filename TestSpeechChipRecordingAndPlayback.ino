#include <LiquidCrystal_I2C.h> // library for LCD
#include <SoftwareSerial.h>

//static FILE *Fp;


const byte ledPin = 13;

SoftwareSerial mySerial(2, 1); // RX, TX

LiquidCrystal_I2C lcd(0x27,16,2);  // I2C address of LCD chip is 0x27. LCD is 16 chars and 2 lines.

void speak(char* msg) {
  Serial.write(0xFD);
  Serial.write((byte)0x0);
  Serial.write(2 + strlen(msg));
  Serial.write(0x01);
  Serial.write((byte)0x0);
  Serial.write(msg);
}

void setup() {
  // put your setup code here, to run once:

  pinMode(ledPin, OUTPUT);

  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);


  // Open serial communications and wait for port to open

  mySerial.begin(9600);

  // LCD setup
  lcd.init();
  lcd.clear();         
  lcd.backlight();      // Make sure backlight is on

}

void loop() {
  // put your main code here, to run repeatedly:

  lcd.clear();

  lcd.print("Recording Now");

  //Fp = fopen("codec.bts", "wb");

  // send command to record

  byte cmd1[] = {0xFD, 0x00, 0x04, 0x41, 0x01, 0x05, 0x08};

  int i1;

  for (i1 = 0; i1 < sizeof(cmd1); i1++) {
    mySerial.write(cmd1[i1]);
  }

  // wait for a few seconds

  delay(10000);

  // stop recording

  lcd.clear();

  lcd.print("Stopping Recording");

  byte cmd2[] = {0xFD, 0x00, 0x01, 0x44};

  int i2;

  for (i2 = 0; i2 < sizeof(cmd2); i2++) {
    mySerial.write(cmd2[i2]);
  }

  //fclose(Fp);

  // wait for a few seconds

  delay(2000);

  // play back our recording

  //Fp = fopen("codec.bts", "rb");

  //Speech Synthesis Chip code
  digitalWrite(9, HIGH);  // enable transmission on TX radio (close relay 1)
  digitalWrite(10, HIGH);  // switch audio source to speech chip (close relay 2)

  lcd.clear();

  lcd.print("Starting Playback");

  speak("[x0][t6][v8][s6][m51][g2][h2][n1]K 3 A U K Testing.");  // send command to speech chip to say callsign

  delay(3000);

  

  byte cmd3[] = {0xFD, 0x00, 0x04, 0x41, 0x01, 0x05, 0x05};

  int i3;

  for (i3 = 0; i3 < sizeof(cmd3); i3++) {
    mySerial.write(cmd3[i3]);
  }

  // byte cmd4[] = {0xFD, 0x00, 0x3F, 0x43, 0xFC, 0x00, 0x01, 0x23};

  // int i4;

  // for (i4 = 0; i4 < sizeof(cmd3); i4++) {
  //   mySerial.write(cmd3[i4]);
  // }

  // wait for a few seconds

  delay(10000);

  // stop the decoding

  //Speech Synthesis Chip code
  digitalWrite(9, LOW);  // enable transmission on TX radio (close relay 1)
  digitalWrite(10, LOW);  // switch audio source to speech chip (close relay 2)

  lcd.clear();

  lcd.print("Stopping Playback");

  byte cmd5[] = {0xFD, 0x00, 0x01, 0x44};

  int i5;

  for (i5 = 0; i5 < sizeof(cmd2); i5++) {
    mySerial.write(cmd5[i5]);
  }

  //fclose(Fp);

  delay(5000);

}
