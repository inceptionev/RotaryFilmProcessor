#include <M5Core2.h>
#include <Tic.h>
#include "Free_Fonts.h"
#include "AtomSPK.h"

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
int beepStart = 0;
int beepCounter = 0;

int state = 0;
int activeTimer = 4;  //default 4, free-running with no timer

int timerA = 40;  //default timer settings in seconds
int timerB = 270;
int timerC = 390;

//create sprite for live timer update
TFT_eSprite disp_buffer = TFT_eSprite(&M5.Lcd);
TFT_eSprite remain_buffer = TFT_eSprite(&M5.Lcd);

ATOMSPK speaker;


Button buttonFreeRun(0, 50, 240, 80);

#define MOTOR_VELOCITY 2000000

#define Speak_I2S_NUMBER I2S_NUM_0  
#define MODE_MIC 0
#define MODE_SPK 1
#define DATA_SIZE 1024
#define BEEP_NOTE 661
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

  SpeakInit();

  speaker.begin();

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
        beepStart = millis();
        beepCounter = BEEPS_30S;
        state = 3;
      }
      break;

    case 3:
      if (beepCounter > 0 && millis()-beepStart > BEEPS_INTERVAL) {
        speaker.playBeep();
        beepCounter--;
        beepStart = millis();
      }
      if (countdown-(nowTime-startTime)/1000 < 20) {
        beepStart = millis();
        beepCounter = BEEPS_20S;
        state = 4;
      }
      break;

    case 4:
      if (beepCounter > 0 && millis()-beepStart > BEEPS_INTERVAL) {
        speaker.playBeep();
        beepCounter--;
        beepStart = millis();
      }
      if (countdown-(nowTime-startTime)/1000 < 10) {
        beepStart = millis();
        beepCounter = BEEPS_10S;
        state = 5;
      }
      break;

    case 5:
      if (beepCounter > 0 && millis()-beepStart > BEEPS_INTERVAL) {
        speaker.playBeep();
        beepCounter--;
        beepStart = millis();
      }
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

    displayMin = constrain(floor((countdown-(nowTime-startTime)/(float)1000)/(float)60), 0, 100);
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

void SpeakInit(void){ // 初始化扬声器
  M5.Axp.SetSpkEnable(true);  //启用扬声器电源
  InitI2SSpeakOrMic(MODE_SPK);
}

bool InitI2SSpeakOrMic(int mode){  //Init I2S.  初始化I2S
    esp_err_t err = ESP_OK;

    i2s_driver_uninstall(Speak_I2S_NUMBER); // Uninstall the I2S driver.  卸载I2S驱动
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER),  // Set the I2S operating mode.  设置I2S工作模式
        .sample_rate = 44100, // Set the I2S sampling rate.  设置I2S采样率
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // Fixed 12-bit stereo MSB.  固定为12位立体声MSB
        .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // Set the channel format.  设置频道格式
        .communication_format = I2S_COMM_FORMAT_I2S,  // Set the format of the communication.  设置通讯格式
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Set the interrupt flag.  设置中断的标志
        .dma_buf_count = 2, //DMA buffer count.  DMA缓冲区计数
        .dma_buf_len = 128, //DMA buffer length.  DMA缓冲区长度
    };
    if (mode == MODE_MIC){
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM);
    }else{
        i2s_config.mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX);
        i2s_config.use_apll = false;  //I2S clock setup.  I2S时钟设置
        i2s_config.tx_desc_auto_clear = true; // Enables auto-cleanup descriptors for understreams.  开启欠流自动清除描述符
    }
    // Install and drive I2S.  安装并驱动I2S
    err += i2s_driver_install(Speak_I2S_NUMBER, &i2s_config, 0, NULL);

    i2s_pin_config_t tx_pin_config;
    tx_pin_config.bck_io_num = CONFIG_I2S_BCK_PIN;  // Link the BCK to the CONFIG_I2S_BCK_PIN pin. 将BCK链接至CONFIG_I2S_BCK_PIN引脚
    tx_pin_config.ws_io_num = CONFIG_I2S_LRCK_PIN;  //          ...
    tx_pin_config.data_out_num = CONFIG_I2S_DATA_PIN;  //       ...
    tx_pin_config.data_in_num = CONFIG_I2S_DATA_IN_PIN; //      ...
    err += i2s_set_pin(Speak_I2S_NUMBER, &tx_pin_config); // Set the I2S pin number.  设置I2S引脚编号
    err += i2s_set_clk(Speak_I2S_NUMBER, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO); // Set the clock and bitwidth used by I2S Rx and Tx. 设置I2S RX、Tx使用的时钟和位宽
    return true;
}
