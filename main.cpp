#include "mbed.h"
#include "TextLCD.h"
#include <math.h>
#include "DRV8825.h"

#define MAX_SPEED 8000
#define MICROSTEPS_PER_STEP 16

#define INJECTB 0
#define INJECTA 1

TextLCD lcd(p25, p26, p24, p23, p22, p21);

// ENABLE, M0, M1, M2, STP, DIR
DRV8825 mtrB(p5, p6, p7, p8, p9, p10);

// ENABLE, M0, M1, M2, STP, DIR
DRV8825 mtrA(p17, p16, p15, p14, p13, p12);

DigitalOut heater(p11);
DigitalOut greenLED(p30);
DigitalOut redLED(p29);
DigitalOut yellowUpLED(p28);
DigitalOut yellowDownLED(p27);

AnalogIn switch1(p20);
AnalogIn switch2(p18);
AnalogIn temperatureSensor(p18); // configures pin20 for analog input. Creates object temperatureSensor.
AnalogIn salinitySensor(p19); // configures pin20 for analog input. Creates object temperatureSensor.

float get_temperature(float Vin) {
    return -30.79759812*Vin + 83.90646865;
    // float R = (-Vin * 15000) / (Vin - 3.3);
    // return 1 / (2.46397852 + -3.92288745e-01 * log(R) + 1.50407123e-03 * pow(log(R), 3));
}

float get_salinity(float Vin) {
    return Vin;
}

int main() {
    float f, Vintt, Vints;

    for (int i = 500; i < MAX_SPEED; i += 5) {
//        mtrA.settings(1 / MICROSTEPS_PER_STEP, INJECTA, i);
        mtrB.settings(1 / MICROSTEPS_PER_STEP, INJECTB, i);
   }

//    int j=;
    for (int i = 0; i < 165266; i++) {
//        mtrA.settings(1 / MICROSTEPS_PER_STEP, INJECTA, MAX_SPEED);
        mtrB.settings(1 / MICROSTEPS_PER_STEP, INJECTB, MAX_SPEED);
    }

    for (int i = MAX_SPEED; i > 500; i -= 5) {
//        mtrA.settings(1 / MICROSTEPS_PER_STEP, INJECTA, i);
        mtrB.settings(1 / MICROSTEPS_PER_STEP, INJECTB, i);
    }

//    wait(5);

    // while(1) {
    //     f = temperatureSensor.read(); // or just float f = temperatureSensor; reads the digital output
    //     Vintt = f * 3.3; // converts the digital input value to volts [V]

    //     f = salinitySensor.read();
    //     Vints = f * 3.3 * (5.f/3.f);

    //     printf("Temp V value is %f\r\n", Vintt);
    //     printf("Sal  V value is %f\r\n", Vints);
    //     printf("----------\r\n");

    //     if (get_temperature(Vintt) < 38){
    //         heater = 1;
    //     } else {
    //         heater = 0;
    //     }

    //     lcd.cls();
    //     if (switch1.read() * 3.3 > 3) {
    //         lcd.printf("Temp: %.2f V\n", Vintt);
    //         lcd.printf("Sal: %.2f V", Vints);
    //     } else {
    //         lcd.printf("Temp: %.2f deg\n", get_temperature(Vintt));
    //         lcd.printf("Sal: %.2f ppt", get_salinity(Vints));
    //     }

    //     redLED = heater;
    //     greenLED = 0;
    //     yellowUpLED = 0;
    //     yellowDownLED = 0;

    //     // print the percentage and 16 bit normalized values
    //     wait(0.1f);
    // }
}
