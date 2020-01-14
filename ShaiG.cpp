#include "ShaiG.h"

void ShaiG::getResolution(unsigned &horizontal, unsigned &vertical){
    RECT desktop;
    GetWindowRect(GetDesktopWindow(),&desktop);
    horizontal = desktop.right;
    vertical = desktop.bottom;
}

void ShaiG::getResolutionMetric(unsigned &horizontal, unsigned &vertical){
    horizontal = GetSystemMetrics(SM_CXSCREEN);
    vertical = GetSystemMetrics(SM_CYSCREEN);
}

void ShaiG::pauseScreen(bool newLine) { 
    std::string temp;
    if (newLine) {
        std::cout << "\n\nPAUSE...";
        std::cin.ignore();
        std::getline(std::cin, temp);
    }
    else {
        std::cout << "\n\nPAUSE...";
        std::getline(std::cin, temp);
    }
}

bool ShaiG::durationPassed(
    std::chrono::time_point<std::chrono::high_resolution_clock>& baseTime,
    std::chrono::time_point<std::chrono::high_resolution_clock>& nowTime,
    std::chrono::minutes& duration
){
    nowTime = std::chrono::high_resolution_clock::now();
    return (nowTime-baseTime)>=duration;
}


void ShaiG::timer::startTimer(){
    start = std::chrono::high_resolution_clock::now();
    started = 1;
}
bool ShaiG::timer::endTimer(){
    if(started){
        end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float>tempDuration{end-start};
        timeElapsed = tempDuration.count(); started = 0;
        return 1;
    }else {timeElapsed = -1;return 0;}
}
float ShaiG::timer::getElapsedTime() const{return timeElapsed;}
ShaiG::timer::timer():started{0},timeElapsed{-1}{}


void ShaiG::strToArrayW(const std::string& str, wchar_t* wstr) {
    for (int c{ 0 }; c <= str.size(); c++) {
        if(c!= str.size())wstr[c] = str[c];
        else wstr[c] = '\0';
    }
}

void ShaiG::strToArrayW(const std::string& str, std::wstring& wstr) {
    wstr.resize(str.size());
    for (int c{ 0 }; c <= str.size(); c++) {
        wstr[c] = str[c];
    }
}

void ShaiG::getDateTimeStr(std::string& dateTime){
    //Get system time, create filepathname with systime in temp folder and create and open log file
    std::time_t sysTime;
    std::time(&sysTime);
    std::tm cTime;//calendar time
    //std::tm* resultPtr = &result;
    localtime_s(&cTime, &sysTime);//puts sysTime into tm obj which holds time as calendar time
    //places the tm obj with specific format into string buff, stringBuffO;fails if you try to put it directly into wostringstream 
    std::ostringstream stringBuffO;
    stringBuffO << std::put_time(&cTime, "%d.%m.%y_%H.%M.%S");
    dateTime = stringBuffO.str();
}

void ShaiG::getDateTimeStrW(std::wstring& dateTime){
    //Get system time, create filepathname with systime in temp folder and create and open log file
    std::time_t sysTime;
    std::time(&sysTime);
    std::tm cTime;//calendar time
    //std::tm* resultPtr = &result;
    localtime_s(&cTime, &sysTime);//puts sysTime into tm obj which holds time as calendar time
    //places the tm obj with specific format into string buff, stringBuffO;fails if you try to put it directly into wostringstream 
    std::ostringstream stringBuffO;
    stringBuffO << std::put_time(&cTime, "%d.%m.%y_%H.%M.%S");
    const std::string dateTimeTemp{stringBuffO.str()};
    strToArrayW(dateTimeTemp, dateTime);
}

    
void ShaiG::logFile::OPEN(std::wstring filename){
    file.open(filename,std::ios::trunc);open = 1;
}
void ShaiG::logFile::OPEN(std::wstring filename, bool dummy){
    file.open(filename,std::ios::app);open=1;
}