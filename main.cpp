#include "mbed.h"
#include "TextLCD.h"
#include "motor.h"
#include <ctime>
#include <math.h>

// Injection direction for Motor A
#define INJECTA 1

// Injection direction for Motor B
#define INJECTB 0

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
    return -30.79759812*Vin + 83.90646865;
}

float get_salinity(float Vin) {
    return Vin;
}

int main() {
    float f, Vtemp, Vsal;
    int motorA, motorB, dirA, dirB;

    while(1) {
        f = temperatureSensor.read(); // or just float f = temperatureSensor; reads the digital output
        Vtemp = f * 3.3; // converts the digital input value to volts [V]

        f = salinitySensor.read();
        Vsal = f * 3.3 * (5.f/3.f);

        lcd.cls();
        if (switch1.read() == 0) {
            lcd.printf("Temp: %.2f deg\n", get_temperature(Vtemp));
            lcd.printf("Sal: %.2f ppt", get_salinity(Vsal));
        } else {
            lcd.printf("Temp: %.2f V\n", Vtemp);
            lcd.printf("Sal: %.2f V", Vsal);
        }

        if (get_temperature(Vtemp) < 38){
            heater = 1;
        } else {
            heater = 0;
        }

        // If switch up
        if (switch2.read() == 0) {
            dirA = INJECTA;
            dirB = INJECTB;
        } else {
            dirA = !INJECTA;
            dirB = !INJECTB;
        }

        // If switch pushed down.
        if (switch3.read() == 0) {
            // motorA = 1;
            mtrA.step(1/MICROSTEPS_PER_STEP, dirA, MAX_SPEED);
        } else if (mtrA.currentState != Motor::idle) {
            mtrA.currentState = Motor::deaccelerate;
            // motorA = 0;
        }

        // If switch pushed down.
        // if (switch4.read() == 0) {
        //     motorB = 1;
        // } else {
        //     motorA = 0;
        // }

        // if (motorA && mtrA.currentState == Motor::idle) {
        // }
        // if (motorB && mtrB.currentState == Motor::idle) {
        //     mtrB.step(1/MICROSTEPS_PER_STEP, dirB, MAX_SPEED, -1);
        // }

        redLED = heater;
        greenLED = 0;
        yellowUpLED = mtrA.currentState != Motor::idle;
        yellowDownLED = mtrB.currentState != Motor::idle;

        // print the percentage and 16 bit normalized values
        wait(0.1f);
    }
}
