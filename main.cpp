#include "mbed.h"
#include "TextLCD.h"
#include "motor.h"
#include <string>
#include <math.h>

// Injection direction for Motor A
#define INJECTA 1

// Injection direction for Motor B
#define INJECTB 0

#define STEPS_FOR_1ML 250

// Values defined in requirements.
#define MIN_SAL 7
#define MAX_SAL 14
#define MIN_TEMP 35
#define MAX_TEMP 42
#define DESIRED_TEMP 37
#define MIN_WATER 250
#define MAX_WATER 450
#define MSG_TIME 3

// string p21="p21", p22="p22", p23="p23", p24="greenLED", p25="p25", p26="p26", p27="p27", p28="p28", p29="p29", p30="p30";
// string p5="p5", p6="p6", p7="p7", p8="p8", p9="p9", p10="p10", p11="p11", p12="p12", p13="p13", p14="heater", p15="p15", p16="p16", p17="p17", p18="p18", p19="p19", p20="p20";

// 1 if manual motor control is active, 0 when deactivated.
bool manualMotor = true;

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
AnalogIn toggle1(p15); // SW1
AnalogIn toggle2(p16); // SW2

// Their corresponding pins normally output 3.3V, but when pushed down they switch to 0V.
AnalogIn switch3(p17); // SW3
AnalogIn switch4(p18); // SW4

// Sensors
AnalogIn temperatureSensor(p19);
AnalogIn salinitySensor(p20);

// Current water level of beaker
int waterLevel = MIN_WATER;

Timer timer;

float get_temperature(float Vin)
{
    return -30.79759812 * Vin + 83.90646865;
}

float get_salinity(float Vin)
{
    return 13.40769944 * exp(0.39797502 * Vin) - 12.3251654;
}

/**
 * @brief Return true if the toggle is down, false if up
 *
 * @param tgl Toggle
 */
bool toggleDown(AnalogIn tgl)
{
    return tgl.read() > 0.5;
}

/**
 * @brief Return true if the switch is pressed down, false else
 *
 * @param sw Switch
 */
bool switchDown(AnalogIn sw)
{
    return sw.read() < 0.5;
}

/**
 * @brief Activates / Deactivates the buzzer, if toggle 1 is switched on
 *
 * @param on Boolean to switch it on or off
 */
void activateBuzzer(bool on)
{

    if(!toggleDown(toggle1)) {
        buzzer = false;
    } else {
        buzzer = on;
    }
}

string getErrorsforLCD(float temp, float sal, int waterLevel)
{
    string err = "";
    if (waterLevel < MIN_WATER) {
        err += "Water level low ";
        // activateBuzzer(true);
    } else if (waterLevel > MAX_WATER) {
        err += "Water level high";
        // activateBuzzer(true);
    } else {
        // activateBuzzer(false);
    }

    if (sal < MIN_SAL) {
        err += "Low salinity    ";
        // activateBuzzer(true);

    } else if (sal > MAX_SAL) {
        err += "High salinity   ";
        // activateBuzzer(true);
    } else {
        // activateBuzzer(false);
    }

    // If less than than two messages have been added to the err string.
    if (err.length() <= 16) {
        if (temp <= MIN_TEMP) {
            err += "Temperature low ";
        } else if (temp >= MAX_TEMP) {
            err += "Temperature high";
        }
    }

    return err;
}

void displayDefaultLCD(float Vtemp, float Vsal, float temp, float sal)
{
    if (!toggleDown(toggle1)) {
        lcd.printf("Temp: %.1f deg\n", temp);
        lcd.printf("Sal: %.1f ppt", sal);
    } else {
        lcd.printf("Temp: %.1f V\n", Vtemp);
        lcd.printf("Sal: %.1f V", Vsal);
    }
}

void manualMotorControl()
{
    int dirA, dirB;

    // If switch up
    if (!toggleDown(toggle2)) {
        mtrA.setDirection(INJECTA);
        mtrB.setDirection(INJECTB);
    } else {
        mtrA.setDirection(!INJECTA);
        mtrB.setDirection(!INJECTB);
    }

    // If switch pushed down.
    if (switchDown(switch3) && mtrA.currentState == Motor::idle) {
        mtrA.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED);
        printf("Initialize motor A\r\n");
    } else if (!switchDown(switch3) && mtrA.currentState != Motor::idle) {
        mtrA.currentState = Motor::deaccelerate;
        printf("Deaccelerate motor A\r\n");
    }

    if (switchDown(switch4) && mtrB.currentState == Motor::idle) {
        mtrB.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED);
    } else if (!switchDown(switch4) && mtrB.currentState != Motor::idle) {
        mtrB.currentState = Motor::deaccelerate;
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

    // printf("Motor A ");
    // mtrA.printState();
    // printf("Motor B ");
    // mtrB.printState();

    // LCD
    lcd.cls();
    // displayDefaultLCD(Vtemp, Vsal, temp, sal);
    errors = getErrorsforLCD(temp, sal, waterLevel);
    if (errors.empty()) {
        displayDefaultLCD(Vtemp, Vsal, temp, sal);
    } else {
        lcd.printf("%s", errors);
    }

    // Heater
    heater = temp <= DESIRED_TEMP;

    // LEDs
    redLED = heater;
    greenLED = temp > MIN_TEMP && temp < MAX_TEMP&&
        sal >= MIN_SAL && sal <= MAX_SAL &&
        waterLevel >= MIN_WATER && waterLevel <= MAX_WATER;

    if (manualMotor) {
        manualMotorControl();
    } else {
        if (sal < MIN_SAL && mtrA.currentState == Motor::idle) {
            mtrA.setDirection(INJECTA);
            mtrA.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_1ML);
        } else if (sal > MAX_SAL && mtrB.currentState == Motor::idle) {
            mtrB.setDirection(INJECTB);
            mtrB.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_1ML);
        }
    }

    // print the percentage and 16 bit normalized values
    if (mtrA.currentState == Motor::idle && mtrB.currentState == Motor::idle) {
        if (errors.empty()) {
            wait(1);
        } else {
            wait(0.5);
            activateBuzzer(true);
            wait(0.5);
            activateBuzzer(false);
        }

    } else {
        if (errors.empty()) {
            timer.reset();
            while (timer.read() < 1) {
                mtrA.update(mtrB.idle);
                mtrB.update(true);
            }
        } else {
            timer.reset();
            while (timer.read() < 0.5) {
                mtrA.update(mtrB.idle);
                mtrB.update(true);
            }
            activateBuzzer(true);

            timer.reset();
            while (timer.read() < 0.5) {
                mtrA.update(mtrB.idle);
                mtrB.update(true);
            }
            activateBuzzer(false);
        }
    }
}

int main(int argc, char const* argv[])
{
    float timeElapsed = 0.;
    timer.start();

    while (1) {
        run();
    }
}
