#include "utility.h"
#include "mbed.h"
#include "motor.h"
#include "TextLCD.h"
#include <string>

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
bool toggleDown(DigitalIn tgl)
{
    return tgl;
}

/**
 * @brief Return true if the switch is pressed down, false else
 *
 * @param sw Switch
 */
bool switchDown(DigitalIn sw)
{
    return !sw;
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

/**
 * @brief Activates the buzzer for the specified interval
 *
 */
void buzzInterval()
{
    wait(0.5);
    activateBuzzer(true);
    wait(0.5);
    activateBuzzer(false);
}

string getErrorsforLCD(float temp, float sal, int waterLevel)
{
    string err = "";
    if (waterLevel < MIN_WATER) {
        err += "Water level low ";
    } else if (waterLevel > MAX_WATER) {
        err += "Water level high";
    }

    if (sal < MIN_SAL) {
        err += "Low salinity    ";

    } else if (sal > MAX_SAL) {
        err += "High salinity   ";
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

void displayOnLCD(const char* format, ...)
{
    lcd.cls();

    va_list args;
    va_start(args, format);
    lcd.printf(format, args);
    va_end(args);
}

void checkTemperature(float temperature)
{
    heater = temperature <= DESIRED_TEMP;
    redLED = heater;
}

void checkRanges(float temperature, float salinity)
{
    greenLED = temperature > MIN_TEMP && temperature < MAX_TEMP &&
               salinity >= MIN_SAL && salinity <= MAX_SAL &&
               waterLevel >= MIN_WATER && waterLevel <= MAX_WATER;
}

// Basic functions that are activated in both Functional mode and Refill mode
void baseFunctions()
{
    float temp, sal;
    temp = get_temperature(temperatureSensor.read() * 3.3);
    sal = get_salinity(salinitySensor.read() * 3.3 * (5.f / 3.f));
    checkTemperature(temp);
    checkRanges(temp, sal);
}