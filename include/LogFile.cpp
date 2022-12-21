#include "LogFile.h"

#include <fstream>
#include <string>
#include <cstring>
#include <time.h>
#include <ctime>
#include <sstream>

using namespace std;

string DebugLog::DoubleToString(double Number){
    stringstream sTemp;
    string Result;

    sTemp << Number;
    sTemp >> Result;

    return Result;
}

string DebugLog::GetTime(bool Type){
    string Result;

    if(Type == true){
        string NowTime;
        time_t Time;
        tm *TmTime;

        time(&Time);
        TmTime = localtime(&Time);

        NowTime = (TmTime->tm_hour + ":" + TmTime->tm_min + ":" + TmTime->tm_sec);
        Result = NowTime;
    }
    else if(Type == false){
        clock_t Time;
        double NowTime;

        Time = clock();
        NowTime = (double)(((double)(Time - this->StartTime)) / CLOCKS_PER_SEC);
        Result = this->DoubleToString(NowTime);
    }

    return Result;
}

DebugLog::DebugLog(){
    this->StartTime = clock();

    this->File.open("Log.txt", ios::out | ios::trunc);
E:
    if(this->File.is_open() == false){
        goto E;
    }
    else if(this->File.is_open() == true){
        LogPrint("**Log Start**");
        return;
    }
}

DebugLog::~DebugLog(){
    this->LogPrint("**Log End**");
    this->File.close();
    return;
}

void DebugLog::LogPrint(string Data){
    string OutData;
    string Time[2];

    Time[0] = this->GetTime(true);
    Time[1] = this->GetTime(false);

    this->File << ("<" + Time[0] + ">" + "[" + Time[1] + "]" + Data);
    this->File << '\n';
    
    return;
}
