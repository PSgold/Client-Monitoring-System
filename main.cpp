#define WINVER 0x0602
#define _WIN32_WINNT 0x0602
#define WIN32_LEAN_AND_MEAN
#define SERVICE
//#define DEBUGLOOP 


#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <filesystem>
#include <memory>
#include "Windows.h"
#include "io.h"
#include "fcntl.h"
#include "WinSock2.h"
#include "ws2tcpip.h"
#include "VersionHelpers.h"
#include "intrin.h"
#include "worker.h"

#pragma comment(lib, "advapi32.lib")

namespace chTime = std::chrono;
typedef chTime::high_resolution_clock chTimeHRC;

//////////////////////////////////FUNCTIONS///////////////////////////////////////////
bool strcmpW(wchar_t* strA, wchar_t* strB);
void callWorker();

template <typename Tchar>
void addStrToBuff(Tchar* buff, Tchar* str){
    unsigned endIndex{0};
	while(buff[endIndex]!='\0')++endIndex;
    for(unsigned c{0};str[c]!='\0';++c){
        buff[endIndex] = str[c]; ++endIndex;
    }
}

template <typename Tchar>
void remEndPath(Tchar* str){
	unsigned endIndex{0};
	while(str[endIndex]!='\0')++endIndex;
	while(str[endIndex]!='\\'){
		str[endIndex] = '\0';
		--endIndex;
	}
}
//////////////////////////////////FUNCTIONS///////////////////////////////////////////


//////////////////////////////////Windows Service Global Definitions/variables and Function Declarations///////////////////////////////////////////
#define SERVICENAME L"ClientMS"
#define SERVICEDISPLAYNAME L"Client Monitoring Service"
SERVICE_STATUS_HANDLE statusHandle;
SERVICE_STATUS svcStatusStruct;
HANDLE svcStopEvent{INVALID_HANDLE_VALUE};

void svcInstall(ShaiG::logFile& iLog, std::wstring& svcPath);
void svcUnInstall();
void WINAPI svcMainW(DWORD argc, LPWSTR* argv);
void WINAPI svcCtrlHandler(DWORD dwCtrl);

//////////////////////////////////Windows Service Global Definitions/variables and Function Declarations///////////////////////////////////////////
std::mutex globalMutex;
std::condition_variable cv;
ShaiG::logFile dLog;
int wmain(int argc, wchar_t* argv[]){
    #ifndef SERVICE
    std::ios_base::sync_with_stdio(0);
    chTime::minutes Minutes2{2};
    chTime::minutes Minutes9{9};
    chTime::time_point<chTimeHRC> newTime;
    chTime::time_point<chTimeHRC> newTime2;
    chTime::time_point<chTimeHRC> baseTime{chTimeHRC::now()};
    chTime::time_point<chTimeHRC> baseTime2{chTimeHRC::now()};

    unsigned count{0};
    while(count<3){
        if (count==0){
            baseTime = chTimeHRC::now();
            std::cout<<"starting work\n";
            startWork();++count;
            std::wcout<<"End work\n";
        }
        if(ShaiG::durationPassed(baseTime2,newTime2,Minutes9))break;
        else if(ShaiG::durationPassed(baseTime,newTime,Minutes2)){
            baseTime = chTimeHRC::now();
            std::cout<<"starting work\n";
            startWork();++count;
            std::wcout<<"End work\n";
        }
    }
    std::wcout<<"End while loop\n";
    return 0;
    #endif
    if (argc>1){
        wchar_t installStr[]{L"/install"};
        wchar_t unInstallStr[]{L"/delete"};
        if (strcmpW(argv[1],installStr)){
            std::wstring svcPath(MAX_PATH,L'\0');
            GetModuleFileNameW( NULL, svcPath.data(), MAX_PATH );
            std::wstring svcProcPath{svcPath};
            remEndPath(svcPath.data());wchar_t ilFileName[]{L"InstallLog.txt"};
            addStrToBuff(svcPath.data(),ilFileName);
            ShaiG::logFile iLog{svcPath,1};
            //set console input and output to unicode
            _setmode(_fileno(stdin), _O_U16TEXT);

            //disable console quick edit mode so mouse click won't pause the process
            DWORD consoleMode;
            HANDLE inputHandle{ GetStdHandle(STD_INPUT_HANDLE) };
            GetConsoleMode(inputHandle, &consoleMode);
            SetConsoleMode(inputHandle, consoleMode & (~ENABLE_QUICK_EDIT_MODE));
            svcInstall(iLog, svcProcPath);return 0;
        }else if(strcmpW(argv[1],unInstallStr)){
            //set console input and output to unicode
            _setmode(_fileno(stdin), _O_U16TEXT);

            //disable console quick edit mode so mouse click won't pause the process
            DWORD consoleMode;
            HANDLE inputHandle{ GetStdHandle(STD_INPUT_HANDLE) };
            GetConsoleMode(inputHandle, &consoleMode);
            SetConsoleMode(inputHandle, consoleMode & (~ENABLE_QUICK_EDIT_MODE));
            svcUnInstall();return 0;
        }
        else {
            std::wstring svcPath(MAX_PATH,L'\0');
            GetModuleFileNameW( NULL, svcPath.data(), MAX_PATH );
            remEndPath(svcPath.data());wchar_t lFileName[]{L"Log.txt"};
            addStrToBuff(svcPath.data(),lFileName);
            dLog.OPEN(svcPath,1);    
            dLog.write("Service was run with unapproved argument!");
            return 0;
        } 
    }
    std::wstring svcPath(MAX_PATH,L'\0');
    GetModuleFileNameW( NULL, svcPath.data(), MAX_PATH );
    remEndPath(svcPath.data());wchar_t lFileName[]{L"Log.txt"};
    addStrToBuff(svcPath.data(),lFileName);
    dLog.OPEN(svcPath,1);    
    SERVICE_TABLE_ENTRYW serviceTable[]{
        {const_cast<LPWSTR>(SERVICENAME),static_cast<LPSERVICE_MAIN_FUNCTIONW>(svcMainW)},
        {NULL,NULL}
    };

    if(StartServiceCtrlDispatcherW(serviceTable)==0){
        dLog.write("StartServiceCtrlDipatcher Failed!");
        return 0;
    }
    dLog.write("Service Stoppped");
    return 0;
}

