#pragma once

#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <iomanip>
#include "windows.h"

namespace ShaiG{
    void getResolution(unsigned &horizontal, unsigned &vertical);

    void getResolutionMetric(unsigned &horizontal, unsigned &vertical);

    void pauseScreen(bool newLine=1);

    bool durationPassed(
        std::chrono::time_point<std::chrono::high_resolution_clock>& baseTime,
        std::chrono::time_point<std::chrono::high_resolution_clock>& nowTime,
        std::chrono::minutes& duration
    );

    class timer{
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::time_point<std::chrono::high_resolution_clock> end;
        bool started;
        float timeElapsed;

        public:
        void startTimer();
        bool endTimer();
        float getElapsedTime() const;

        timer();
    };

    void strToArrayW(const std::string& str, wchar_t* wstr);

    void strToArrayW(const std::string& str, std::wstring& wstr);
    
    void getDateTimeStr(std::string& dateTime);

    void getDateTimeStrW(std::wstring& dateTime);

    class logFile{
        std::ofstream file;
        bool open;
        
        public:
        logFile():file{},open{0}{}
        logFile(std::wstring fileName):file{ fileName,std::ios::trunc},open{1}{}
        logFile(std::wstring fileName,bool dummy):file{ fileName,std::ios::app},open{1}{}
        ~logFile(){file<<'\n'<<'\n';file.close();}
        
        void OPEN(std::wstring filename);
        void OPEN(std::wstring filename, bool dummy);

        template<typename Twritable>
        bool write(Twritable txt,bool newLine=1){
            if(open){
                std::string dateTime;
                ShaiG::getDateTimeStr(dateTime);
                if(newLine)file<<dateTime<<"          "<<txt<<'\n';
                else file<<dateTime<<"          "<<txt;
                return 1;
            }
            else return 0;
        }   
    };
}