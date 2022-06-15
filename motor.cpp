#include "motor.h"
#include "mbed.h"

Motor::Motor(PinName _en, PinName m0, PinName m1, PinName m2, PinName _stepPin, PinName dir):en(_en),
    microstepping(m0, m1, m2),
    stepPin(_stepPin),
    direction(dir)
{
}

void Motor::setResolution(float microstep)
{
    //Microstepping step
    if (microstep == 1) microstepping = 0;
    else if (microstep == 1/2) microstepping = 1;
    else if (microstep == 1/4) microstepping = 2;
    else if (microstep == 1/8) microstepping = 3;
    else if (microstep == 1/16) microstepping = 4;
    else if (microstep == 1/32) microstepping = 5;
}

void Motor::setDirection(int dir)
{
    if (dir == 1) {
        direction = 0;
    } else if (dir == 0) {
        direction = 1;
    }
}

void Motor::step(float microstep, int dir)
{
    setResolution(microstep);
    setDirection(dir);
    this->enable();
}

// Activate motor with the switch (unknown number of steps)
void Motor::step(float microstep, int dir, float speed)
{
    this.step(microstep, dir);

    //  Speed or times per second
    currentState = accelerate;
    currentSpeed = MIN_SPEED;
    targetSpeed = speed;
    this->steps = steps;
    this->stepsToStop = -1;
    this->accRate = 5;
    move();
}

// Activate motor for a given number of steps with 20% acceleration and 20% deacceleration
void Motor::step(float microstep, int dir, float speed, int steps)
{
    this.step(microstep, dir);

    //  Speed or times per second
    currentState = accelerate;
    currentSpeed = MIN_SPEED;
    targetSpeed = speed;
    this->steps = steps;
    this->stepsToStop = (steps * 0.2);
    this->accRate = (targetSpeed - speed) / stepsToStop;
    move();
}

void Motor::move()
{
    float period = 2 / currentSpeed;
    steps -= 1;

    if (currentState == Motor::accelerate) {
        stepPin.period(period);
        stepPin.write(0.5f);
        currentSpeed += accRate;
        if (currentSpeed == targetSpeed)
            currentState = Motor::constant;

        timer.attach(this, &Motor::move, period);
    } else if (currentState == Motor::constant) {
        if (steps == stepsToStop)
            currentState = Motor::deaccelerate;

        timer.attach(this, &Motor::move, period);
    } else if (currentSpeed == Motor::deaccelerate) {
        if (currentSpeed == MIN_SPEED) {
            currentSpeed = Motor::idle;
            timer.attach(this, &Motor::stop, 0);
        } else {
            currentSpeed -= accRate;
            timer.attach(this, &Motor::move, period);
        }
    }
}

void Motor::stop()
{
    stepPin.write(0);
    this->disable();
}

void Motor::enable()
{
    en = 0;
}

void Motor::disable()
{
    en = 1;
}