void WINAPI svcMainW(DWORD argc, LPWSTR* argv){
    statusHandle = RegisterServiceCtrlHandlerW(SERVICENAME,svcCtrlHandler);
    if (statusHandle==0)return;
    ZeroMemory(&svcStatusStruct,sizeof(svcStatusStruct));
    svcStatusStruct.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    svcStatusStruct.dwServiceSpecificExitCode = 0;
    svcStatusStruct.dwCurrentState = SERVICE_START_PENDING;
    svcStatusStruct.dwWin32ExitCode = 0;
    svcStatusStruct.dwCheckPoint = 0;

    if(SetServiceStatus(statusHandle,&svcStatusStruct)==0)return;
    
    svcStopEvent = CreateEventW(NULL,TRUE,FALSE,NULL);
    if(svcStopEvent==NULL) {
        svcStatusStruct.dwControlsAccepted = 0;
        svcStatusStruct.dwCurrentState = SERVICE_STOPPED;
        svcStatusStruct.dwWin32ExitCode = GetLastError();
        svcStatusStruct.dwCheckPoint = 1;
        SetServiceStatus(statusHandle,&svcStatusStruct);
        return;
    }
    
    svcStatusStruct.dwCurrentState = SERVICE_RUNNING;
    svcStatusStruct.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    svcStatusStruct.dwWin32ExitCode = 0;
    svcStatusStruct.dwCheckPoint = 0;

    SetServiceStatus(statusHandle,&svcStatusStruct);
    
    dLog.write("Service Started Successfully");

    #ifdef DEBUGLOOP
    callWorker();
    #endif

   #ifndef DEBUGLOOP
    chTime::minutes minutes2{2};
    std::unique_lock globalLocked(globalMutex);
    int swrt;//Start Work return code
    while(WaitForSingleObject(svcStopEvent,0)!=WAIT_OBJECT_0){
        dLog.write("Calling startWork");
        swrt = startWork(dLog);
        if(swrt==1)dLog.write("startWork Successfully returned");
        else {
            dLog.write("startWork Failed. Error Code ",0);
            dLog.write(swrt);
        }
        cv.wait_for(globalLocked,minutes2);
    }
    #endif

    CloseHandle(svcStopEvent);

    svcStatusStruct.dwControlsAccepted = 0;
    svcStatusStruct.dwCurrentState = SERVICE_STOPPED;
    svcStatusStruct.dwWin32ExitCode = 0;
    svcStatusStruct.dwCheckPoint = 3;
    dLog.write("Service Stopping");
    SetServiceStatus(statusHandle,&svcStatusStruct);

    return;
}

void WINAPI svcCtrlHandler(DWORD dwCtrl){
     // Handle the requested control code. 
    switch(dwCtrl) {  
      case SERVICE_CONTROL_STOP: 
        if(svcStatusStruct.dwCurrentState != SERVICE_RUNNING) break;
        svcStatusStruct.dwControlsAccepted = 0;
        svcStatusStruct.dwCurrentState = SERVICE_STOP_PENDING;
        svcStatusStruct.dwWin32ExitCode = 0;
        svcStatusStruct.dwCheckPoint = 4; 
        SetEvent(svcStopEvent);
        cv.notify_one();
        break;
      
      default: 
         break;
   }
   return; 
}

bool strcmpW(wchar_t* strA, wchar_t* strB){
    for(unsigned c{0};1;++c){
        if (strA[c]!=strB[c])return 0;
        if (strA[c] == L'\0')return 1;
    }
}

