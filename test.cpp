#include "UnitTest++/UnitTest++.h"
#include "modules.h"
#include "utility.h"
#include "mbed.h"
#include "motor.h"
#include "TextLCD.h"

#include <iostream>
#include <thread>
#include <cstring>

using namespace std;

bool stopWaiting;
vector<string> testOutput;
Mode currentMode = Setup;

SUITE(RANDOM)
{
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

        // 0 0 0
        checkRanges(37.0, 8.5, 450);
        CHECK_EQUAL(1, greenLED);
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
}

/**
 * Control Flow Testing
 */

SUITE(BASE_FUNCTIONS)
{
    TEST(BF_01)
    {
        temperatureSensor = 0.48023; // 35.1
        salinitySensor = 0.16702; // 7.0
        waterLevel = 500;
        baseFunctions();

        CHECK_EQUAL(1, heater);
        CHECK_EQUAL(1, redLED);
        CHECK_EQUAL(1, greenLED);
    }

    TEST(BF_02)
    {
        temperatureSensor = 0.41332; // 41.9
        salinitySensor = 0.23294; // 10.0
        waterLevel = 400;
        baseFunctions();

        CHECK_EQUAL(0, heater);
        CHECK_EQUAL(0, redLED);
        CHECK_EQUAL(1, greenLED);
    }

    TEST(BF_03)
    {
        temperatureSensor = 0.48121; // 35.0
        salinitySensor = 0.23499; // 10.1
        waterLevel = 501;
        baseFunctions();

        CHECK_EQUAL(1, heater);
        CHECK_EQUAL(1, redLED);
        CHECK_EQUAL(0, greenLED);
    }

    TEST(BF_04)
    {
        temperatureSensor = 0.41234; // 42.0
        salinitySensor = 0.16465; // 6.9
        waterLevel = 600;
        baseFunctions();

        CHECK_EQUAL(0, heater);
        CHECK_EQUAL(0, redLED);
        CHECK_EQUAL(0, greenLED);
    }
}

SUITE(SETUP_MODE)
{
    TEST(ST_01)
    {
        testOutput.clear();
        mtrA.currentState = Motor::idle;
        mtrB.currentState = Motor::idle;
        currentMode = Mode::Setup;
        switch3 = 0; // confirm switch pressed down
        setup();

        int numOutputs = 19;
        const string output[19] = {
            "Setup Mode",
            "Turn valves to\nposition R",
            "Buzzer activated", "Buzzer activated",
            "Empty Injector Awith Switch 8",
            "Empty Injector Bwith Switch 8",
            "Motor A activated", "topYellowLED=1",
            "Motor B activated", "botYellowLED=1",
            "Refilling...",
            "Turn valves to\nposition T",
            "Buzzer activated", "Buzzer activated",
            "Setup Complete"
        };

        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(35, injectorAlevel);
        CHECK_EQUAL(35, injectorBlevel);
        CHECK_EQUAL(Mode::Functional, currentMode);
    }

    TEST(ST_02)
    {
        // Skipped
    }

    TEST(ST_03)
    {
        // Hard skip
    }
}


SUITE(REFILL_MODE)
{
    TEST(RF_01)
    {
        testOutput.clear();
        injectorAlevel = 0;
        injectorBlevel = 0;
        refill();

        CHECK_EQUAL(35, injectorAlevel);
        CHECK_EQUAL(35, injectorBlevel);
        int numOutputs = 11;
        const string output[11] = {
            "Motor A activated",
            "topYellowLED=1",
            "Motor B activated",
            "botYellowLED=1",
            "Turn valves to\nposition R",
            "Buzzer activated",
            "Buzzer activated",
            "Refilling...",
            "Turn valves to\nposition T",
            "Buzzer activated",
            "Buzzer activated",
        };

        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(Mode::Functional, currentMode);
    }

    TEST(RF_02)
    {
        testOutput.clear();
        injectorAlevel = 0;
        injectorBlevel = 1;
        refill();

        CHECK_EQUAL(35, injectorAlevel);
        CHECK_EQUAL(1, injectorBlevel);
        int numOutputs = 9;
        const string output[9] = {
            "Motor A activated",
            "topYellowLED=1",
            "Turn valve A to\nposition R",
            "Buzzer activated", "Buzzer activated",
            "Refilling...",
            "Turn valve A to\nposition T",
            "Buzzer activated", "Buzzer activated"
        };

        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(Mode::Functional, currentMode);
    }

    TEST(RF_03)
    {
        testOutput.clear();
        injectorAlevel = 1;
        injectorBlevel = 0;
        refill();

        CHECK_EQUAL(1, injectorAlevel);
        CHECK_EQUAL(35, injectorBlevel);
        int numOutputs = 9;
        const string output[9] = {
            "Motor B activated",
            "botYellowLED=1",
            "Turn valve B to\nposition R",
            "Buzzer activated", "Buzzer activated",
            "Refilling...",
            "Turn valve B to\nposition T",
            "Buzzer activated", "Buzzer activated",
        };
        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(Mode::Functional, currentMode);
    }
}


