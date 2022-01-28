#include "config_Freemode.h"
#include "ssd1306.h"
#include "nanodeUNIO.h"
#include "accessories.h"
#include <math.h>
#include <stdio.h>
#include <string.h>

accessories_t accessories;

/*
     define material profiles for 3D extension
     didn't remove for Freemode but shouldn't affect anything 
*/
const profile_t materials[] PROGMEM = {
  // {temperature (deg. C), motorSpeed (%), materialName}
     {0,                  0,            "OFF"},    /* NEW! BEGIN OFF - BUT IF YOU SELECT THIS AFTER PETG, 3DPEN COOLS TO 153º PRIOR TO SHUTDOWN*/
     {230,                60,           "PLA"},
     {250,                50,           "ABS"},
     {245,                60,           "PETG"},
};

#define MAXSPEED 70         // for safety. change this up to 100 (%), if you know what you are doing.
#define MAXTEMP 275         // not sure whats better: define or const, also not sure about the actual max temp... testing with 300
#define MINTEMP 155         // 153/154 is the lowest measurement possible. actual temperature can be lower. 
                            // We can use this to put heater in off mode (COOLING) if we choose "150°C" aka "LOW". However: hot temperatures will start at 155°C

int setTemperature = 0;         // set heater temperature (we raise this directly to "155" when up button is pressed once. and drop it down to "0" if the user goes below "155")
int setMotorSpeed = 40;         // set motor speed in %
                                // ToDo maybe add icrement step value (hardcoded at 5)
char controlMode = MODE_TEMP;   // Control Mode: MODE_TEMP or MODE_SPEED. selects which values the UP-DOWN buttons change

/*
     define number of materials in list and variables
*/
const int MATERIAL_COUNT  = sizeof(materials)/sizeof(profile_t);

int materialID = 0;         // chosen material profile


int elapsedTime = 0;
char statusHeating = STATE_HEATING;
char stateMotor = MOTOR_STOP, lastMotorState = MOTOR_STOP;
int timeMotorReverse = 0;
char powerPercent = 50;

void startup3D(void) {
  loadMaterial(materialID);
  digitalWrite(MOTOR_DIR, LOW);
  analogWrite(MOTOR_PWM, 0);
}

void startupBurning(void) {
  analogWrite(HEATER_EN, 255);
  digitalWrite(MOTOR_DIR, LOW);
  analogWrite(MOTOR_PWM, 0);
  powerPercent = 50;
}

void startupSoldering(void) {
  analogWrite(HEATER_EN, 255);
  digitalWrite(MOTOR_DIR, LOW);
  analogWrite(MOTOR_PWM, 0);
  powerPercent = 50;
}

void startupFoamCutting(void) {
  analogWrite(HEATER_EN, 255);
  digitalWrite(MOTOR_DIR, LOW);
  analogWrite(MOTOR_PWM, 0);
  powerPercent = 50;
}

void startupNone(void) {
  analogWrite(HEATER_EN, 255);
  digitalWrite(MOTOR_DIR, LOW);
  analogWrite(MOTOR_PWM, 0);
  powerPercent = 50;
}

void percentRoutine(void){
  char text[15];
    // button UP pressed
  if (!digitalRead(BTN_UP) && digitalRead(BTN_DOWN)) {
    if (powerPercent < 100) {
      ++powerPercent;
    }
  }

  // button DOWN pressed
  if (digitalRead(BTN_UP) && !digitalRead(BTN_DOWN)) {
    if (powerPercent > 0) {
      --powerPercent;
    }
  }
  
  // button EXTRUSION pressed
  if (!digitalRead(BTN_REV)) {
    int output = (255*(100-powerPercent))/100;
    analogWrite(HEATER_EN, output);

    // indicate that extension is working
    digitalWrite(LED_R, HIGH);   // turn the LED on
    digitalWrite(LED_L, HIGH);   // turn the LED on
  }
  else{
    analogWrite(HEATER_EN, 255);
    
    // indicate that extension is not working
    digitalWrite(LED_R, LOW);   // turn the LED off
    digitalWrite(LED_L, LOW);   // turn the LED off
  }
  
  sprintf(text, "%s", accessories.name);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixedN(0, 0, text, STYLE_NORMAL, FONT_SIZE_2X);

  sprintf(text, "%3d %%", powerPercent);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixedN(0, 16, text, STYLE_NORMAL, FONT_SIZE_2X);
}

