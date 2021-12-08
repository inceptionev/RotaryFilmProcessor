#include <M5Core2.h>
#include <Tic.h>
#include "Free_Fonts.h"

//state planning
//state 0, everything stopped, allows free-running drive, can edit settings
//state 1, free running
//state 2, run a timer
//state 3, timer 30-21s remaining
//state 4, timer 20-11s remaining
//state 5, timer, last 10 seconds
//state 6, adjust setting


TicI2C tic;

bool runMotor = false;
float stopwatch = 0;
int startTime = 0;
int countdown = 0;
int nowTime = 0;
int displayMin = 0;
int displaySec = 0;
char displayBuffer[10];
int beepsCounter = 0;

int state = 0;
int activeTimer = 4;  //default 4, free-running with no timer

int timerA = 390;  //default timer settings in seconds
int timerB = 270;
int timerC = 390;

//create sprite for live timer update
TFT_eSprite disp_buffer = TFT_eSprite(&M5.Lcd);
TFT_eSprite remain_buffer = TFT_eSprite(&M5.Lcd);

Button buttonFreeRun(0, 50, 240, 80);

#define MOTOR_VELOCITY 2000000

#define BEEPS_30S 2
#define BEEPS_20S 4
#define BEEPS_10S 20
#define BEEPS_INTERVAL 500

#define BGCOLOR BLACK
#define TEXTCOLOR WHITE
#define ELAPSEDCOLOR GREEN
#define REMAINCOLOR RED
#define TITLE_FONT FMB12
#define ELAPSED_FONT FMB24
#define REMAIN_FONT FMB18
#define PARAM_FONT FMB12
#define HLCOLOR BLUE

void setup() {
  // put your setup code here, to run once:

  M5.begin(true, false, true, true);  //Init M5Core, including i2c begin.  LCD on, SD card off, Serial on, I2C on

  M5.Lcd.setTextColor(TEXTCOLOR, BGCOLOR);
  M5.Lcd.setFreeFont(TITLE_FONT);
  M5.Lcd.drawString("Rotary Processor", 50, 10, GFXFF);

  delay(20);
  tic.setTargetVelocity(0);
  tic.exitSafeStart();

  M5.update(); //clear any remaining button presses

  disp_buffer.setSwapBytes(false);
  disp_buffer.createSprite(240, 40);

  remain_buffer.setSwapBytes(false);
  remain_buffer.createSprite(240, 30);

  M5.Lcd.setTextColor(ELAPSEDCOLOR, BGCOLOR);
  M5.Lcd.setFreeFont(PARAM_FONT);
  M5.Lcd.drawString("ELAPSED", 110, 45, GFXFF);
  
  M5.Lcd.setTextColor(REMAINCOLOR, BGCOLOR);
  M5.Lcd.setFreeFont(PARAM_FONT);
  M5.Lcd.drawString("REMAIN", 118, 120, GFXFF);
  
  writeParam(1, (char*)"A", false);
  writeParam(3, (char*)"B", false);
  writeParam(5, (char*)"C", false);
  sprintf(displayBuffer,"%d:%02d", int(floor(timerA/60)), int(timerA%60));
  writeParam(0, displayBuffer, false);
  sprintf(displayBuffer,"%d:%02d", int(floor(timerB/60)), int(timerB%60));
  writeParam(2, displayBuffer, false);
  sprintf(displayBuffer,"%d:%02d", int(floor(timerC/60)), int(timerC%60));
  writeParam(4, displayBuffer, false);
}

