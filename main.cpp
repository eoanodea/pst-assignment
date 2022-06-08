#include "mbed.h"
#include "TextLCD.h"

DigitalOut myled(LED1);
DigitalOut heater(p21);
TextLCD lcd(p25, p26, p24, p23, p22, p21);
DigitalOut greenLED(p25);
DigitalOut redLED(p24);
DigitalOut yellowUpLED(p23);
DigitalOut yellowDownLED(p22);

AnalogIn temperatureSensor(p15); // configures pin20 for analog input. Creates object temperatureSensor.
AnalogIn salinitySensor(p16); // configures pin20 for analog input. Creates object temperatureSensor.

int main() {
    float f, Vintf, Vints;
    while(1) {
        // myled = 1;
//         wait(0.05);
        // myled = 0;
        // wait(0.2);
        heater = 1;

        greenLED = 1;
        redLED = 1;
        yellowUpLED = 1;
        yellowDownLED = 1;

        f = temperatureSensor.read(); // or just float f = temperatureSensor; reads the digital output
        Vintf = f * 3.3; // converts the digital input value to volts [V]

        f = salinitySensor.read();
        Vints = f * 3.3 * 5.f/3.f;

        // printf("F value is %5.2f\r\n", f, Vin);
        printf("Temp V value is %5.2f\r\n", Vintf);
        printf("Sal  V value is %5.2f\r\n", Vints);
        printf("----------\r\n");

        lcd.cls();
        lcd.printf("Hello world\n");

        // print the percentage and 16 bit normalized values
        wait(0.1f);
    }
}
