#ifndef MODULES_H
#define MODULES_H

#include "mbed.h"
#include "TextLCD.h"
#include "motor.h"
#include <string>

// Injection direction for Motor A
#define INJECTA 1

// Injection direction for Motor B
#define INJECTB 0

#define STEPS_FOR_1ML_MTRA 4849
#define STEPS_FOR_1ML_MTRB 4849
#define STEPS_FOR_35ML_MTRA 218750
#define STEPS_FOR_35ML_MTRB 211435

// Values defined in requirements.
#define MIN_SAL 8.2
#define MAX_SAL 8.7
#define DESIRED_SAL 7.5
#define MIN_TEMP 35
#define MAX_TEMP 42
#define DESIRED_TEMP 37
#define MIN_WATER 400
#define MAX_WATER 500

// string p21="p21", p22="p22", p23="p23", p24="greenLED", p25="p25", p26="p26", p27="p27", p28="p28", p29="p29", p30="p30";
// string p5="p5", p6="p6", p7="p7", p8="p8", p9="p9", p10="p10", p11="p11", p12="p12", p13="p13", p14="heater", p15="p15", p16="p16", p17="p17", p18="p18", p19="p19", p20="p20";

// RS, ENABLE, A4, A5, A6, A7
extern TextLCD lcd;

// ENABLE, M0, M1, M2, STP, DIR, yellowUpLED
extern Motor mtrA;

// ENABLE, M0, M1, M2, STP, DIR, yellowDownLED
extern Motor mtrB;

extern DigitalOut heater;
extern DigitalOut greenLED;
extern DigitalOut redLED;
extern DigitalOut buzzer;

// When switched down, the corresponding pin SW1/SW2 will output 3.3V;
// When switched up, the pin is connected to the ground (0V).
extern DigitalIn toggle1; // SW1
extern DigitalIn toggle2; // SW2

// Their corresponding pins normally output 3.3V, but when pushed down they switch to 0V.
extern DigitalIn switch3; // SW3
extern DigitalIn switch4; // SW4

// Sensors
extern AnalogIn temperatureSensor;
extern AnalogIn salinitySensor;

// Current water level of beaker
extern int waterLevel;
extern int injectorAlevel, injectorBlevel;

extern Timer timerMotor, timerInjectFreq;
extern int numInjections;

enum Mode
{
    Setup,
    Functional,
    Refill
};
extern Mode currentMode;

extern void manualMotorControl(Motor *mtr);
extern void run();
extern void refillInjectors();
extern void setup();
extern void refill();

#endif
