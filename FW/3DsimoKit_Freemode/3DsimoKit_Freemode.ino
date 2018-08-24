/*
 * WIP - A alternative control mode that does not use predefined material profiles. 
 * But allows de-/increasing the temperature in steps of 5, from 0 to maximum.
 * Later: UP and DOWN pressed together: change control mode to set motor speed the same way
 * Therefore the Up-Down-Buttons need to do their thing On Release (not while pressed)
 * my Comments are just my personal notes...
 */

/*
 *  ssd1306   128x32   Alexey Dinda Library
 */
#include "ssd1306.h"
#include "nano_gfx.h"
#include <EveryTimer.h>

/*
 *   define Input/Outputs
 */

#define LED_NANO    13    // LED placed on Arduino Nano board

#define BTN_UP      11    // controlling button UP/PLUS
#define BTN_DOWN    12    // controlling button DOWN/MINUS
#define BTN_EXT     8     // button for material extrusion
#define BTN_REV     7     // button for material reverse

#define MOTOR_DIR   6     // motor direction output
#define MOTOR_PWM   10    // motor PWM output for power driving
#define MOTOR_SLEEP 5
#define HEATER_EN   9     // heater/power output 

#define TEMP_IN     A0    // temperature measure ADC input 

/*
 *   define heating states
 */
enum {
  STATE_HEATING,
  STATE_COOLING,
  STATE_READY,
} STATE_e;

enum {
  MOTOR_STOP,
  MOTOR_EXTRUSION,
  MOTOR_REVERSE,
  MOTOR_CONTINUOUS,
  MOTOR_REVERSE_AFTER_EXTRUSION
} MOTOR_STATE_e;

/* TODO
 *   define structure for the material list
 */
typedef struct {
  int temperature;
  int motorSpeed;
  char* materialName;
} profile_t;

/* TODO
 *   define material profiles
 */
const profile_t materials[] PROGMEM = {  
  // {temperature (deg. C), motorSpeed (%), materialName}
     {0,                  0,             "OFF"},    /* NEW! BEGIN OFF - BUT IF YOU SELECT THIS AFTER PETG, 3DPEN COOLS TO 153º PRIOR TO SHUTDOWN*/
     {210,                  40,             "PLA"},
     {230,                  30,             "ABS"},
     {235,                  40,             "PETG"}
};

/* TODO
 *   define number of materials in list and variables
 */
#define MATERIAL_COUNT  4

#define MAXTEMP 255         // not sure whats better: define or cont, also not sure about the actual max temp... testing with 255
#define MINTEMP 0

int materialID = 0;         // chosen material profile
int setTemperature = 0;   // set heater temperature
int setMotorSpeed = 0;     // set motor speed in %

/*
 *   create timer for main loop
 */
EveryTimer timer;

/*
 *   function for measuring temperature of the tip
 */
int getTemperature(){  // get temperature in deg. Celsius from ADU value
  // set reference for ADC to power supply (5V)
  analogReference(DEFAULT);

  int avgTemp = 0;
  for(int i = 0; i<16; i++){
    avgTemp += analogRead(TEMP_IN);
  }
  
  // read averaged analog value of temperature
  long tempADU = avgTemp >> 4;
  
  // convert ADU into temperature
  // constants could slightly change for different ceramic tip
  tempADU -= 1692; // increase this value when temperature is too high and vice versa
  tempADU <<= 7;
  tempADU /= (-557); 
  
  return tempADU; 
}

/* TODO Change into update_display funtion. will be callled after button updown press and simply draws speed and temp
 *   load actual material profile
 */
void loadMaterial(int id){
  profile_t profile;
  char text[10];
  
  // load material profile from PROGMEM and assign variables
  memcpy_P(&profile, &materials[id], sizeof(profile_t));
  setTemperature = profile.temperature; //  !!! IMPORTANT
  setMotorSpeed  = profile.motorSpeed;
  
  // clear display and show all information TODO: Move
  sprintf(text, "%d %%", setMotorSpeed);
  ssd1306_clearScreen();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixedN(0,  0, profile.materialName, STYLE_NORMAL, FONT_SIZE_2X);  
  ssd1306_printFixedN(80, 0, text, STYLE_NORMAL, FONT_SIZE_2X);
}

/*
 *  PID variables and constants for tuning
 */
float Kp=15, Ki=1, Kd=1.0, dT = 0.1, Hz=10;

/*
 *   basic PID routine to get output value
 */
