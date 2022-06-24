#include "UnitTest++/UnitTest++.h"
#include "modules.h"
#include "utility.h"
#include "mbed.h"
#include "motor.h"

Mode currentMode = Setup;

TEST(GET_TEMP)
{
    float errorMargin = 0.5;
    CHECK(get_temperature(1.58) > 35-errorMargin && get_temperature(1.58) < 35+errorMargin);
}

TEST(GET_SAL)
{
    float errorMargin = 1.;
    CHECK(get_salinity(1.04) > 8.2-errorMargin && get_salinity(1.04) < 8.2+errorMargin);
}

TEST(checkTemperature_above)
{
    checkTemperature(37.1);
    CHECK_EQUAL(0, heater);
    CHECK_EQUAL(0, redLED);
}

TEST(checkTemperature_equal)
{
    checkTemperature(37);
    CHECK_EQUAL(1, heater);
    CHECK_EQUAL(1, redLED);
}

TEST(checkRanges)
{
    // Temperature, salinity, water level: in range (1), not in range (0)

    // 1 1 1
    checkRanges(37, 8.5, 450);
    CHECK_EQUAL(1, greenLED);

    // 1 1 0
//     checkRanges(37, )
//     CHECK_EQUAL(0, greenLED);

//     // 1 0 1
//     checkRanges(37, )
//     CHECK_EQUAL(0, greenLED);

//     // 1 0 1
//     checkRanges(37, )
//     CHECK_EQUAL(0, greenLED);

//     // 0 1 1
//     checkRanges(37, )
//     CHECK_EQUAL(0, greenLED);

//     // 0 1 0
//     CHECK_EQUAL(0, greenLED);

//     // 0 0 1
//     CHECK_EQUAL(0, greenLED);

//     // 0 0 0
//     CHECK_EQUAL(0, greenLED);
}

int main(int, const char *[])
{
   return UnitTest::RunAllTests();
}
