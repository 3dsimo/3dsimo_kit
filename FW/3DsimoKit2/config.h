
#ifndef _CONFIG_H
#define _CONFIG_H

/*
 *   define Input/Outputs
 */

#define LED_L         13    // LED placed on Arduino Nano board
#define LED_R         4     // LED placed on 3Dsimo KIT opposite to LED_NANO

#define BTN_UP        12    // controlling button UP/PLUS                /* NEW! changed for right hand - default 11 */
#define BTN_DOWN      11    // controlling button DOWN/MINUS             /* NEW! changed for right hand - default 12 */
#define BTN_EXT       7     // button for material extrusion
#define BTN_REV       8     // button for material reverse

#define MOTOR_DIR     6     // motor direction output
#define MOTOR_PWM     10    // motor PWM output for power driving
#define MOTOR_SLEEP   5
#define HEATER_EN     9     // heater/power output 

#define TEMP_IN       A0    // temperature measure ADC input 

#define LOW_TEMP_EN   3     // enable measurement for low temperatures (~from 40deg. Celsius)
#define HIGH_TEMP_EN  2     // enable measurement for high temperatures (~from 150deg. Celsius)

#define ID_PIN        16    // Pin for identification of the accessories

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

#endif
