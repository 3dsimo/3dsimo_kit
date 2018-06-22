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

/*
 *   define structure for the material list
 */
typedef struct {
  int temperature;
  int motorSpeed;
  char* materialName;
} profile_t;

/*
 *   define material profiles
 */
const profile_t materials[] PROGMEM = {  
  // {temperature (deg. C), motorSpeed (%), materialName}
     {225,                  25,             "ABS"},
     {210,                  40,             "PLA"}
};

/*
 *   define number of materials in list and variables
 */
#define MATERIAL_COUNT  2

int materialID = 0;         // chosen material profile
int setTemperature = 225;   // set heater temperature
int setMotorSpeed = 60;     // set motor speed in %

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

/*
 *   load actual material profile
 */
void loadMaterial(int id){
  profile_t profile;
  char text[10];
  
  // load material profile from PROGMEM and assign variables
  memcpy_P(&profile, &materials[id], sizeof(profile_t));
  setTemperature = profile.temperature;
  setMotorSpeed  = profile.motorSpeed;
  
  // clear display and show all information
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
  int valuePID = getPIDoutput(setTemperature, temperature, 255, 0);
   
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

  // button UP pressed
  if(!digitalRead(BTN_UP) && digitalRead(BTN_DOWN)){
    if(!(buttonsPressed & 0x01)){
      if(materialID < MATERIAL_COUNT-1){
        ++materialID;        
      }
      else{
        materialID = 0;
      }
      loadMaterial(materialID);
    }
    // save that this button UP was already pressed and used
    buttonsPressed |= 0x01;
  }
  else{
    // save that this button UP was released
    buttonsPressed &= 0xFE;
  }

  // button DOWN pressed
  if(digitalRead(BTN_UP) && !digitalRead(BTN_DOWN)){
    if(!(buttonsPressed & 0x02)){
      if(materialID > 0){
        --materialID;
      }
      else{
        materialID = MATERIAL_COUNT-1;
      }
      loadMaterial(materialID);
    }
    // save that this button DOWN was already pressed and used
    buttonsPressed |= 0x02;
  }
  else{
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