void loop() {
  // put your main code here, to run repeatedly:

  tic.resetCommandTimeout();
  
  M5.update(); //Read the press state of the key.

  switch(state) {

    case 0:
      if (buttonFreeRun.wasReleased()) {
        runMotor = true;
        startTime = millis();
        stopwatch = 0; 
        tic.setTargetVelocity(MOTOR_VELOCITY);
        state = 1; 
        } else if (M5.BtnA.wasReleased()) {
          stopwatch = 0;
          countdown = timerA;
          state = 2;
          tic.setTargetVelocity(MOTOR_VELOCITY);
          runMotor = true;
          startTime = millis();
        } else if (M5.BtnB.wasReleased()) {
          stopwatch = 0;
          countdown = timerB;
          state = 2;
          tic.setTargetVelocity(MOTOR_VELOCITY);
          runMotor = true;
          startTime = millis();
        } else if (M5.BtnC.wasReleased()) {
          stopwatch = 0;
          countdown = timerC;
          state = 2;
          tic.setTargetVelocity(MOTOR_VELOCITY);
          runMotor = true;
          startTime = millis();
        }
      break;

    case 1:
      if (buttonFreeRun.wasReleased()) { 
        runMotor = false;
        tic.setTargetVelocity(0);
        state = 0; 
        }
      break;

    case 2:
      if (countdown-(nowTime-startTime)/1000 < 30) {
        state = 3;
      }
      break;

    case 3:
      if (countdown-(nowTime-startTime)/1000 < 20) {
        state = 4;
      }
      break;

    case 4:
      if (countdown-(nowTime-startTime)/1000 < 10) {
        state = 5;
      }
      break;

    case 5:
      if (countdown*1000-(nowTime-startTime) < 0) {
        countdown = 0;
        tic.setTargetVelocity(0);
        runMotor = false;
        state = 0;
      }
      break;

    case 6:
      break;

    default:
      state = 0;
      break;
  }

  if (runMotor) {
    nowTime = millis();
    displayMin = floor((nowTime-startTime)/60000);
    disp_buffer.setFreeFont(ELAPSED_FONT);
    disp_buffer.setTextColor(ELAPSEDCOLOR, BGCOLOR);
    sprintf(displayBuffer,"%d:%04.1f ",displayMin,((nowTime-startTime)%60000)/(float)1000);
    disp_buffer.drawString(displayBuffer, 75, 0, GFXFF);
    disp_buffer.pushSprite(0, 70);

    displayMin = floor((countdown-(nowTime-startTime)/(float)1000)/(float)60);
    displaySec = ceil(((countdown*1000-nowTime+startTime)%60000)/(float)1000);
    if (countdown == 0) {
      displayMin = 0;
      displaySec = 0;
    }
    remain_buffer.setFreeFont(REMAIN_FONT);
    remain_buffer.setTextColor(REMAINCOLOR, BGCOLOR);
    sprintf(displayBuffer,"-%d:%02d ",displayMin, displaySec);
    remain_buffer.drawString(displayBuffer, 100, 0, GFXFF);
    remain_buffer.pushSprite(0, 145);

  }

  //delay(30);

}

void writeParam(int nParam, char* paramString, bool hl) {
  
  M5.Lcd.setFreeFont(PARAM_FONT);
  
  switch(nParam) {
    case 0: //Column 1 Top
      hl ? M5.Lcd.setTextColor(WHITE, HLCOLOR) : M5.Lcd.setTextColor(WHITE, BGCOLOR);
      M5.Lcd.setCursor(30,206);
      M5.Lcd.print(paramString);
      break;

    case 1: //Column 1 Bottom
      hl ? M5.Lcd.setTextColor(WHITE, HLCOLOR) : M5.Lcd.setTextColor(WHITE, BGCOLOR);
      M5.Lcd.setCursor(48,230);
      M5.Lcd.print(paramString);
      break;

    case 2: //Column 2 Top
      hl ? M5.Lcd.setTextColor(WHITE, HLCOLOR) : M5.Lcd.setTextColor(WHITE, BGCOLOR);
      M5.Lcd.setCursor(133,206);
      M5.Lcd.print(paramString);
      break;

    case 3: //Column 2 Bottom
      hl ? M5.Lcd.setTextColor(WHITE, HLCOLOR) : M5.Lcd.setTextColor(WHITE, BGCOLOR);
      M5.Lcd.setCursor(153,230);
      M5.Lcd.print(paramString);
      break;

    case 4: //Column 3 Top
      hl ? M5.Lcd.setTextColor(WHITE, HLCOLOR) : M5.Lcd.setTextColor(WHITE, BGCOLOR);
      M5.Lcd.setCursor(241,206);
      M5.Lcd.print(paramString);
      break;

    case 5: //Column 3 Bottom
      hl ? M5.Lcd.setTextColor(WHITE, HLCOLOR) : M5.Lcd.setTextColor(WHITE, BGCOLOR);
      M5.Lcd.setCursor(259,230);
      M5.Lcd.print(paramString);
      break;

  }
}
