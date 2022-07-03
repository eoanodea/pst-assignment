#ifndef MBED_H
#define MBED_H

#include <string>
#include <ctime>
#include <chrono>
#include <thread>
#include <stdarg.h>
#include <stdio.h>
#include <random>

using namespace std;

typedef string PinName;

inline void wait(float x)
{
    int s = x * 1e6;
    this_thread::sleep_for(std::chrono::microseconds(s));
}

/**
 * @brief Mockup for AnalogIn
 */
class AnalogIn
{
public:
    AnalogIn(string PinName)
    {
        pin = PinName;
    }

    float read()
    {
        return _value;
    }

    AnalogIn& operator=(const float val)
    {
        _value = val;
        return *this;
    }

    AnalogIn(const AnalogIn&)
    {
    }
private:
    float _value;
    string pin;
};


/**
 * @brief Mockup for DigitalIn
 */
struct DigitalIn final
{
    DigitalIn() = default;

    DigitalIn(string val)
        : _value{ false }
    {
        pin = val;
    }

    operator bool() const
    {
        return _value;
    }

    DigitalIn& operator=(const bool val)
    {
        _value = val;
        return *this;
    }

    bool operator==(const bool other)
    {
        return _value == other;
    }

private:
    string pin;
    bool _value{ false };
};


/**
 * @brief Mockup for DigitalOut
 */
struct DigitalOut final
{
    DigitalOut() = default;

    DigitalOut(string val)
        : _value{ false }
    {
        pin = val;
    }

    operator bool() const
    {
        return _value;
    }

    DigitalOut& operator=(const bool val)
    {
        _value = val;
        return *this;
    }

    bool operator==(const bool other)
    {
        return _value == other;
    }

private:
    string pin;
    bool _value{ false };
};


/**
 * @brief Mockup for BusOut
 */
struct BusOut final
{
    BusOut() = default;

    BusOut(string val1, string val2, string val3)
        : _value{ false }
    {
    }

    explicit operator bool() const
    {
        return _value;
    }

    BusOut& operator=(const bool val)
    {
        _value = val;
        return *this;
    }

private:
    bool _value{ false };
};


/**
 * @brief Mockup for Timer
 *
 */
class Timer
{
public:
    clock_t start_time;

    void start()
    {
        start_time = clock();
    }

    void reset()
    {
        this->start();
    }

    double read()
    {
        return (std::clock() - start_time) / (double)CLOCKS_PER_SEC;
    }

    Timer& operator=(float sec)
    {
        reset();
        start_time -= sec * (double)CLOCKS_PER_SEC;
        return *this;
    }
};

#endif