void callWorker(){
    std::wstring svcPathN(MAX_PATH,L'\0');
    GetModuleFileNameW( NULL, svcPathN.data(), MAX_PATH );
    remEndPath(svcPathN.data());wchar_t lFileName[]{L"DebugLoop.txt"};
    addStrToBuff(svcPathN.data(),lFileName);
    ShaiG::logFile debugLoop{svcPathN};
    chTime::minutes minutes2{2};
    std::unique_lock globalLocked(globalMutex);
    while(WaitForSingleObject(svcStopEvent,0)!=WAIT_OBJECT_0){
        debugLoop.write("Debugging Loop!");
        cv.wait_for(globalLocked,minutes2);
    }
}

void svcInstall(ShaiG::logFile& iLog,std::wstring& svcPath){
    std::cout<<"Installing Service\n";
    iLog.write("Installing clientMS service");
    
    //wchar_t szPath[MAX_PATH];//buffer to store full service path
    //copies process name to szPath buffer
    //GetModuleFileNameW( NULL, szPath, MAX_PATH );

    // Get a handle to the SCM database. 
    SC_HANDLE schSCManager {
        OpenSCManagerW( 
            NULL,                         // local computer
            SERVICES_ACTIVE_DATABASEW,    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS         // full access rights
        )
    };   
 
    if (schSCManager == NULL){
        dLog.write("Failed to connect to SCM database");
        return;
    };

    // Create the service
    SC_HANDLE schService {
        CreateServiceW( 
            schSCManager,              // SCM database 
            SERVICENAME,               // name of service 
            SERVICEDISPLAYNAME,        // service name to display 
            SERVICE_ALL_ACCESS,        // desired access 
            SERVICE_WIN32_OWN_PROCESS, // service type 
            SERVICE_AUTO_START,        // start type 
            SERVICE_ERROR_NORMAL,      // error control type 
            svcPath.data(),            // path to service's binary 
            NULL,                      // no load ordering group 
            NULL,                      // no tag identifier 
            NULL,                      // no dependencies 
            NULL,                      // LocalSystem account 
            NULL                       // no password 
        )
    };                     
 
    if (schService == NULL){
        iLog.write("Create service failed");
        iLog.write("Installation Failed");
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
    }
    else{
        std::wcout<<"Service, "<<SERVICENAME<<", installed successfully\n";
        iLog.write("Create Service Successfull");
        iLog.write("Installation Successfull");
        _SERVICE_DESCRIPTIONW svcDescriptStruct;
        wchar_t svcDescript[]{
            L"A monitoring service that sends client information to server."
        };
        svcDescriptStruct.lpDescription = svcDescript;
        if(ChangeServiceConfig2W(schService,1,&svcDescriptStruct)==0){
            iLog.write("Set service description Failed");
        }else{
            iLog.write("Set service description Successfull");
        }

        StartServiceW(schService,NULL,NULL);
        CloseServiceHandle(schService); 
        CloseServiceHandle(schSCManager);
    }
}

void svcUnInstall(){
    // Get a handle to the SCM database. 
    SC_HANDLE schSCManager {
        OpenSCManagerW( 
            NULL,                         // local computer
            SERVICES_ACTIVE_DATABASEW,    // ServicesActive database 
            SC_MANAGER_ALL_ACCESS         // full access rights
        )
    };

    if (schSCManager == NULL){
        std::wcout<<L"Delete Sevice Failed\n";
        dLog.write("Failed to connect to SCM database");
        return;
    };

    SC_HANDLE schSvc {
        OpenServiceW(
            schSCManager,
            SERVICENAME,
            SERVICE_ALL_ACCESS
        )
    };

    if (schSvc == NULL){
        std::wcout<<L"Delete Sevice Failed\n";
        dLog.write("Open Service Failed");
        CloseServiceHandle(schSvc); 
        CloseServiceHandle(schSCManager);
        return;
    }

    if(DeleteService(schSvc)==0){
        std::wcout<<L"Delete Sevice Failed\n";
        dLog.write("Delete Service Failed");
        CloseServiceHandle(schSvc); 
        CloseServiceHandle(schSCManager);
    }else{
        std::wcout<<L"Deleted Service, "<<SERVICENAME
        <<L", Successfully\n";
        CloseServiceHandle(schSvc); 
        CloseServiceHandle(schSCManager);
    }
}

/* void workerThread(){
    std::ofstream svcTest{"c:\\Program Files\\svcTest\\checking.txt",std::ios::trunc};
    
    chTime::minutes twoMinutes{2};
    chTime::time_point<chTimeHRC> newTime;
    chTime::time_point<chTimeHRC> baseTime{
        chTimeHRC::now()
    };

    while(WaitForSingleObject(svcStopEvent,0)!=WAIT_OBJECT_0){
        if(ShaiG::durationPassed(baseTime,newTime,twoMinutes)){
            baseTime = chTimeHRC::now();
            svcTest<<"checking\n";
        }
    }
    svcTest.close();
    return;
} */