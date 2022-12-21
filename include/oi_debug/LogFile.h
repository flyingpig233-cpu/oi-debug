#include <fstream>
#include <string>
#include <cstring>
#include <time.h>
#include <ctime>

using namespace std;

#ifndef __LogFILE_H__
#define __LogFILE_H__

class DebugLog {
    private:
        ofstream File;
        clock_t StartTime;
        string GetTime(bool Type);
        string DoubleToString(double Number);

    public:
        DebugLog();
        ~DebugLog();
        void LogPrint(string Data);
};

#endif  //LogFile.h
