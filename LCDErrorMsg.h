#include "string"

using namespace std;

class LCDErrorMsg {
    public:
        string type;
        string highMessage;
        string lowMessage;
        bool high;
        bool display;

        LCDErrorMsg(string str);
        string getMsg();
        void toggle(bool enable, bool isHigh);
};
