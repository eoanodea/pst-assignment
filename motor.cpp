#include "motor.h"
#include "mbed.h"

Motor::Motor(PinName _en, PinName m0, PinName m1, PinName m2, PinName _stepPin, PinName dir, PinName _led) :en(_en),
microstepping(m0, m1, m2),
stepPin(_stepPin),
direction(dir),
led(_led)
{
    disable();
}

void Motor::setResolution(float microstep)
{
    //Microstepping initializeMove
    if (microstep == 1) microstepping = 0;
    else if (microstep == 1 / 2) microstepping = 1;
    else if (microstep == 1 / 4) microstepping = 2;
    else if (microstep == 1 / 8) microstepping = 3;
    else if (microstep == 1 / 16) microstepping = 4;
    else if (microstep == 1 / 32) microstepping = 5;
}

void Motor::setDirection(int dir)
{
    if (dir == 1) {
        direction = 0;
    } else if (dir == 0) {
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
void Motor::initializeMove(float microstep, float speed)
{
    initializeMove(microstep);

    //  Speed or times per second
    this->steps = -1;
    this->stepsToStop = 0;
    this->accRate = 100;
}

// Activate motor for a given number of steps with 20% acceleration and 20% deacceleration
void Motor::initializeMove(float microstep, float speed, int steps)
{
    initializeMove(microstep);

    //  Speed or times per second
    this->steps = steps;
    this->stepsToStop = (steps * 0.2);
    this->accRate = (MAX_SPEED - currentSpeed) / stepsToStop;
}

void Motor::update(bool doWait)
{
    if (currentState != Motor::idle) {
        float period = 1 / currentSpeed;
        steps -= 1;

        // Speed or times per second
        if (doWait) {
            stepPin = !stepPin;
            wait(1/currentSpeed);
        }

        // printf("%f\r\n", currentSpeed);

        if (currentState == Motor::accelerate) {
            currentSpeed += accRate;
            if (currentSpeed >= MAX_SPEED) {
                currentState = Motor::constant;
            }
        } else if (currentState == Motor::constant) {
            if (steps == stepsToStop) {
                currentState = Motor::deaccelerate;
            }
        } else if (currentState == Motor::deaccelerate) {
            currentSpeed -= accRate;
            if (currentSpeed <= MIN_SPEED) {
                disable();
            }
        }
    }
}

void Motor::enable()
{
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
    switch (currentState) {
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