int getPIDoutput(int setPoint, int actualValue, int maxValue, int minValue){
  static float sumE = 0;
  static int16_t error, previousError = 0;
  float outputValue;
  static int pidAvg[4] = {0,0,0,0};
  static int pidAvgIndex = 0;
  

  // reset sumE when actualValue exceed setPoint by 5
  static int noWaitCycles = 0;
  if(actualValue > setPoint + 5){
    ++noWaitCycles;
    if(noWaitCycles >= 30){
      sumE = 100;
      noWaitCycles = 0;
    }
  }
  else{
    noWaitCycles = 0;
  }

  // PID implementation
  error = setPoint - actualValue;
  sumE += (float) error * dT;
  outputValue =  Kp*error + Ki*sumE + Kd*(error - previousError)/dT;
  previousError = error;

  // restrict output PID value into range between minValue and maxValue
  if(outputValue > maxValue)
    outputValue = maxValue;
  else if(outputValue < minValue)
    outputValue = minValue;

  // store n output values for averaging
  pidAvg[pidAvgIndex] = outputValue;
  ++pidAvgIndex;
  if(pidAvgIndex >= 4)
    pidAvgIndex = 0;

  // average last n output values
  int sumPIDavg = 0;
  for(int i = 0; i<4; i++){
    sumPIDavg += pidAvg[i];
  }
  sumPIDavg >>= 2;
  
  return sumPIDavg;
}

#define NO_AVERAGES_VALUES  64

/*
 *   heating function for heater driving by PID regulator
 */
int heating(){
  static int tempAvg[NO_AVERAGES_VALUES];   // temperature array for averaging it
  static int tempAvgIter = 0;               // current index in temperature array
  static char firstTime = 0;                // if is 1, this function ran at least one time
  char text[30];                            // buffer for text

  // variables initialization
  if(!firstTime){
    memset(tempAvg, 0, sizeof(tempAvg)*sizeof(int));
    firstTime = 1;
  }

  // resolve PID value for heater PWM
  int temperature = getTemperature();
  int valuePID = getPIDoutput(setTemperature, temperature, 255, 0); // !!! IMPORTANT
   
  analogWrite(HEATER_EN, valuePID);

  // save actual temperature for averaging
  tempAvg[tempAvgIter] = temperature;
  if(++tempAvgIter>=NO_AVERAGES_VALUES)
    tempAvgIter = 0;

  // make temperature average from NO_AVERAGES_VALUES
  int sumTemp = 0;
  for(int i = 0; i<NO_AVERAGES_VALUES; i++){
    sumTemp += tempAvg[i];  
  }
  sumTemp /= NO_AVERAGES_VALUES;

  // show on display actual and preset temperature
  sprintf(text, "%3d/%3dC", sumTemp, setTemperature);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixedN(0, 16, text, STYLE_NORMAL, FONT_SIZE_2X);

/*
 * debug output into display and to serial
 */
/*
  sprintf(text, "PID=%03d", valuePID);
  ssd1306_printFixed(0, 0, text, STYLE_NORMAL);

  sprintf(text, "%3d;%3d", valuePID, sumTemp);
  Serial.println(text);
*/
  
  return sumTemp;
}

/*
 *   call every 50ms timer for buttons action and heating 
 *   execute buttons function according to heating state
 *   change material profile   
 */
