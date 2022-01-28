/*
 *  ssd1306   128x32   Alexey Dinda Library
 */
#include <stdio.h>
#include <string.h>
#include "config_Freemode.h"
#include "ssd1306.h"
#include "nano_gfx.h"
#include "nanodeUNIO.h"
#include "accessories.h"
#include <EveryTimer.h>

/*
 *   create timer for main loop 
 */
EveryTimer timer;

bool righthanded = true;
bool pressUPDOWNhandled = true;
extern int materialID;

/*
 *   call every 50ms timer for buttons action and heating 
 *   execute buttons function according to heating state
 *   change material profile   
 */
void timerAction(){
  accessories_t acs;
  static accessories_t acsLast;
  char text[20];

  acsIdentify();
  acs = getAccessories();  
  
  if(acsLast.type != acs.type){
    ssd1306_clearScreen();    
  }
  
  if(acs.type == ACS_NONE){
    sprintf(text, "No tip     ");
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_printFixedN(0, 0, text, STYLE_NORMAL, FONT_SIZE_2X);
    
    sprintf(text, "Connected  ");
    ssd1306_setFixedFont(ssd1306xled_font6x8);
    ssd1306_printFixedN(0, 16, text, STYLE_NORMAL, FONT_SIZE_2X);
  }
  else {
    acs.function();    
  }
  if(acs.type != acsLast.type){
      acs.startup();
    }
  acsLast = acs;
}


/*
 *   GPIO, OLED initialize
 *   load material profile
 *   preset timer
 */
void setup() {

  // initialize OLED display
  ssd1306_128x32_i2c_init();
  ssd1306_clearScreen();
  ssd1306_flipHorizontal(1);  /* oled_ssd1306.h   NEW! rotate screen in X */
  ssd1306_flipVertical(1);   /* oled_ssd1306.h   NEW!  rotate screen in Y */
  
  

  // initialize outputs 
  digitalWrite(HEATER_EN, HIGH);
  digitalWrite(HIGH_TEMP_EN, LOW);
  digitalWrite(LOW_TEMP_EN, HIGH);
  
  pinMode(LED_R,  OUTPUT);
  pinMode(LED_L,  OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_PWM, OUTPUT);
  pinMode(MOTOR_SLEEP, OUTPUT);
  pinMode(HEATER_EN, OUTPUT);
  pinMode(HIGH_TEMP_EN, OUTPUT);
  pinMode(LOW_TEMP_EN, OUTPUT);

  // initialize inputs
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_EXT,  INPUT_PULLUP);
  pinMode(BTN_REV,  INPUT_PULLUP);

  // preset timer period every 50 ms and call timerAction function when time expire
  timer.Every(50, timerAction);

  // initialize outputs
  digitalWrite(MOTOR_SLEEP, HIGH);

  Serial.begin(9600);
}

/*
 *   main loop
 */
void loop() {
  // call timer each preset period
  timer.Update();

  if (Serial.available() > 0) {
    char incomingByte;
    // read the incoming byte:
    incomingByte = Serial.read();
    Serial.write(incomingByte);
  }
}
