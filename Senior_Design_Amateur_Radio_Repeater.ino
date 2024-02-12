#include <LiquidCrystal_I2C.h>

#include <SoftwareSerial.h>

SoftwareSerial ss(7,8);


LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);

  ss.begin(9600);

  
  // put your setup code here, to run once:

  // code to display test message

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Hello! Test message!");

  delay(5000);


  lcd.clear();
    

}

void loop() {
  // put your main code here, to run repeatedly:

}
