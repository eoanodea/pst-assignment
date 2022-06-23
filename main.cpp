#include "mbed.h"
#include "TextLCD.h"
#include "utility.h"
#include "motor.h"
#include <string>
#include <math.h>

// string p21="p21", p22="p22", p23="p23", p24="greenLED", p25="p25", p26="p26", p27="p27", p28="p28", p29="p29", p30="p30";
// string p5="p5", p6="p6", p7="p7", p8="p8", p9="p9", p10="p10", p11="p11", p12="p12", p13="p13", p14="heater", p15="p15", p16="p16", p17="p17", p18="p18", p19="p19", p20="p20";

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

enum Mode { Setup, Functional, Refill };
Mode currentMode = Setup;

Timer timerMotor, timerInjectFreq;

int numInjections = 0;



// void displayDefaultLCD(float Vtemp, float Vsal, float temp, float sal)
// {
//     if (!toggleDown(toggle1)) {
//         lcd.printf("Temp: %.1f deg\n", temp);
//         lcd.printf("Sal: %.1f ppt", sal);
//     } else {
//         lcd.printf("Temp: %.1f V\n", Vtemp);
//         lcd.printf("Sal: %.1f V", Vsal);
//     }
// }

void manualMotorControl(Motor *mtr)
{
    mtr->setDirection((mtr == &mtrA) ? INJECTA : INJECTB);

    if (switchDown(switch4) && mtr->currentState == Motor::idle) {
        mtr->initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED-1000);
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

    // printf("Motor A ");
    // mtrA.printState();
    // printf("Motor B ");
    // mtrB.printState();

    errors = getErrorsforLCD(temp, sal, waterLevel);
    lcd.cls();
    lcd.printf("Temp: %.1f deg\nSal: %.1f ppt", temp, sal);
    // if (errors.empty()) {
        // lcd.cls();
    //     lcd.printf("Temp: %.1f deg\nSal: %.1f ppt", temp, sal);
    // } else {
    //     displayOnLCD(errors.c_str());
    // }

    // Heater + red LED
    checkTemperature(temp);

    // Green LED
    checkRanges(temp, sal);

    float timePassed = timerInjectFreq.read();
    if (timePassed < 60 && numInjections < 5) {
        if (sal < MIN_SAL && mtrA.currentState == Motor::idle) {
            mtrB.setDirection(INJECTB);
            mtrB.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED-1000, STEPS_FOR_1ML_MTRB);
            injectorBlevel--;
            numInjections++;
        } else if (sal > MAX_SAL && mtrB.currentState == Motor::idle) {
            mtrA.setDirection(INJECTA);
            mtrA.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED-1000, STEPS_FOR_1ML_MTRA);
            injectorAlevel--;
            numInjections++;
        }
    } else if (timePassed > 60) {
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
                mtrA.update(mtrB.idle);
                mtrB.update(true);
            }
        } else {
            timerMotor.reset();
            while (timerMotor.read() < 0.5) {
                mtrA.update(mtrB.idle);
                mtrB.update(true);
            }
            activateBuzzer(true);

            timerMotor.reset();
            while (timerMotor.read() < 0.5) {
                mtrA.update(mtrB.idle);
                mtrB.update(true);
            }
            activateBuzzer(false);
        }
    }

    if (injectorAlevel == 0 || injectorBlevel == 0) {
        currentMode = Refill;
    }
}

void refillInjectors()
{
    mtrA.setDirection(!INJECTA);
    mtrB.setDirection(!INJECTB);
    displayOnLCD("Refilling...");
    while (mtrA.currentState != Motor::idle && mtrB.currentState != Motor::idle) {
        baseFunctions();
        timerMotor.reset();
        while (timerMotor.read() < 1) {
            mtrA.update(mtrB.idle);
            mtrB.update(true);
        }
    }

    injectorAlevel = 35;
    injectorBlevel = 35;
}

void setup()
{
    displayOnLCD("Setup Mode");

    wait(2);

    displayOnLCD("Turn valves to\nposition R");
    buzzInterval(); buzzInterval();

    // LCD: valves to position R
    while(!switchDown(switch3)) { } // Confirmation
    wait(0.5);

    // Manually empty injector A

    mtrA.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED);
    mtrA.setDirection(INJECTA);
    while(!switchDown(switch3)) { // Confirmation
        displayOnLCD("Empty Injector Awith Switch 8");
        manualMotorControl(&mtrA);
        timerMotor.reset();
        while (timerMotor.read() < 0.5) {
            mtrA.update(true);
        }
    }
    wait(0.5);

   // Manually empty injector A

    mtrB.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED);
    mtrB.setDirection(INJECTB);
    while(!switchDown(switch3)) { // Confirmation
        displayOnLCD("Empty Injector Bwith Switch 8");
        manualMotorControl(&mtrB);
        timerMotor.reset();
        while (timerMotor.read() < 0.5) {
             mtrB.update(true);
         }
    }
    wait(0.5);

    // Fully fill injectors.
    refillInjectors();

    displayOnLCD("Turn valves to\nposition T");
    buzzInterval(); buzzInterval();

    // LCD: valves to position T
    while(!switchDown(switch3)) { } // Confirmation
    wait(0.5);
    displayOnLCD("Setup Complete");

    currentMode = Functional;
    timerInjectFreq.start();
}



void refill()
{
    string valves;

    printf("%i %i \r\n", injectorAlevel, injectorBlevel);
    
    if (injectorAlevel == 0 && injectorBlevel == 0) {
        valves = "valves";
        mtrA.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRA);
        mtrB.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRB);
    } else if (injectorAlevel == 0) {
        valves = "valve A";
        mtrA.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRA);
    } else { // injectorBlevel == 0
        valves = "valve B";
        mtrB.initializeMove(1/MICROSTEPS_PER_STEP, MAX_SPEED, STEPS_FOR_35ML_MTRB);
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
}


int main(int argc, char const* argv[])
{
    printf("\r\n\r\n----------V1.6-----------\r\n\r\n");

    float timeElapsed = 0.;
    timerMotor.start();

    while (1) {
        switch (currentMode) {
            case Setup:
                setup();
                break;
            case Functional:
                run();
                break;
            case Refill:
                refill();
                break;
        }
    }
}
