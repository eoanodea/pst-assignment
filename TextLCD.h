#ifndef TEXTLCD_H
#define TEXTLCD_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
using namespace std;

// Contains outputs the LCD screen and other status messages.
extern vector<string> testOutput;

/**
 * @brief Mockup for TextLCD
 *
 */
class TextLCD
{
public:
    TextLCD(string p29, string p30, string p28, string p27, string p26, string p25) {}

    // void printf(const char *format, ...)
    // {
    //     va_list args;
    //     va_start(args, format);
    //     vprintf(format, args);
    //     va_end(args);
    // }

    // Source: https://stackoverflow.com/questions/19009094/c-variable-arguments-with-stdstring-only
    void printf(const string format, ...)
    {
        const char *const zcFormat = format.c_str();

        // initialize use of the variable argument array
        va_list vaArgs;
        va_start(vaArgs, format);

        // reliably acquire the size from a copy of the variable argument array
        // and a functionally reliable call to mock the formatting
        va_list vaCopy;
        va_copy(vaCopy, vaArgs);
        const int iLen = std::vsnprintf(NULL, 0, zcFormat, vaCopy);
        va_end(vaCopy);

        // return a formatted string without risking memory mismanagement
        // and without assuming any compiler or platform specific behavior
        std::vector<char> zc(iLen + 1);
        std::vsnprintf(zc.data(), zc.size(), zcFormat, vaArgs);
        va_end(vaArgs);

        testOutput.push_back(string(zc.data(), zc.size() - 1));
    }

    void cls()
    {
        // testOutput += "\n";
    }
};

#endif