/*
 *   function for measuring temperature of the tip in ranges 40-130 deg.C and 185-350 deg.C
 *   with automatic autorange function
 *       
 *    forbidden range is from 130 C to 185 C, because temperature is inacurate and unstable
 */
int getTemperature() { // get temperature in deg. Celsius from ADU value
  long avgTemp = 0;
  char text[10];

  // set reference for ADC to power supply (5V)
  analogReference(DEFAULT);

  // enable measuring for high temperatures
  digitalWrite(HIGH_TEMP_EN, LOW);
  digitalWrite(LOW_TEMP_EN, HIGH);
  delayMicroseconds(1000);

  for (int i = 0; i < 16; i++) {
    avgTemp += analogRead(TEMP_IN);
  }

  // read averaged analog value of temperature
  long tempADU;

  // decide if high temperatures are possible to measure, 980 according to ~160 deg. C
  if ((avgTemp >> 4) > 980) {   
    // enable measuring for low temperatures
    digitalWrite(HIGH_TEMP_EN, HIGH);
    digitalWrite(LOW_TEMP_EN, LOW);
    delayMicroseconds(1000);

    avgTemp = 0;
    for (int i = 0; i < 16; i++) {
      avgTemp += analogRead(TEMP_IN);
    }
    
    // convert ADU into temperature (multiplied by 10 -> 200deg = 2000)
    // constants could slightly change for different ceramic tip
    // T = -0.1232*ADU + 146.1
    // Low temperatures (approx. 40 - 120 deg.)
    tempADU = avgTemp;
    tempADU *= -1261;   // -1261 = -0.1232*1024
    tempADU >>=  14;
    tempADU += 1461; 

  }
  else {
    // convert ADU into temperature
    // constants could slightly change for different ceramic tip
    // T = -0.2059*ADU + 351.4
    // High temperatures (approx. 180 - 280 deg.)
    tempADU = avgTemp;
    tempADU *= -2108;   // -2108 = -0.2059*1024
    tempADU >>=  14;
    tempADU += 3514;
  }

  return tempADU;
}

/*
     load actual material profile
*/
void loadMaterial(int id) {
  profile_t profile;
  char text[10];

  // load material profile from PROGMEM and assign variables
  memcpy_P(&profile, &materials[id], sizeof(profile_t));
/*
  setTemperature = profile.temperature;
  setMotorSpeed  = profile.motorSpeed;
*/
  // clear display and show all information
/*  sprintf(text, "%d %%", setMotorSpeed);
  ssd1306_clearScreen();
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixedN(0,  0, profile.materialName, STYLE_NORMAL, FONT_SIZE_2X);
  ssd1306_printFixedN(80, 0, text, STYLE_NORMAL, FONT_SIZE_2X);*/

}

/*
    PID variables and constants for tuning
*/
float Kp = 3.2, Ki = 0.33, Kd = 0.14, dT = 0.1, Hz = 10;

/*
     basic PID routine to get output value
*/
int getPIDoutput(int setPoint, int actualValue, int maxValue, int minValue) {
  static float sumE = 0;
  static int16_t error, previousError = 0;
  float outputValue;
  static int pidAvg[4] = {0, 0, 0, 0};
  static int pidAvgIndex = 0;

  // reset sumE when actualValue exceed setPoint by 5
  static int noWaitCycles = 0;
  
  if (actualValue > setPoint + 50) {
    ++noWaitCycles;
    if (noWaitCycles >= 30) {
      sumE = maxValue;
      noWaitCycles = 0;
    }
  }
  else {
    noWaitCycles = 0;
  }

  // PID implementation
  error = setPoint - actualValue;
  sumE += (float) error * dT;
  outputValue =  Kp * error + Ki * sumE + Kd * (error - previousError) / dT;
  previousError = error;

  // restrict output PID value into range between minValue and maxValue
  if (outputValue > maxValue)
    outputValue = maxValue;
  else if (outputValue < minValue)
    outputValue = minValue;

  // store n output values for averaging
  pidAvg[pidAvgIndex] = outputValue;
  ++pidAvgIndex;
  if (pidAvgIndex >= 4)
    pidAvgIndex = 0;

  // average last n output values
  int sumPIDavg = 0;
  for (int i = 0; i < 4; i++) {
    sumPIDavg += pidAvg[i];
  }
  sumPIDavg >>= 2;

  return sumPIDavg;
}

