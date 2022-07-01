#include "motor.h"
#include "mbed.h"
#include "modules.h"

Motor::Motor(PinName _en, PinName m0, PinName m1, PinName m2, PinName _stepPin, PinName dir, PinName _led) : en(_en),
                                                                                                             microstepping(m0, m1, m2),
                                                                                                             stepPin(_stepPin),
                                                                                                             direction(dir),
                                                                                                             led(_led)
{
    disable();
}

void Motor::setResolution(float microstep)
{
    // Microstepping initializeMove
    if (microstep == 1)
        microstepping = 0;
    else if (microstep == 1 / 2)
        microstepping = 1;
    else if (microstep == 1 / 4)
        microstepping = 2;
    else if (microstep == 1 / 8)
        microstepping = 3;
    else if (microstep == 1 / 16)
        microstepping = 4;
    else if (microstep == 1 / 32)
        microstepping = 5;
}

void Motor::setDirection(int dir)
{
    if (dir == 1)
    {
        direction = 0;
    }
    else if (dir == 0)
    {
        direction = 1;
    }
}

void Motor::initializeMove(float microstep)
{
    setResolution(microstep);
    currentState = accelerate;
    currentSpeed = MIN_SPEED;
    enable();
}

// Activate motor with the switch (unknown number of steps)
void Motor::initializeMove(float microstep, int speed)
{
    initializeMove(microstep);

    //  Speed or times per second
    this->speed = speed;
    this->steps = -1;
    this->stepsToStop = 0;
    this->accRate = 100;
}

// Activate motor for a given number of steps with 20% acceleration and 20% deacceleration
void Motor::initializeMove(float microstep, int speed, int steps)
{
    initializeMove(microstep);

    //  Speed or times per second
    this->speed = speed;
    this->steps = steps;
    this->stepsToStop = ceil(steps * 0.2);
    this->accRate = ceil((float)(speed - currentSpeed) / stepsToStop);
}

void Motor::update(bool doWait)
{
    if (currentState != Motor::idle)
    {
        steps -= 1;
        stepPin = !stepPin;

        // Speed or times per second
        if (doWait)
        {
            wait(1 / currentSpeed);
        }

        if (currentState == Motor::accelerate)
        {
            currentSpeed += accRate;
            if (currentSpeed >= speed)
            {
                currentState = Motor::constant;
            }
        }
        else if (currentState == Motor::constant)
        {
            if (steps == stepsToStop)
            {
                currentState = Motor::deaccelerate;
            }
        }
        else if (currentState == Motor::deaccelerate)
        {
            currentSpeed -= accRate;
            if (currentSpeed <= MIN_SPEED)
            {
                disable();
            }
        }
    }
}

void Motor::enable()
{

#ifdef UNIT_TESTING
    string msg = "Motor ";
    msg += this == &mtrA ? "A activated" : "B activated";
    testOutput.push_back(msg);

    msg = "";
    msg += this == &mtrA ? "topYellowLED=1" : "botYellowLED=1";
    testOutput.push_back(msg);
#endif
    en = 0;
    led = 1;
}

void Motor::disable()
{
    currentState = Motor::idle;
    en = 1;
    led = 0;
}

void Motor::printState()
{
    switch (currentState)
    {
    case Motor::idle:
        printf("idle\r\n");
        break;
    case Motor::accelerate:
        printf("accelerate\r\n");
        break;
    case Motor::constant:
        printf("constant\r\n");
        break;
    case Motor::deaccelerate:
        printf("deaccelerate\r\n");
        break;
    default:
        break;
    }
}

bool Motor::operator==(Motor *other)
{
    return this == other;
}
