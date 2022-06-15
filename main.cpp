#include "mbed.h"
#include "TextLCD.h"
#include "motor.h"
#include "LCDErrorMsg.h"
// #include <ctime>
#include <math.h>
// #include <time.h>

// Injection direction for Motor A
#define INJECTA 1

// Injection direction for Motor B
#define INJECTB 0

// 1 if manual motor control is active, 0 when deactivated.
bool manualMotor = true;

Timer t;

// Water level for beaker
int waterLevel = 251;

LCDErrorMsg tempMsg("Temp");
LCDErrorMsg waterMsg("water");
LCDErrorMsg salinityMsg("Sal");

// RS, ENABLE, A4, A5, A6, A7
TextLCD lcd(p29, p30, p28, p27, p26, p25);

// ENABLE, M0, M1, M2, STP, DIR
Motor mtrB(p7, p11, p12, p13, p22, p8);

// ENABLE, M0, M1, M2, STP, DIR
Motor mtrA(p10, p11, p12, p13, p21, p9);

// BusOut microstepping(p11, p12, p13);
// DigitalOut stepA(p21);
// DigitalOut dirA(p9);
// DigitalOut stepB(p22);
// DigitalOut dirB(p8);

DigitalOut heater(p14);
DigitalOut greenLED(p24);
DigitalOut redLED(p23);
DigitalOut yellowUpLED(p22);
DigitalOut yellowDownLED(p21);
DigitalOut buzzer(p5);

// when switched down, the corresponding pin SW1/SW2 will output 3.3V;
// when switched up, the pin is connected to the ground (0V).
AnalogIn switch1(p17);
AnalogIn switch2(p18);

// their corresponding pins normally output 3.3V, but when pushed down they switch to 0V.
AnalogIn switch3(p19);
AnalogIn switch4(p20);

// configures pin20 for analog input. Creates object temperatureSensor.
AnalogIn temperatureSensor(p15);

// configures pin20 for analog input. Creates object salinitySensor.
AnalogIn salinitySensor(p16);

float get_temperature(float Vin) {
    // return 38.00;
    return -30.79759812*Vin + 83.90646865;
}

float get_salinity(float Vin) {
    // return 8.00;
    return 13.40769944 *  exp(0.39797502 * Vin) - 12.3251654;
}

// Determines whether to display the flashing error messages on the LCD screen
bool shouldDisplayError() {
    if (switch2.read() != 0) return false;
    return waterMsg.display || salinityMsg.display || tempMsg.display;
}

// Checks water, temp and salinity and toggles their error messages
void checkErrorsforLCD(float Vtemp, float Vsal) {
    if (waterLevel < 251) { // Beaker holds more than 250 mL of water
        // @TODO activate buzzer
        waterMsg.toggle(true, false);
    } else if (waterLevel > 450) {
        // @TODO activate buzzer
        waterMsg.toggle(true, true);
    } else {
        // @TODO disable buzzer
        waterMsg.toggle(false, false);
    }

    if (get_temperature(Vtemp) < 35) {
        // @TODO activate buzzer
        tempMsg.toggle(true, false);
    } else if(get_temperature(Vtemp) > 42) {
        // @TODO activate buzzer
        tempMsg.toggle(true, true);
    } else {
        // @TODO disable buzzer
        tempMsg.toggle(false, false);
    }

    if (get_salinity(Vsal) < 7) {
        // @TODO activate buzzer
        salinityMsg.toggle(true, false);
    } else if (get_salinity(Vsal) > 14) {
        // @TODO activate buzzer
        salinityMsg.toggle(true, true);
    } else {
        // @TODO disable buzzer
        salinityMsg.toggle(false, false);
    }
}