#define NO_AVERAGES_VALUES  64

/*
     heating function for heater driving by PID regulator
*/
int heating() {
  static int tempAvg[NO_AVERAGES_VALUES];   // temperature array for averaging it
  static int tempAvgIter = 0;               // current index in temperature array
  static char firstTime = 0;                // if is 1, this function ran at least one time
  char text[30];                            // buffer for text

  // variables initialization
  if (!firstTime) {
    memset(tempAvg, 0, NO_AVERAGES_VALUES*sizeof(int));
    firstTime = 1;
  }

  // resolve PID value for heater PWM
  int temperature = getTemperature();
  int valuePID = getPIDoutput(setTemperature*10, temperature, 255, 0);

  analogWrite(HEATER_EN, 255 - valuePID);

  // save actual temperature for averaging
  tempAvg[tempAvgIter] = temperature;
  if (++tempAvgIter >= NO_AVERAGES_VALUES)
    tempAvgIter = 0;

  // make temperature average from NO_AVERAGES_VALUES
  long sumTemp = 0;
  for (int i = 0; i < NO_AVERAGES_VALUES; i++) {
    sumTemp += tempAvg[i];
  }
  sumTemp /= NO_AVERAGES_VALUES;

 
displayControls();
 // show on display actual and preset temperature
  sprintf(text, "%3d", (int)sumTemp/10);
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixedN(0, 0, text, STYLE_NORMAL, FONT_SIZE_2X);
  ssd1306_printFixedN(39, 12, "C", STYLE_NORMAL, FONT_SIZE_NORMAL);


 return sumTemp;
}