void timerAction(){
  static int elapsedTime = 0;
  static char statusHeating = STATE_HEATING;   
  static char stateMotor = MOTOR_STOP, lastMotorState = MOTOR_STOP;
  static int timeMotorReverse = 0;

  // decide temperature state (heating, cooling, ready) and show it on display
  if(++elapsedTime==2){ // 100ms
    elapsedTime = 0;
    int actualTemperature = heating();
    
    // tolerant zone where temperature is ABOVE preset temperature, 
    // but it is possible to do extrusion/reverse
    if(actualTemperature > setTemperature + 10){
      statusHeating = STATE_COOLING;
      ssd1306_printFixedN(116, 16, "C", STYLE_NORMAL, FONT_SIZE_2X);
    }

    // tolerant zone where temperature is OK for extrusion/reverse
    else if(actualTemperature > setTemperature - 10){
      statusHeating = STATE_READY;
      ssd1306_printFixedN(116, 16, "R", STYLE_NORMAL, FONT_SIZE_2X);
      digitalWrite(LED_NANO, HIGH);   // turn the LED on (HIGH is the voltage level)
    }

    // tolerant zone where temperature is LOW for extrusion/reverse
    else{
      statusHeating = STATE_HEATING;
      ssd1306_printFixedN(116, 16, "H", STYLE_NORMAL, FONT_SIZE_2X);
      digitalWrite(LED_NANO, !digitalRead(LED_NANO));   // turn the LED on (HIGH is the voltage level)
    }
  }

  // assing functions according to heating state (mainly button function)
  switch(statusHeating){
    case STATE_COOLING:
    case STATE_READY:{
      // button EXTRUSION is pressed, extrude material
      if(!digitalRead(BTN_EXT) && digitalRead(BTN_REV)){
        stateMotor = MOTOR_EXTRUSION;
      }
      
      // button REVERSE is pressed, retract material
      else if(digitalRead(BTN_EXT) && !digitalRead(BTN_REV)){
        stateMotor = MOTOR_REVERSE;
        timeMotorReverse = 400; // reverse time is 50ms * timeMotorReverse (400 = 20s)
      }
      
      // both buttons are pressed, motor stopped
      else if(!digitalRead(BTN_EXT) && !digitalRead(BTN_REV)){
        stateMotor = MOTOR_STOP;
      }
      
      // not buttons are pressed
      else{
        if(lastMotorState == MOTOR_EXTRUSION){
          stateMotor = MOTOR_REVERSE_AFTER_EXTRUSION;
          timeMotorReverse = 20; // reverse time is 50ms * timeMotorReverse (20 = 1s)
        }
      }    
      break;
    }
    
    case STATE_HEATING:
      // if happened that heater has so low temperature, motor stop
      digitalWrite(MOTOR_DIR, LOW);
      analogWrite(MOTOR_PWM, 0);
      stateMotor = MOTOR_STOP;
      break;
  }

  // resolve motor states (Extrusion, Reverse, Stop, ...)
  switch(stateMotor){
    case MOTOR_STOP:
      digitalWrite(MOTOR_DIR, LOW);
      analogWrite(MOTOR_PWM, 0);
      break;
      
    case MOTOR_EXTRUSION:{
      int pwmSpeed = setMotorSpeed*255;
      digitalWrite(MOTOR_DIR, LOW);
      analogWrite(MOTOR_PWM, pwmSpeed/100);
      break;
    }
    
    case MOTOR_REVERSE:
      --timeMotorReverse;
      if(timeMotorReverse > 0){
        digitalWrite(MOTOR_DIR, HIGH);
        analogWrite(MOTOR_PWM, 0);
      }
      else{
        stateMotor = MOTOR_STOP;
      }
      break;

    case MOTOR_REVERSE_AFTER_EXTRUSION:
      --timeMotorReverse;
      if(timeMotorReverse > 0){
        int pwmSpeed = (100-setMotorSpeed)*255;
        digitalWrite(MOTOR_DIR, HIGH);
        analogWrite(MOTOR_PWM, pwmSpeed/100);
      }
      else{
        stateMotor = MOTOR_STOP;
      }  
      break;        
  }
  lastMotorState = stateMotor;  
  
  // one time action, mainly for material change
  static char buttonsPressed = 0;



  // button UP increases profile on release
  if (digitalRead(BTN_UP) && !digitalRead(BTN_DOWN)) {
    // save that this button UP was already pressed
    buttonsPressed |= 0x01; // This might be efficient, but it's obscure. Bitwise "flags" or something...
    // What |= 0x01 does: sets the last bit to true, no matter what
    // Note: http://www.hw2sw.com/2011/09/13/arduino-bitwise-operators-and-advanced-tricks/
  } else {
    if (buttonsPressed & 0x01) {  // "Was I pressed in the last cycle?"
      if (materialID < MATERIAL_COUNT-1){
        ++materialID;        
      } else {
        materialID = 0;
      }
      loadMaterial(materialID);
    }
    
    // save that this button UP was released
    buttonsPressed &= 0xFE;
    
    // ToDo: okay I'm puzzled, it's a mask for the bitwise flag thingy, I guess. 
    // But why this huge HEX number... why not "2"... maybe to support future buttons? 
    // what it does: "and" sets only true if both are true. wtf it actually does with the 0xFE?! black magic...
    // 0xFE = 11111110, I see now... it sets the last bit to false
  }

  // button DOWN change profile down on release
  if (!digitalRead(BTN_UP) && digitalRead(BTN_DOWN)) {    
    // save that this button DOWN was already pressed and used
    buttonsPressed |= 0x02;
  } else {
    if (buttonsPressed & 0x02) {
      if (materialID > 0){
        --materialID;
      } else {
        materialID = MATERIAL_COUNT-1;
      }
      loadMaterial(materialID);
    }    
    // save that this button DOWN was released
    buttonsPressed &= 0xFD;
  }
    
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
  pinMode(LED_NANO,  OUTPUT);
  pinMode(MOTOR_DIR, OUTPUT);
  pinMode(MOTOR_PWM, OUTPUT);
  pinMode(MOTOR_SLEEP, OUTPUT);
  pinMode(HEATER_EN, OUTPUT);

  // initialize inputs
  pinMode(BTN_UP,   INPUT_PULLUP);
  pinMode(BTN_DOWN, INPUT_PULLUP);
  pinMode(BTN_EXT,  INPUT_PULLUP);
  pinMode(BTN_REV,  INPUT_PULLUP);

/* TODO
 * TODO material ID
 */
  // load material profile
  loadMaterial(materialID);

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
}
