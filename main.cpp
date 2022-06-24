#include "mbed.h"
#include "modules.h"
#include <string>
#include <math.h>

Mode currentMode = Setup;

int main(int argc, char const* argv[])
{
    printf("\r\n\r\n----------V1.8-----------\r\n\r\n");

    float timeElapsed = 0.;
    timerMotor.start();

    while (1) {
        switch (currentMode) {
            case Setup:
                setup();
                break;
            case Functional:
                run();
                break;
            case Refill:
                refill();
                break;
        }
    }
}
