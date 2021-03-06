#include "modules.h"
#include "mbed.h"
#include "TextLCD.h"
#include "utility.h"
#include "motor.h"

#include <string>
#include <math.h>

#ifdef UNIT_TESTING
string p21 = "STP_B", p22 = "STP_A", p23 = "redLED", p24 = "greenLED", p25 = "A7", p26 = "A6", p27 = "A5", p28 = "A4", p29 = "LCD_RS", p30 = "LCD_EN",
p5 = "buzzer", p6 = "p6", p7 = "mtrB_EN", p8 = "DIR_B", p9 = "DIR_A", p10 = "mtrA_EN", p11 = "M0", p12 = "M1", p13 = "M2", p14 = "heater",
p15 = "toggle1", p16 = "SW2", p17 = "SW3", p18 = "SW4", p19 = "temp", p20 = "sal";
#endif

// RS, ENABLE, A4, A5, A6, A7
TextLCD lcd(p29, p30, p28, p27, p26, p25);

// ENABLE, M0, M1, M2, STP, DIR, yellowUpLED
Motor mtrA(p10, p11, p12, p13, p22, p9, p22);

// ENABLE, M0, M1, M2, STP, DIR, yellowDownLED
Motor mtrB(p7, p11, p12, p13, p21, p8, p21);

DigitalOut heater(p14);
DigitalOut greenLED(p24);
DigitalOut redLED(p23);
DigitalOut buzzer(p5);

// When switched down, the corresponding pin SW1/SW2 will output 3.3V;
// When switched up, the pin is connected to the ground (0V).
DigitalIn toggle1(p15); // SW1
DigitalIn toggle2(p16); // SW2

// Their corresponding pins normally output 3.3V, but when pushed down they switch to 0V.
DigitalIn switch3(p17); // SW3
DigitalIn switch4(p18); // SW4

// Sensors
AnalogIn temperatureSensor(p19);
AnalogIn salinitySensor(p20);

// Current water level of beaker
int waterLevel = MIN_WATER;
int injectorAlevel, injectorBlevel;

Timer timerMotor, timerInjectFreq;

int numInjections = 0;

void manualMotorControl(Motor* mtr)
{
    mtr->setDirection((mtr == &mtrA) ? INJECTA : INJECTB);

    if (switchDown(switch4) && mtr->currentState == Motor::idle) {
        mtr->initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED);
    } else if (!switchDown(switch4) && mtr->currentState != Motor::idle) {
        mtr->currentState = Motor::deaccelerate;
    }
}

void run()
{
    activateBuzzer(false);
    float Vtemp, Vsal, temp, sal;
    string errors = "";

    Vtemp = temperatureSensor.read() * 3.3;
    Vsal = salinitySensor.read() * 3.3 * (5.f / 3.f);

    temp = get_temperature(Vtemp);
    sal = get_salinity(Vsal);

    errors = getErrorsforLCD(temp, sal, waterLevel);
    lcd.cls();
    if (errors.empty()) {
        lcd.cls();
        lcd.printf("Temp: %.1f deg\nSal: %.1f ppt", temp, sal);
    } else {
        displayOnLCD(errors.c_str());
    }

    // Heater + red LED
    checkTemperature(temp);

    // Green LED
    checkRanges(temp, sal, waterLevel);

    float timePassed = timerInjectFreq.read();
    if (timePassed < 30 && numInjections < 5) {
        if (sal < MIN_INJECT_SAL && mtrB.currentState == Motor::idle) {
            mtrB.setDirection(INJECTB);
            mtrB.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_1ML_MTRB);
            injectorBlevel--;
            numInjections++;

        } else if (sal > MAX_INJECT_SAL && mtrA.currentState == Motor::idle) {
            mtrA.setDirection(INJECTA);
            mtrA.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_1ML_MTRA);
            injectorAlevel--;
            numInjections++;
        }
    } else if (timePassed >= 30) {
        timerInjectFreq.reset();
        numInjections = 0;
    }

    // print the percentage and 16 bit normalized values
    if (mtrA.currentState == Motor::idle && mtrB.currentState == Motor::idle) {
        if (errors.empty()) {
            wait(1);
        } else {
            buzzInterval();
        }
    } else {
        if (errors.empty()) {
            timerMotor.reset();
            while (timerMotor.read() < 1) {
                mtrA.update(mtrB.currentState == mtrB.idle);
                mtrB.update(true);
            }
        } else {
            timerMotor.reset();
            while (timerMotor.read() < 0.5) {
                mtrA.update(mtrB.currentState == mtrB.idle);
                mtrB.update(true);
            }
            activateBuzzer(true);

            timerMotor.reset();
            while (timerMotor.read() < 0.5) {
                mtrA.update(mtrB.currentState == mtrB.idle);
                mtrB.update(true);
            }
            activateBuzzer(false);
        }
    }

    if ((mtrA.currentState == mtrA.idle && injectorAlevel == 0) || (mtrB.currentState == mtrB.idle && injectorBlevel == 0)) {
        currentMode = Refill;
    }
}

