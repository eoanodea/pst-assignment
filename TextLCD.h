#ifndef TEXTLCD_H
#define TEXTLCD_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <stdexcept>
using namespace std;

/**
 * @brief Mockup for TextLCD
 *
 */
class TextLCD
{
public:
    TextLCD(string p29, string p30, string p28, string p27, string p26, string p25) {}

    void printf(const char* format, ...)
    {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }

    // template<typename ... Args>
    // void printf(const string& format, Args ... args)
    // {
    //     int size_s = snprintf( nullptr, 0, format.c_str(), args ... ) + 1; // Extra space for '\0'
    //     if( size_s <= 0 ){ throw runtime_error( "Error during formatting." ); }
    //     auto size = static_cast<size_t>( size_s );
    //     unique_ptr<char[]> buf( new char[ size ] );
    //     snprintf( buf.get(), size, format.c_str(), args ... );
    //     string str = string( buf.get(), buf.get() + size - 1 );

    //     char out[32];
    //     // for (int i = 0; i < str.length(); i++) {
    //     //     out[i] = str[i] % 32;
    //     // }

    //     printf("%s", out);
    // }

    void cls() {}
};


#endif
