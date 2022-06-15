#ifndef MBED_Motor_H
#define MBED_Motor_H
#endif

#include "mbed.h"

#define MICROSTEPS_PER_STEP 16
#define MAX_SPEED 8000
#define MIN_SPEED 500

class Motor
{
public:
    enum State { accelerate, constant, deaccelerate, idle };
    State currentState;

    Motor(PinName _en, PinName m0, PinName m1, PinName m2, PinName _stepPin, PinName dir);

    void step(float microstep, int dir, float speed);
    void step(float microstep, int dir, float speed, int steps);

    void enable();
    void disable();

    PwmOut stepPin;
    DigitalOut direction;

private:

    float minSpeed, maxSpeed, currentSpeed, targetSpeed;
    int steps, stepsToStop, accRate;

    DigitalOut en;
    BusOut microstepping;
    Timeout timer;

    void step(float microstep, int dir);
    void move();
    void stop();
    void setResolution(float microstep);
    void setDirection(int dir);

    //DigitalOut stepPin;
    //DigitalOut direction;
};