void acs3Ddrawing() {
  // decide temperature state (heating, cooling, ready) and show it on display
  if (++elapsedTime == 2) { // 100ms
    elapsedTime = 0;
    int actualTemperature = heating();

    // tolerant zone where temperature is ABOVE preset temperature,
    // but it is possible to do extrusion/reverse
    if (actualTemperature > setTemperature*10 + 100) {
      statusHeating = STATE_COOLING;
      ssd1306_printFixedN(64, 0, "Cooling", STYLE_NORMAL, FONT_SIZE_NORMAL);
    }

    // tolerant zone where temperature is OK for extrusion/reverse
    else if (actualTemperature > setTemperature*10 - 100) {
      statusHeating = STATE_READY;
      ssd1306_printFixedN(64, 0, "Ready  ", STYLE_NORMAL, FONT_SIZE_NORMAL); //spaces used instead of clearing space as heating is longer
      digitalWrite(LED_R, HIGH);   // turn the LED on (HIGH is the voltage level)
      digitalWrite(LED_L, HIGH);   // turn the LED on (HIGH is the voltage level)
    }

    // tolerant zone where temperature is LOW for extrusion/reverse
    else {
      statusHeating = STATE_HEATING;
      ssd1306_printFixedN(64, 0, "Heating", STYLE_NORMAL, FONT_SIZE_NORMAL);
      digitalWrite(LED_R, !digitalRead(LED_R));   // turn the LED on (HIGH is the voltage level)
      digitalWrite(LED_L, !digitalRead(LED_L));   // turn the LED on (HIGH is the voltage level)
    }
  }

  // assing functions according to heating state (mainly button function)
  switch (statusHeating) {
    case STATE_COOLING:
    case STATE_READY: {
        // button EXTRUSION is pressed, extrude material
        if (!digitalRead(BTN_EXT) && digitalRead(BTN_REV)) {
          stateMotor = MOTOR_EXTRUSION;
        }

        // button REVERSE is pressed, retract material
        else if (digitalRead(BTN_EXT) && !digitalRead(BTN_REV)) {
          stateMotor = MOTOR_REVERSE;
          timeMotorReverse = 400; // reverse time is 50ms * timeMotorReverse (400 = 20s)
        }

        // both buttons are pressed, motor stopped
        else if (!digitalRead(BTN_EXT) && !digitalRead(BTN_REV)) {
          stateMotor = MOTOR_STOP;
        }
        
        // no buttons are pressed
        else {
          if (lastMotorState == MOTOR_EXTRUSION) {
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
  switch (stateMotor) {
    case MOTOR_STOP:
      digitalWrite(MOTOR_DIR, LOW);
      analogWrite(MOTOR_PWM, 0);
      break;

    case MOTOR_EXTRUSION: {
        int pwmSpeed = setMotorSpeed * 255;
        digitalWrite(MOTOR_DIR, LOW);
        analogWrite(MOTOR_PWM, pwmSpeed / 100);
        break;
      }

    case MOTOR_REVERSE:
      --timeMotorReverse;
      if (timeMotorReverse > 0) {
        digitalWrite(MOTOR_DIR, HIGH);
        analogWrite(MOTOR_PWM, 0);
      }
      else {
        stateMotor = MOTOR_STOP;
      }
      break;

    case MOTOR_REVERSE_AFTER_EXTRUSION:
      --timeMotorReverse;
      if (timeMotorReverse > 0) {
        int pwmSpeed = (100 - setMotorSpeed) * 255;
        digitalWrite(MOTOR_DIR, HIGH);
        analogWrite(MOTOR_PWM, pwmSpeed / 100);
      }
      else {
        stateMotor = MOTOR_STOP;
      }
      break;
  }
  lastMotorState = stateMotor;

  // one time action, mainly for material change
  static char buttonsPressed = 0;

 // The Mode Control Buttons
  // (note: buttons are LOW/0/false when PRESSED!)
  //// UP && DOWN are pressed
  if(!digitalRead(BTN_UP) && !digitalRead(BTN_DOWN)) {    // Are both Buttons pressed?
    
    if ((~buttonsPressed & B10000000)) {                  // was I NOT pressed in the LAST cycle? = on time only
      if (controlMode == MODE_TEMP) {
        controlMode = MODE_SPEED;
      } else {
        controlMode = MODE_TEMP;
      }
      displayControls();
   }
   buttonsPressed |= B10000000;                           // set both buttons to "was pressed", new flag instead of 00000011!

  } else { // else = not BOTH are pressed at the same time -> UP || DOWN || NONE
    
    //// UP Button on Release Action
    if(!digitalRead(BTN_UP)) {                            // if UP pressed
      buttonsPressed |= B00000001;                        // set UP to pressed
    } else {
      if (!(buttonsPressed & B10000000) && (buttonsPressed & B00000001) && digitalRead(BTN_DOWN)) {  // was I pressed in the LAST cycle? = onRelease
        if (controlMode == MODE_TEMP) {
          if (setTemperature == 0) {
            setTemperature = MINTEMP;
          } else if (setTemperature <= MAXTEMP - 5){
            setTemperature += 5;
          }
        } else {
          if (setMotorSpeed <= MAXSPEED - 5) {
            setMotorSpeed += 5;            
          }
        }
        displayControls();
      }
      buttonsPressed &= B11111110;                        // set UP to released
    }
    
    //// DOWN Button on Release action
    if(!digitalRead(BTN_DOWN)) {                          // if DOWN pressed
      buttonsPressed |= B00000010;                        // set DOWN to pressed
    } else {
      if (!(buttonsPressed & B10000000) && (buttonsPressed & B00000010) && digitalRead(BTN_UP)) {  // was I pressed in the LAST cycle? = onRelease
        if (controlMode == MODE_TEMP) {
          if (setTemperature == MINTEMP) {
            setTemperature = 0;
          } else if (setTemperature >= MINTEMP + 5) {
            setTemperature -= 5;
          }
        } else {
          if (setMotorSpeed >= 0 +5) {
            setMotorSpeed -= 5;            
          }
        }
        displayControls();
      }
      buttonsPressed &= B11111101;                        // set DOWN to released
    }

    //// NONE
    if (digitalRead(BTN_UP) && digitalRead(BTN_DOWN)) {
      buttonsPressed &= B01111111;                          // BOTH release
    }
    
  } // end of if-UP&&DOWN-are-pressed-else...
  
    
}

void displayControls() {
  
  char textSetTemp[5]; // Buffers for formatted control input text
  char textSetMotor[5];
  if (setTemperature >= MINTEMP) {
    sprintf(textSetTemp,"%3d ", setTemperature);
  } else {
    sprintf(textSetTemp,"OFF ");
  }
  
  sprintf(textSetMotor,"%3d ",setMotorSpeed);
  
  // clear display and show all information
  /*ssd1306_clearScreen();*/
  ssd1306_setFixedFont(ssd1306xled_font6x8);

  if (controlMode == MODE_TEMP) {
    ssd1306_negativeMode();
    ssd1306_printFixedN(0, 16, textSetTemp, STYLE_NORMAL, FONT_SIZE_2X);
    ssd1306_printFixedN(36+3, 24, "C", STYLE_NORMAL, FONT_SIZE_NORMAL);
    ssd1306_positiveMode();
    ssd1306_printFixedN(68, 16, textSetMotor, STYLE_NORMAL, FONT_SIZE_2X);
    ssd1306_printFixedN(110, 24, "%", STYLE_NORMAL, FONT_SIZE_NORMAL);
  } else {
    ssd1306_printFixedN(0, 16, textSetTemp, STYLE_NORMAL, FONT_SIZE_2X);
    ssd1306_printFixedN(36+3, 24, "C", STYLE_NORMAL, FONT_SIZE_NORMAL);
    ssd1306_negativeMode();
    ssd1306_printFixedN(68, 16, textSetMotor, STYLE_NORMAL, FONT_SIZE_2X);
    ssd1306_printFixedN(110, 24, "%", STYLE_NORMAL, FONT_SIZE_NORMAL);
    ssd1306_positiveMode();
  }

  ssd1306_printFixedN(60, 16, "<", STYLE_NORMAL, FONT_SIZE_NORMAL);   // icon in the middle
  ssd1306_printFixedN(60, 24, ">", STYLE_NORMAL, FONT_SIZE_NORMAL);

}


void acsBurning() {
  percentRoutine();
}

void acsSoldering() {
  percentRoutine();
}

void acsFoamCutting() {
  percentRoutine();
}

void acsNone() {
  ;
}

const accessories_t accessoriesList[] = {
  //     ID                 type                  display name      function                    startup
  ACCS(ACS_ID_3D_DRAW,      ACS_3D_DRAW,          "3D drawing",     (callBack)acs3Ddrawing,     (callBack)startup3D),
  ACCS(ACS_ID_BURNING,      ACS_BURNING,          "Burning",        (callBack)acsBurning,       (callBack)startupBurning),
  ACCS(ACS_ID_SOLDERING,    ACS_SOLDERING,        "Soldering",      (callBack)acsSoldering,     (callBack)startupSoldering),
  ACCS(ACS_ID_FOAM_CUTTING, ACS_FOAM_CUTTING,     "Foam cutt.",     (callBack)acsFoamCutting,   (callBack)startupFoamCutting),
  ACCS(ACS_ID_NONE,         ACS_NONE,             "None",           (callBack)acsNone,          (callBack)startupNone)
};

const char noAccs = sizeof(accessoriesList) / sizeof(accessories_t);

void acsIdentify(void) {
  accessories_t input;
  static accessories_t lastInput;
  char id            = 0;
  static char idIter = NO_ITERATIONS;
  int i;

  NanodeUNIO(0xA0);
  if (read(&id, 0x00, 1)) {
    for (i = 0; i < noAccs; i++) {
      memcpy(&input, &(accessoriesList[i]), sizeof(accessories_t));
      if (input.ID == id) {
        idIter    = 0;
        lastInput = input;
        break;
      }
    }

    if (i == noAccs) {
      if (++idIter >= NO_ITERATIONS) {
        idIter = 0;
        memcpy(&input, &(accessoriesList[noAccs - 1]), sizeof(accessories_t));
        lastInput = input;
      } else {
        input = lastInput;
      }
    }
  }
  else {
    if (++idIter >= NO_ITERATIONS) {
      idIter = 0;
      memcpy(&input, &(accessoriesList[noAccs - 1]), sizeof(accessories_t));
      lastInput = input;
    } else {
      input = lastInput;
    }
  }

  accessories = input;
}

accessories_t getAccessories(void) {
  return accessories;
}
