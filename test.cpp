#include "UnitTest++/UnitTest++.h"
#include "modules.h"
#include "utility.h"
#include "mbed.h"
#include "motor.h"

Mode currentMode = Setup;

TEST(TEMP)
{
    float errorMargin = 1.;
    CHECK(get_temperature(2.7) > 0 - errorMargin && get_temperature(2.7) < 0 + errorMargin);
    CHECK(get_temperature(1.82) > 28 - errorMargin && get_temperature(1.82) < 28 + errorMargin);
    CHECK(get_temperature(1.47) > 38.5 - errorMargin && get_temperature(1.47) < 38.5 + errorMargin);
    CHECK(get_temperature(1.35) > 42.5 - errorMargin && get_temperature(1.35) < 42.5 + errorMargin);
}

TEST(SAL)
{
    float errorMargin = 1.;
    CHECK(get_salinity(0.56) > 4.4 - errorMargin && get_salinity(0.56) < 4.4 + errorMargin);
    CHECK(get_salinity(1.03) > 8.2 - errorMargin && get_salinity(1.03) < 8.2 + errorMargin);
    CHECK(get_salinity(1.8) > 15.3 - errorMargin && get_salinity(1.8) < 15.3 + errorMargin);
}

TEST(HEATER)
{
    checkTemperature(36.9);
    CHECK_EQUAL(1, heater);
    CHECK_EQUAL(1, redLED);

    checkTemperature(37.0);
    CHECK_EQUAL(0, heater);
    CHECK_EQUAL(0, redLED);

    checkTemperature(37.1);
    CHECK_EQUAL(0, heater);
    CHECK_EQUAL(0, redLED);
}

TEST(RNG)
{
    // Temperature, salinity, water level: under range (-1), in range (0), above range (1)

    // 0 0 0
    checkRanges(37.0, 8.5, 450);
    CHECK_EQUAL(1, greenLED);

    // -1 -1 -1
    checkRanges(34.0, 4.0, 200);
    CHECK_EQUAL(0, greenLED);

    // -1 0 0
    checkRanges(34.0, 8.5, 450);
    CHECK_EQUAL(0, greenLED);

    // -1 1 1
    checkRanges(34.0, 14.0, 650);
    CHECK_EQUAL(0, greenLED);

    // 0 0 1
    checkRanges(37.0, 8.5, 650);
    CHECK_EQUAL(0, greenLED);

    // 0 1 -1
    checkRanges(37.0, 14.0, 250);
    CHECK_EQUAL(0, greenLED);

    // 0 -1 0
    checkRanges(37.0, 4.0, 450);
    CHECK_EQUAL(0, greenLED);

    // 1 1 0
    checkRanges(44.0, 14.0, 450);
    CHECK_EQUAL(0, greenLED);

    // 1 -1 1
    checkRanges(44.0, 4.0, 650);
    CHECK_EQUAL(0, greenLED);

    // 1 0 -1
    checkRanges(44.0, 8.5, 250);
    CHECK_EQUAL(0, greenLED);
}

TEST(LCD)
{
    string errors;
    // Temperature, salinity, water level: under range (-1), in range (0), above range (1)
    // float temp, float sal, int waterLevel
    // water level is always in range because it is not returned in the error string

    // 0,0,0
    errors = getErrorsforLCD(37, 8.5, 450);
    CHECK_EQUAL("", errors);

    // 0,-1,0
    errors = getErrorsforLCD(37, 4, 450);
    CHECK_EQUAL("Low salinity    ", errors);

    // 0, 1, 0
    errors = getErrorsforLCD(37, 14, 450);
    CHECK_EQUAL("High salinity   ", errors);

    // -1,-1,0
    errors = getErrorsforLCD(34, 4, 450);
    CHECK_EQUAL("Low salinity    Temperature low ", std::string(errors));

    //-1,1,0
    errors = getErrorsforLCD(34, 14, 450);
    CHECK_EQUAL("High salinity   Temperature low ", errors);

    //-1,0,0
    errors = getErrorsforLCD(34, 8.5, 450);
    CHECK_EQUAL("Temperature low ", errors);

    // 1,1,0
    errors = getErrorsforLCD(43, 14, 450);
    CHECK_EQUAL("High salinity   Temperature high", errors);

    // 1,0,0
    errors = getErrorsforLCD(43, 8.5, 450);
    CHECK_EQUAL("Temperature high", errors);

    // 1,-1,1
    errors = getErrorsforLCD(43, 4, 450);
    CHECK_EQUAL("Low salinity    Temperature high", errors);
}

int main(int, const char *[])
{
    return UnitTest::RunAllTests();
}