SUITE(FUNCTIONAL_MODE)
{
    TEST(FM_01)
    {
        testOutput.clear();
        mtrA.currentState = Motor::idle;
        mtrB.currentState = Motor::idle;
        currentMode = Mode::Functional;

        switch3 = 0; // confirm switch pressed down
        waterLevel = 501;
        temperatureSensor = 0.48122; // 35 deg
        salinitySensor = 0.16465; // 6.9 PPT
        injectorAlevel = 1;
        injectorBlevel = 1;
        numInjections = 4;
        timerInjectFreq = 20;
        run();

        int numOutputs = 4;
        const string output[4] = {
            "Water level highEmpty tank",
            "Motor B activated",
            "botYellowLED=1"
        };
        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(1, heater);
        CHECK_EQUAL(1, redLED);
        CHECK_EQUAL(0, greenLED);
        CHECK_EQUAL(5, numInjections);
        CHECK_EQUAL(0, mtrA.led); // topYellowLED
        CHECK(timerInjectFreq.read() > 20);

        CHECK_EQUAL(Mode::Refill, currentMode);
    }

    TEST(FM_02)
    {
        testOutput.clear();
        mtrA.currentState = Motor::idle;
        mtrB.currentState = Motor::idle;
        currentMode = Mode::Functional;

        waterLevel = 499;
        temperatureSensor = 0.41234; // 42.5 deg
        salinitySensor = 0.23499; // 15.3 PPT
        injectorAlevel = 5;
        injectorBlevel = 5;
        numInjections = 0;
        timerInjectFreq = 0;
        run();

        int numOutputs = 4;
        const string output[4] = {
            "High salinity   ",
            "Motor A activated",
            "topYellowLED=1",
            "Buzzer activated",
        };

        // printf(testOutput);
        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(0, heater);
        CHECK_EQUAL(0, redLED);
        CHECK_EQUAL(0, greenLED);
        CHECK(timerInjectFreq.read() < 30);
        CHECK_EQUAL(1, numInjections);
        CHECK_EQUAL(0, mtrB.led); // botYellowLED
        CHECK_EQUAL(Mode::Functional, currentMode);

    }

    TEST(FM_03)
    {
        testOutput.clear();
        mtrA.currentState = Motor::idle;
        mtrB.currentState = Motor::idle;
        currentMode = Mode::Functional;

        waterLevel = 499;
        temperatureSensor = 0.46252; // 39.6 deg
        salinitySensor = 0.20117; // 8.5 PPT

        injectorAlevel = 5;
        injectorBlevel = 5;
        numInjections = 0;
        timerInjectFreq = 0;
        run();

        int numOutputs = 1;
        const string output[1] = {
            "Temp: 36.9 deg\nSal: 8.5 ppt"
        };

        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(1, heater);
        CHECK_EQUAL(1, redLED);
        CHECK_EQUAL(1, greenLED);
        CHECK(timerInjectFreq.read() < 30);
        CHECK_EQUAL(0, numInjections);
        CHECK_EQUAL(0, mtrA.led); // topYellowLED
        CHECK_EQUAL(0, mtrB.led); // botYellowLED
        CHECK_EQUAL(Mode::Functional, currentMode);
    }

    TEST(FM_04)
    {
        testOutput.clear();
        mtrA.currentState = Motor::idle;
        mtrB.currentState = Motor::idle;
        currentMode = Mode::Functional;

        waterLevel = 499;
        temperatureSensor = 0.41332; // 41.9 deg
        salinitySensor = 0.20117; // 8.5 PPT
        injectorAlevel = 5;
        injectorBlevel = 5;
        numInjections = 5;
        timerInjectFreq = 30;
        run();

        int numOutputs = 1;
        const string output[1] = {
            "Temp: 41.9 deg\nSal: 8.5 ppt"
        };

        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(0, heater);
        CHECK_EQUAL(0, redLED);
        CHECK_EQUAL(1, greenLED);
        CHECK(timerInjectFreq.read() < 30);
        CHECK_EQUAL(0, numInjections);
        CHECK_EQUAL(0, mtrA.led); // topYellowLED
        CHECK_EQUAL(0, mtrB.led); // botYellowLED
        CHECK_EQUAL(Mode::Functional, currentMode);
    }

    TEST(FM_05)
    {
        testOutput.clear();
        mtrA.currentState = Motor::idle;
        mtrB.currentState = Motor::idle;
        currentMode = Mode::Functional;

        waterLevel = 400;
        temperatureSensor = 0.6288; // 41.9 deg
        salinitySensor = 0.; // 8.5 PPT
        injectorAlevel = 35;
        injectorBlevel = 35;
        numInjections = 5;
        timerInjectFreq = 15;
        run();

        int numOutputs = 2;
        const string output[2] = {
            "Low salinity    Temperature low",
            "Buzzer activated"
        };

        CHECK_EQUAL(numOutputs, testOutput.size());
        CHECK_ARRAY_EQUAL(output, testOutput, numOutputs);
        CHECK_EQUAL(1, heater);
        CHECK_EQUAL(1, redLED);
        CHECK_EQUAL(0, greenLED);
        CHECK(timerInjectFreq.read() > 15);
        CHECK(timerInjectFreq.read() < 30);
        CHECK_EQUAL(0, mtrA.led); // topYellowLED
        CHECK_EQUAL(0, mtrB.led); // botYellowLED
        CHECK_EQUAL(5, numInjections);
        CHECK_EQUAL(Mode::Functional, currentMode);

    }
}

int main(int argc, char** argv)
{
    stopWaiting = false;
    toggle1 = 1; // do not mute buzzer
    UnitTest::RunAllTests();
}
