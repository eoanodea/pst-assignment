#include "LCDErrorMsg.h"
#include "string"
using namespace std;

LCDErrorMsg::LCDErrorMsg(string str) {
    type = str;
    highMessage = "too high";
    lowMessage = "too low";
    high = false;
    display = false;
}

string LCDErrorMsg::getMsg() {
    if(high) return type + " " + highMessage;
    return type + " " + lowMessage;
}

void LCDErrorMsg::toggle(bool enable, bool isHigh) {
    display = enable;
    high = isHigh;
}