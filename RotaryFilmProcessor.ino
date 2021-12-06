#include <M5Core2.h>
#include <Tic.h>
#include "Free_Fonts.h"

TicI2C tic;

bool runMotor = false;
float stopwatch = 0;
int startTime = 0;
int nowTime = 0;
int displayMin = 0;
char displaySec[10];
char displayBuffer[10];

#define BGCOLOR BLACK
#define TEXTCOLOR WHITE
#define TITLE_FONT FMB12
#define TIMER_FONT FMB24

void setup() {
  // put your setup code here, to run once:

  M5.begin(true, false, true, true);  //Init M5Core, including i2c begin.  LCD on, SD card off, Serial on, I2C on

  M5.Lcd.setTextColor(TEXTCOLOR, BGCOLOR);
  M5.Lcd.setFreeFont(TITLE_FONT);
  M5.Lcd.drawString("Rotary Processor", 45, 10, GFXFF);

  delay(20);
  tic.exitSafeStart();

}

void loop() {
  // put your main code here, to run repeatedly:

  tic.resetCommandTimeout();
  
  M5.update(); //Read the press state of the key.

  if (M5.BtnA.wasReleased() && !runMotor) {
    runMotor = true;
    startTime = millis();
    stopwatch = 0;
    tic.setTargetVelocity(2000000);  //200 steps/second
  } else if (M5.BtnA.wasReleased() && runMotor) {
    runMotor = false;
    tic.setTargetVelocity(0);
  }

  if (runMotor) {
    nowTime = millis();
    M5.Lcd.setFreeFont(TIMER_FONT);
    sprintf(displayBuffer,"%d:%04.1f ",floor((nowTime-startTime)/60000),((nowTime-startTime)%60000)/(float)1000);
    M5.Lcd.drawString(displayBuffer, 70, 70, GFXFF);

  }

  delay(30);

}
