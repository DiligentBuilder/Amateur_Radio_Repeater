#include <LiquidCrystal_I2C.h>

#include <SoftwareSerial.h>

#include "Codec.h"

#include "StdAfx.h"

static FILE * Fp;

static CODEECMODEID g_systate;

static VOLUMEID g_volume;

SoftwareSerial ss(7,8);


LiquidCrystal_I2C lcd(0x27, 16, 2);

// Start recording coding

void StartEncode (void) {
  VOLUMEID volume = BR_volume7;
  g_systate = Codec_encode;
  g_volume = BR_volume7;
  Fp = fopen("codec.bts", "wb");

  // start coding, set the sample rate to 16k, the bit rate to 5, and the volume to 7
  Codec_Start(g_systate, SR_16000, BR_bitrate5, app_sendcommand);

  while(1) {
    // if the data processing returns non-byte 0, then there is an error, quit
    if (Codec_Byte_Proc(UART_Get_byte())) {
      Codec_Stop();
    }

    // Need to exit
    if (g_systate != Codec_encode) {
      Codec_Stop();
    }

    // Whether to reset the volume
    if (g_volume != volume) {
      g_volume = volume;
      Codec_SetVolume(g_volume);

    }
  }

  fclose(Fp);
  
}

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
