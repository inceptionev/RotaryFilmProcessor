#include <M5Core2.h>
#include <Tic.h>
#include "Free_Fonts.h"

TicI2C tic;

bool runMotor = false;
float stopwatch = 0;
int startTime = 0;
int nowTime = 0;
int displayMin = 0;
char displayBuffer[10];

//create sprite for live timer update
TFT_eSprite disp_buffer = TFT_eSprite(&M5.Lcd);

#define BGCOLOR BLACK
#define TEXTCOLOR WHITE
#define TIMERCOLOR GREEN
#define TITLE_FONT FMB12
#define TIMER_FONT FMB24

void setup() {
  // put your setup code here, to run once:

  M5.begin(true, false, true, true);  //Init M5Core, including i2c begin.  LCD on, SD card off, Serial on, I2C on

  M5.Lcd.setTextColor(TEXTCOLOR, BGCOLOR);
  M5.Lcd.setFreeFont(TITLE_FONT);
  M5.Lcd.drawString("Rotary Processor", 45, 10, GFXFF);

  delay(20);
  tic.setTargetVelocity(0);
  tic.exitSafeStart();

  M5.update(); //clear any remaining button presses

}

void loop() {
  // put your main code here, to run repeatedly:

  disp_buffer.setSwapBytes(false);
  disp_buffer.createSprite(240, 40);

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
    displayMin = floor((nowTime-startTime)/60000);
    disp_buffer.setFreeFont(TIMER_FONT);
    disp_buffer.setTextColor(TIMERCOLOR, BGCOLOR);
    sprintf(displayBuffer,"%d:%04.1f ",displayMin,((nowTime-startTime)%60000)/(float)1000);
    disp_buffer.drawString(displayBuffer, 70, 0, GFXFF);
    disp_buffer.pushSprite(0, 70);

  }

  delay(30);

}