void refillInjectors()
{
    mtrA.setDirection(!INJECTA);
    mtrB.setDirection(!INJECTB);

    displayOnLCD("Refilling...");
    while (mtrA.currentState != Motor::idle || mtrB.currentState != Motor::idle) {
        baseFunctions();
        timerMotor.reset();
        while (timerMotor.read() < 1) {
            mtrA.update(mtrB.currentState == mtrB.idle);
            mtrB.update(true);
        }
    }
}

void setup()
{
    displayOnLCD("Setup Mode");

    wait(2);

    displayOnLCD("Turn valves to\nposition R");
    buzzInterval();
    buzzInterval();

    // LCD: valves to position R
    waitForConfirmation();

    // Manually empty injector A
    mtrA.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED + 1000);
    mtrA.setDirection(INJECTA);
    while (!switchDown(switch3)) { // Confirmation
        displayOnLCD("Empty Injector Awith Switch 8");
        manualMotorControl(&mtrA);
        timerMotor.reset();
        while (timerMotor.read() < 0.5) {
            mtrA.update(true);
        }
    }
    wait(1);

    // Manually empty injector A
    mtrB.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED + 1000);
    mtrB.setDirection(INJECTB);
    while (!switchDown(switch3)) { // Confirmation
        displayOnLCD("Empty Injector Bwith Switch 8");
        manualMotorControl(&mtrB);
        timerMotor.reset();
        while (timerMotor.read() < 0.5) {
            mtrB.update(true);
        }
    }
    wait(1);

    // Fully fill injectors.
    mtrA.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRA);
    mtrB.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRB);
    refillInjectors();
    injectorAlevel = 35;
    injectorBlevel = 35;

    displayOnLCD("Turn valves to\nposition T");
    buzzInterval();
    buzzInterval();

    // LCD: valves to position T
    waitForConfirmation();
    displayOnLCD("Setup Complete");

    currentMode = Functional;
    timerInjectFreq.start();
}

void refill()
{
    string valves;

    if (injectorAlevel == 0 && injectorBlevel == 0) {
        valves = "valves";
        mtrA.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRA);
        mtrB.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRB);
        injectorAlevel = 35;
        injectorBlevel = 35;
    } else if (injectorAlevel == 0) {
        valves = "valve A";
        mtrA.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRA);
        injectorAlevel = 35;
    } else { // injectorBlevel == 0
        valves = "valve B";
        mtrB.initializeMove(1 / MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRB);
        injectorBlevel = 35;
    }

    displayOnLCD("Turn %s to\nposition R", valves.c_str());

    // Buzzer for two seconds while still checking for heater and green LED every second.
    buzzInterval();
    baseFunctions();
    buzzInterval();

    // Wait for confirmation.
    while (!switchDown(switch3)) {
        baseFunctions();
    }

    refillInjectors();

    displayOnLCD("Turn %s to\nposition T", valves.c_str());
    buzzInterval();
    baseFunctions();
    buzzInterval();

    // Wait for confirmation.
    while (!switchDown(switch3)) {
        baseFunctions();
    }

    currentMode = Functional;
    timerInjectFreq.reset();
}
