#ifndef MBED_Motor_H
#define MBED_Motor_H

#include "mbed.h"

#define MICROSTEPS_PER_STEP 16
#define MAX_SPEED 3000
#define MIN_SPEED 10

class Motor
{
public:
    enum State
    {
        accelerate,
        constant,
        deaccelerate,
        idle
    };
    State currentState;
    float currentSpeed;
    int accRate;

    Motor(PinName _en, PinName m0, PinName m1, PinName m2, PinName _stepPin, PinName dir, PinName _led);

    void initializeMove(float microstep, int speed);
    void initializeMove(float microstep, int speed, int steps);
    void setDirection(int dir);
    void update(bool wait);

    void enable();
    void disable();
    void printState();

    bool operator==(Motor* other);

    DigitalOut led;
    int steps;

    DigitalOut led;
    int steps;

private:
    DigitalOut direction, stepPin;
    int stepsToStop, speed;

    DigitalOut en;
    BusOut microstepping;

    void initializeMove(float microstep);
    void setResolution(float microstep);
};

#endif /* MBED_Motor_H */