void manualMotorControl()
{
    int dirA, dirB;

    // If switch up
    if (switch2.read() == 0) {
        dirA = INJECTA;
        dirB = INJECTB;
    } else {
        dirA = !INJECTA;
        dirB = !INJECTB;
    }

    // If switch pushed down.
    if (switch3.read() == 0 && mtrA.currentState == Motor::idle) {
        // motorA = 1;
        mtrA.step(1/MICROSTEPS_PER_STEP, dirA, MAX_SPEED);
    } else if (switch3.read() == 1 && mtrA.currentState != Motor::idle) {
        mtrA.currentState = Motor::deaccelerate;
    }

    mtrA.move();
    // if (mtrB.currentState != Motor::idle) {
    //     mtrB.move();
    // }
}

// void displayDefaultLCD(float Vtemp, float Vsal) {
//     if (switch1.read() == 0) {
//         lcd.printf("Temp: %.2f deg\n", get_temperature(Vtemp));
//         lcd.printf("Sal: %.2f ppt", get_salinity(Vsal));
//     } else {
//         lcd.printf("Temp: %.2f V\n", Vtemp);
//         lcd.printf("Sal: %.2f V", Vsal);
//     }
// }

int main() {
    float f, Vtemp, Vsal;
    bool motorA, motorB;

    t.start();
    int timerInterval = 3;	// seconds

    while(1) {
        f = temperatureSensor.read(); // or just float f = temperatureSensor; reads the digital output
        Vtemp = f * 3.3; // converts the digital input value to volts [V]

        f = salinitySensor.read();
        Vsal = f * 3.3 * (5.f/3.f);

        mtrA.printState();

        lcd.cls();


        if (!shouldDisplayError() || t.read() <= timerInterval) {
            if (switch1.read() == 0) {
                lcd.printf("Temp: %.2f deg\n", get_temperature(Vtemp));
                lcd.printf("Sal: %.2f ppt", get_salinity(Vsal));
            } else {
                lcd.printf("Temp: %.2f V\n", Vtemp);
                lcd.printf("Sal: %.2f V", Vsal);
            }

            // lcd.printf("Temp: %.2f deg\n", get_temperature(Vtemp));
            // lcd.printf("Sal: %.2f ppt", get_salinity(Vsal));
            // displayDefaultLCD(Vtemp, Vsal);
        } else {

            if(waterMsg.display) {
                lcd.printf("%s \n", waterMsg.getMsg());
            }

            if(tempMsg.display) {
                lcd.printf("%s \n", tempMsg.getMsg());
            }

            if(salinityMsg.display) {
                lcd.printf("%s", salinityMsg.getMsg());
            }
            // lcd.printf("Error message \n");
            // lcd.printf(errorMessages[1]);

            if (t.read() >= (timerInterval * 2)) {
                t.reset();
            }

        }

        checkErrorsforLCD(Vtemp, Vsal);

        if (get_temperature(Vtemp) < 38){
            heater = 1;
        } else {
            heater = 0;
        }

        // Controlling Green LED (Fuck requirements team)
        if (
            get_temperature(Vtemp) > 36 && get_temperature(Vtemp) < 40 && // Temp between 37 & 39 deg
            get_salinity(Vsal) > 8 && get_salinity(Vsal) < 13 && // Salinity between 9 & 12 ppt
            waterLevel > 249 && waterLevel < 451 // Water level between 250 & 450 mL
        ) {
            greenLED = 1;
        } else {
            greenLED = 0;
        }

        if (manualMotor) {
            manualMotorControl();
        }

        // if (motorB && mtrB.currentState == Motor::idle) {
        //     mtrB.step(1/MICROSTEPS_PER_STEP, dirB, MAX_SPEED, -1);
        // }

        redLED = heater;
        // greenLED = 0;

        // print the percentage and 16 bit normalized values
        if (mtrA.currentState == Motor::idle && mtrB.currentState == Motor::idle) {
            wait(1);
        } else {
            for (int i = 0; i < MIN_SPEED; i++) {
                mtrA.move();
                mtrB.move();
                yellowUpLED = mtrA.currentState != Motor::idle;
                yellowDownLED = mtrB.currentState != Motor::idle;
            }
        }
    }
}
