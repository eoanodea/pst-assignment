#ifndef UTILITY_H
#define UTILITY_H

#include "mbed.h"
#include "motor.h"
#include <string>
#include "TextLCD.h"

extern float get_temperature(float Vin);
extern float get_salinity(float Vin);

extern bool toggleDown(DigitalIn tgl);
extern bool switchDown(DigitalIn sw);
extern void waitForConfirmation();

extern void activateBuzzer(bool on);
extern void buzzInterval();

extern string getErrorsforLCD(float temp, float sal, int waterLevel);
extern void displayOnLCD(const char *format, ...);

extern void checkTemperature(float temperature);
extern void checkRanges(float temperature, float salinity, int waterLevel);
extern void baseFunctions();

#endif
