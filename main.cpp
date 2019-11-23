#define WINVER 0x0602
#define _WIN32_WINNT 0x0602
#define WIN32_LEAN_AND_MEAN
#define Debug

#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <thread>
#include <Windows.h>
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <VersionHelpers.h>
#include <intrin.h>

#pragma comment(lib, "Ws2_32.lib")


//////////////////////////////////////function templates/////////////////////////////////////////////
template <typename Tchar>
void zeroMemory(Tchar buff,unsigned size){
    for(unsigned c{0};c<size;c++){
        buff[c] = '\0';
    }
}
template <typename num>
double byteToGB(num number){return (number / 1073741824.00);}
//////////////////////////////////////function templates////////////////////////////////////////////


///////////////////////////////////User Defined Types//////////////////////////////////////////////
#pragma pack(push,1)
struct driveInfo{
        char root;
        unsigned lpSectorsPerCluster;
        unsigned lpBytesPerSector;
        unsigned lpNumberOfFreeClusters;
        unsigned lpTotalNumberOfClusters;

        unsigned long long lpFreeBytesAvailableToCaller;
        unsigned long long lpTotalNumberOfBytes;
        unsigned long long lpTotalNumberOfFreeBytes;
};

//this is a struct to serialize and send over the network
//it doesn't include the drive info as that will be sent separately
struct clientSystemInfo{
    //os info
    char hostName[25];
    char osVersion[50];
    unsigned osBuild[3];
    
    //processor info
    char cpuBitNum[15];
    unsigned coreCount;
    unsigned threadCount;
    char manufacturer[13];
    char cpuModelStr[75];

    //memory info
    double totalMemory;
    double availableMemory;
    unsigned percentInUse;

    clientSystemInfo(){
        zeroMemory<char*>(hostName,25);
        zeroMemory<char*>(osVersion,50);
        zeroMemory<char*>(cpuBitNum,15);
        zeroMemory<char*>(manufacturer,13);
        zeroMemory<char*>(cpuModelStr,75);
    }
};
#pragma pack(pop)
///////////////////////////////////User Defined Types//////////////////////////////////////////////


//////////////////////////////////Function Declarations///////////////////////////////////////////
std::wstring getHostName(); void getOSVer(std::wstring&,unsigned*); 
std::wstring getOSProduct(const DWORD); 
void getProcessorInfo(std::wstring&,unsigned&,char* manufacturer,char* cpuModelStr);
std::wstring getProcessorArchName(WORD); 
std::wstring getDriveInfo(); void getDriveBytes(std::wstring&, driveInfo*);
void printWcharT(wchar_t*,unsigned); void pause();
void printFormat(std::wstring string);void printE(std::string text);
//////////////////////////////////Function Declarations///////////////////////////////////////////

//////////////////////////////////Main Function//////////////////////////////////////////////
int wmain(){
    std::unique_ptr<clientSystemInfo>csiPtr{new clientSystemInfo};
    //get client host name
    std::wstring hostName; 
    hostName = getHostName();
    WideCharToMultiByte(CP_UTF8,0,hostName.data(),-1,csiPtr->hostName,25,NULL,NULL);
    #ifdef Debug
    std::wcout<<csiPtr->hostName<<L'\r'<<L'\n';
    #endif


    //get os version
    std::wstring osProdName;
    unsigned build[3];
    getOSVer(osProdName,csiPtr->osBuild);
    WideCharToMultiByte(CP_UTF8,0,osProdName.data(),-1,csiPtr->osVersion,50,NULL,NULL);
    #ifdef Debug
    std::wcout<<csiPtr->osVersion<<L'\r'<<L'\n'
    <<csiPtr->osBuild[0]<<L'.'<<csiPtr->osBuild[1]<<L'.'<<csiPtr->osBuild[2]
    <<L'\r'<<L'\n'<<L'\n';
    #endif
    

    //get CPU info
    std::wstring architecture;
    csiPtr->threadCount = std::thread::hardware_concurrency();
    std::unique_ptr<char>manufacturer{new char[13]{"\0\0\0\0\0\0\0\0\0\0\0\0"}};
    std::unique_ptr<char>cpuModelStr{new char[49]{
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    }}; getProcessorInfo(architecture,csiPtr->coreCount,csiPtr->manufacturer,csiPtr->cpuModelStr);
    WideCharToMultiByte(CP_UTF8,0,architecture.data(),-1,csiPtr->cpuBitNum,15,NULL,NULL);
    #ifdef Debug
    std::wcout<<csiPtr->cpuBitNum<<L'\r'<<L'\n'<<csiPtr->coreCount
    <<L'\r'<<L'\n'<<csiPtr->threadCount<<L'\r'<<L'\n'<<csiPtr->manufacturer
    <<L'\r'<<L'\n'<<csiPtr->cpuModelStr<<L'\r'<<L'\n'<<L'\n';
    #endif


    //get memory info
    MEMORYSTATUSEX memoryInfo{sizeof(MEMORYSTATUSEX)};//sets dwLength to size of struct
    GlobalMemoryStatusEx(&memoryInfo);
    //auto byteToGB = [](DWORDLONG num){return num / 1073741824.00;};//lambda: converts bytes to GB
    csiPtr->totalMemory = byteToGB<DWORDLONG>(memoryInfo.ullTotalPhys);
    csiPtr->availableMemory = byteToGB<DWORDLONG>(memoryInfo.ullAvailPhys);
    csiPtr->percentInUse = memoryInfo.dwMemoryLoad;
    #ifdef Debug
    std::wcout<<csiPtr->totalMemory<<L'\r'<<L'\n'<<csiPtr->availableMemory<<L'\r'<<L'\n'
    <<csiPtr->percentInUse<<'%'<<L" in use"<<L'\r'<<L'\n'<<L'\n';
    #endif


    //get drive info
    /*driveStr will hold list of drive letters
    each letter followed by a number that represents drive type
    '3' = Fixed drive ; '2' = Removable drive(ignored) ; '5' = DVD drive*/
    std::wstring driveStr;
    driveStr = getDriveInfo();
    unsigned driveCount{driveStr.length()/2};
    /* #ifdef Debug
    std::wcout<<driveCount<<L'\r'<<L'\n'<<driveStr<<L'\r'<<L'\n'<<L'\n';
    #endif */
    std::unique_ptr<driveInfo> driveInfoArray{new driveInfo[driveCount]};
    getDriveBytes(driveStr,driveInfoArray.get());
    
    #ifdef Debug
    printFormat(L"Drive");printFormat(L"Total Bytes");printFormat(L"Available Bytes");
    std::wcout<<L'\r'<<L'\n';
    for(unsigned c{0};c<driveCount;c++){
        std::wcout<<std::setw(15)<<std::left<<driveInfoArray.get()[c].root<<
        std::setw(15)<<std::right<<driveInfoArray.get()[c].lpTotalNumberOfBytes
        <<std::setw(15)<<std::right<<driveInfoArray.get()[c].lpTotalNumberOfFreeBytes<<L'\r'<<L'\n';
    }
    #endif

    #ifdef Debug
    std::wcout<<L'\r'<<L'\n'<<L'\n';
    #endif


    /////////////////////////////////////////////create client socket///////////////////////////////////
    std::wstring ipAddress;//server ip address
    unsigned port{62211};//server port
    std::wifstream getIPaddr("clientIP.txt");
    getIPaddr>>ipAddress;
    #ifdef Debug
    //std::wcout<<ipAddress<<L'\n';
    #endif

    //Initialize WinSock
    WSADATA wsaData;
    if((WSAStartup(MAKEWORD(2,2),&wsaData))!=0){
        #ifdef Debug
        printE("WSAstart Failed");
        #endif
        return 1;
    }
    
    //Create Socket
    SOCKET cSock{socket(AF_INET,SOCK_STREAM,IPPROTO_TCP)};
    if(cSock==INVALID_SOCKET){
        #ifdef Debug
        printE("cSock Failed");
        #endif
        WSACleanup(); return 1;
    }
    //Fill in a socket address structure
    sockaddr_in cSockAddr;
        cSockAddr.sin_family = AF_INET;
        cSockAddr.sin_port = htons(port);
        InetPtonW(AF_INET,ipAddress.data(),&cSockAddr.sin_addr);
    //connect to server
    if(
        (connect(
                cSock,reinterpret_cast<sockaddr*>(&cSockAddr),
                sizeof(cSockAddr))
        )!=0
    ){closesocket(cSock);printE("failed to connect");WSACleanup();return 1;}
    //Do-While loop to send and receive data
    char* serializedData1{reinterpret_cast<char*>(csiPtr.get())};
    char* serializedData2{reinterpret_cast<char*>(driveInfoArray.get())};
    char confirmReception[7];
    if(send(cSock,serializedData1,sizeof(clientSystemInfo),MSG_OOB)==SOCKET_ERROR){
        #ifdef Debug
        printE("send1 failed");
        #endif
        closesocket(cSock);WSACleanup();return 1;
    }

    if(send(cSock,reinterpret_cast<char*>(&driveCount),4,MSG_OOB)==SOCKET_ERROR){
        #ifdef Debug
        printE("send2 failed");
        #endif
        closesocket(cSock);WSACleanup();return 1;
    }
    
    if(send(cSock,serializedData2,sizeof(driveInfo)*driveCount,MSG_OOB)==SOCKET_ERROR){
        #ifdef Debug
        printE("send3 failed");
        #endif
        closesocket(cSock);WSACleanup();return 1;
    }
    
    /* if(recv(cSock,confirmReception,7,MSG_WAITALL)==SOCKET_ERROR){
        closesocket(cSock);WSACleanup();return 1;
    } */

    #ifdef Debug
    //std::wcout<<confirmReception<<L'\n';
    #endif
    /* if(recv(cSock,confirmReception,7,MSG_WAITALL)==SOCKET_ERROR){
        closesocket(cSock);WSACleanup();return 1;
    } */
    
    //Gracefully close down everything
    closesocket(cSock);WSACleanup();
    #ifdef Debug
    //std::wcout<<confirmReception<<L'\n';
    #endif
    //////////////////////////////////////////create client socket/////////////////////////////////////
    
    //Exit
    std::wcout.flush();
    delete serializedData1;delete []serializedData2;
    pause();
    return 0;
}
//////////////////////////////////Main Function//////////////////////////////////////////////




/////////////////////////////////Function Definitions/////////////////////////////////////////

std::wstring getHostName(){
    unsigned long hostNameLen;
    GetComputerNameExW(ComputerNameDnsHostname,NULL,&hostNameLen);
    std::unique_ptr<wchar_t> hostName{new wchar_t[hostNameLen]};
        //if returns 0 error
    if(GetComputerNameExW(ComputerNameDnsHostname,hostName.get(),&hostNameLen)==0)return L"HostNameError";
    else return hostName.get(); 
}

void getOSVer(std::wstring& productName,unsigned* build){
    unsigned fileVerInfoSize{GetFileVersionInfoSizeExW(FILE_VER_GET_LOCALISED,L"Kernel32.dll",NULL)};
    std::unique_ptr<char> fileInfo{new char[fileVerInfoSize]};
    void* fileInfoPtr{nullptr};
            //returns bool need to add conditioanl statement to test success
    GetFileVersionInfoExW(FILE_VER_GET_LOCALISED,L"Kernel32.dll",NULL,fileVerInfoSize,fileInfo.get());
    LPCWSTR file{L"\\VarFileInfo\\Translation"};
    PUINT fileLen{0};
    VerQueryValueW(fileInfo.get(),L"\\",&fileInfoPtr,fileLen);
    const VS_FIXEDFILEINFO* fileInfoStruct{static_cast<const VS_FIXEDFILEINFO*>(fileInfoPtr)};

	DWORD product{};
	GetProductInfo(
		HIWORD(fileInfoStruct->dwFileVersionMS),
		LOWORD(fileInfoStruct->dwFileVersionMS),
		HIWORD(fileInfoStruct->dwFileVersionLS),
		LOWORD(fileInfoStruct->dwFileVersionLS),
		&product			
	);
    productName = getOSProduct(product);
    build[0] = HIWORD(fileInfoStruct->dwFileVersionMS);
    build[1] = LOWORD(fileInfoStruct->dwFileVersionMS);
    build[2] = HIWORD(fileInfoStruct->dwFileVersionLS);
}

std::wstring getOSProduct(const DWORD product){
    switch (product){
        case 0x00000030:return L"Windows 10 Pro";
        case 0x00000065:return L"Windows 10 Home";
        case 0x00000004:return L"Windows 10 Enterprise";
        case 0x0000002A:return L"Microsoft Hyper-V Server";
        case 0x0000000A:return L"Server Enterprise(full)";
        case 0x0000000E:return L"Server Enterprise(core)";
        case 0x00000007:return L"Server Standard(full)";
        case 0x00000006:return L"Business";
        case 0x00000010:return L"Business N";
        case 0x00000012:return L"HPC Edition";
        case 0x00000040:return L"Server Hyper Core V";
        case 0x00000063:return L"Windows 10 Home China";
        case 0x00000062:return L"Windows 10 Home N";
        case 0x00000064:return L"Windows 10 Home Single Language";
        case 0x00000050:return L"Server Datacenter Evaluation";
        case 0x00000091:return L"Server Datacenter, Semi-Annual(core)";
        case 0x00000092:return L"Server Standard, Semi-Annual(core)";
        case 0x00000008:return L"Server Datacenter (full)";
        case 0x0000000C:return L"Server Datacenter (core, 2008 R2 and earlier)";
        case 0x00000027:return L"Server Datacenter without Hyper-V(core)";
        case 0x00000025:return L"Server Datacenter without Hyper-V(full)";
        case 0x00000079:return L"Windows 10 Education";
        case 0x0000007A:return L"Windows 10 Education N";
        case 0x00000046:return L"Windows 10 Enterprise E";
        case 0x00000048:return L"Windows 10 Enterprise Evaluation";
        case 0x0000001B:return L"Windows 10 Enterprise N";
        case 0x00000054:return L"Windows 10 Enterprise N Evaluation";
        case 0x0000007D:return L"Windows 10 Enterprise 2015 LTSB";
        case 0x00000081:return L"Windows 10 Enterprise 2015 LTSB Evaluation";
        case 0x0000007E:return L"Windows 10 Enterprise 2015 LTSB N";
        case 0x00000082:return L"Windows 10 Enterprise 2015 LTSB N Evaluation";
        case 0x00000029:return L"Server Enterprise without Hyper-V(core)";
        case 0x0000000F:return L"Server Enterprise Itanium based systems";
        case 0x00000026:return L"Server Enterprise without Hyper-V(full)";
        case 0x0000003C:return L"Windows Essential Server Solution Additional";
        case 0x0000003E:return L"Windows Essential Server Solution Additional SVC";
        case 0x0000003B:return L"Windows Essential Server Solution Management";
        case 0x0000003D:return L"Windows Essential Server Solution Management SVC";
        case 0x00000002:return L"Home Basic";
        case 0x00000005:return L"Home Basic N";
        case 0x00000003:return L"Home Premium";
        case 0x0000001A:return L"Home Premium N";
        case 0x00000022:return L"Windows Home Server 2011";
        case 0x00000013:return L"Windows Storage Server 2008 R2 Essentials";
        case 0x0000007B:return L"Windows 10 IoT Core";
        case 0x00000083:return L"Windows 10 IoT Core Commercial";
        case 0x0000001E:return L"Windows Essential Business Server Management Server";
        case 0x00000020:return L"Windows Essential Business Server Messaging Server";
        case 0x0000001F:return L"Windows Essential Business Server Security Server";
        case 0x00000068:return L"Windows 10 Mobile";
        case 0x00000085:return L"Windows 10 Mobile Enterprise";
        case 0x0000004D:return L"Windows Multipoint Server Premium(full)";
        case 0x0000004C:return L"Windows Multipoint Server Standard(full)";
        case 0x000000A1:return L"Windows 10 Pro for Workstations";
        case 0x000000A2:return L"Windows 10 Pro for Workstations N";
        case 0x00000031:return L"Windows 10 Pro N";
        case 0x00000067:return L"Professional with Media Center";
        case 0x00000001:return L"Ultimate";
        case 0x0000001C:return L"Ultimate N";
        case 0x00000011:return L"Web Server(full)";
        case 0x0000001D:return L"Web Server(core)";
        case 0x0000004F:return L"Server Standard Evaluation";
        case 0x0000000D:return L"Server Standard(core) 2008 R2 and earlier";
        case 0x00000028:return L"Server Standard without Hyper-V(core)";
        case 0x00000024:return L"Server Standard without Hyper-V";
        
        default: return L"N\\A";
    }
}

void getProcessorInfo(
    std::wstring& cpuArchName,unsigned& coreCount,
    char* manufacturer,char* cpuModelStr
){
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    cpuArchName = getProcessorArchName(sysInfo.wProcessorArchitecture);
    coreCount = sysInfo.dwNumberOfProcessors;
    
    int processorDetails[4];
    __cpuid(processorDetails,0);
    memcpy(manufacturer,&processorDetails[1],4);
    memcpy(manufacturer+4,&processorDetails[3],4);
    memcpy(manufacturer+8,&processorDetails[2],4);

    char cpuStr[]{
        "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    };
    unsigned charIndex{0};
    for(unsigned c{0x80000002};c<0x80000005;c++){
        __cpuid(processorDetails,c);
        memcpy(cpuStr+charIndex,&processorDetails,16);
        charIndex+=16;    
    }
    
    unsigned brandIndex{0};
    for(unsigned c{0};c<48;c++){
        if(cpuStr[c]!='\0'&&cpuStr[c]!=' '){
            cpuModelStr[brandIndex] = cpuStr[c];
            brandIndex++;
            for (unsigned d{c+1};d<48;d++){
                if (cpuStr[d]!='\0'){
                    cpuModelStr[brandIndex] = cpuStr[d];
                    brandIndex++;
                }else break;
            }
            break;
        }
    }
}

std::wstring getProcessorArchName(WORD archNum){
    switch (archNum){
        case 9:return L"AMD64";
        case 0:return L"AMDx86";
        case 5:return L"ARM";
        case 12:return L"ARM64";
        case 6:return L"Itanium";
        
        default:return L"N\\A";
    }
}

std::wstring getDriveInfo(){
    DWORD driveInfoSize;
    driveInfoSize = GetLogicalDriveStringsW(NULL,NULL);
    std::unique_ptr<wchar_t> driveInfoBuffer{new wchar_t[driveInfoSize]};
    zeroMemory<wchar_t*>(driveInfoBuffer.get(),driveInfoSize);
    GetLogicalDriveStringsW(driveInfoSize,driveInfoBuffer.get());
    unsigned driveCount {(driveInfoSize-1)/4};
    std::unique_ptr<wchar_t> driveLetters{new wchar_t[driveCount+driveCount+1]};
    zeroMemory<wchar_t*>(driveLetters.get(),driveCount+driveCount+1);
    
    wchar_t driveRoot[4]{L"\0\0\0"};
    driveRoot[1] = L':';driveRoot[2] = L'\\';
    unsigned driveBuffIndex{0};
    unsigned driveLetterIndex{0};
    for(unsigned c{0};c<driveCount;c++){
        driveRoot[0] = driveInfoBuffer.get()[driveBuffIndex];
        //'3' = Fixed drive ; '2' = Removable drive(ignored) ; '5' = DVD drive
        if(GetDriveTypeW(driveRoot)==3)driveLetters.get()[driveLetterIndex+1] = L'3';
        else if(GetDriveTypeW(driveRoot)==5)driveLetters.get()[driveLetterIndex+1] = L'5';
        else {driveBuffIndex+=4; continue;}
        driveLetters.get()[driveLetterIndex] = driveInfoBuffer.get()[driveBuffIndex];
        driveBuffIndex+=4;
        driveLetterIndex+=2;
    }
    /* #ifdef Debug
    printWcharT(driveInfoBuffer.get(),driveInfoSize);
    std::wcout<<'\r'<<'\n';
    printWcharT(driveLetters.get(),driveCount+driveCount);
    std::wcout<<'\r'<<'\n';
    #endif */
    return driveLetters.get();
}

void getDriveBytes(std::wstring& driveStr, driveInfo* driveInfoArray){
    struct diskStruct{
        DWORD* lpSectorsPerCluster;
        DWORD* lpBytesPerSector;
        DWORD* lpNumberOfFreeClusters;
        DWORD* lpTotalNumberOfClusters;

        ULARGE_INTEGER* lpFreeBytesAvailableToCaller;
        ULARGE_INTEGER* lpTotalNumberOfBytes;
        ULARGE_INTEGER* lpTotalNumberOfFreeBytes;

        diskStruct():
            lpSectorsPerCluster{new DWORD},
            lpBytesPerSector{new DWORD},
            lpNumberOfFreeClusters{new DWORD},
            lpTotalNumberOfClusters{new DWORD},

            lpFreeBytesAvailableToCaller{new ULARGE_INTEGER},
            lpTotalNumberOfBytes{new ULARGE_INTEGER},
            lpTotalNumberOfFreeBytes{new ULARGE_INTEGER}
        {}
        
        ~diskStruct(){
            delete lpSectorsPerCluster;
            delete lpBytesPerSector;
            delete lpNumberOfFreeClusters;
            delete lpTotalNumberOfClusters;

            delete lpFreeBytesAvailableToCaller;
            delete lpTotalNumberOfBytes;
            delete lpTotalNumberOfFreeBytes;
        }
    };

    unsigned long long* lpFreeBytesAvailableToCallerPtr{nullptr};
    unsigned long long* lpTotalNumberOfBytesPtr{nullptr};
    unsigned long long* lpTotalNumberOfFreeBytesPtr{nullptr};
    wchar_t rootDiskStr[3];rootDiskStr[2] = L'\0';rootDiskStr[1] = L':';
    unsigned indexDIA{0};
    diskStruct diskInfo;
    for(unsigned c{0};c<driveStr.length();c+=2){
        rootDiskStr[0] = driveStr[c];
        GetDiskFreeSpaceW(
            rootDiskStr,
            diskInfo.lpSectorsPerCluster,
            diskInfo.lpBytesPerSector,
            diskInfo.lpNumberOfFreeClusters,
            diskInfo.lpTotalNumberOfClusters
            );
        GetDiskFreeSpaceExW(
            rootDiskStr,
            diskInfo.lpFreeBytesAvailableToCaller,
            diskInfo.lpTotalNumberOfBytes,
            diskInfo.lpTotalNumberOfFreeBytes
        );
        driveInfoArray[indexDIA].root = driveStr[c]; //compiler needs to convert wchar(of wstring) to char
        driveInfoArray[indexDIA].lpSectorsPerCluster = *(diskInfo.lpSectorsPerCluster);
        driveInfoArray[indexDIA].lpBytesPerSector = *(diskInfo.lpBytesPerSector);
        driveInfoArray[indexDIA].lpNumberOfFreeClusters = *(diskInfo.lpNumberOfFreeClusters);
        driveInfoArray[indexDIA].lpTotalNumberOfClusters = *(diskInfo.lpTotalNumberOfClusters);
        
        unsigned long long* lpFreeBytesAvailableToCallerPtr = reinterpret_cast<unsigned long long*>(diskInfo.lpTotalNumberOfFreeBytes);
        unsigned long long* lpTotalNumberOfBytesPtr = reinterpret_cast<unsigned long long*>(diskInfo.lpTotalNumberOfBytes);
        unsigned long long* lpTotalNumberOfFreeBytesPtr = reinterpret_cast<unsigned long long*>(diskInfo.lpTotalNumberOfFreeBytes);
        driveInfoArray[indexDIA].lpFreeBytesAvailableToCaller = *(lpFreeBytesAvailableToCallerPtr);
        driveInfoArray[indexDIA].lpTotalNumberOfBytes = *(lpTotalNumberOfBytesPtr);
        driveInfoArray[indexDIA].lpTotalNumberOfFreeBytes = *(lpTotalNumberOfFreeBytesPtr);

        indexDIA++;
    }
    delete lpFreeBytesAvailableToCallerPtr;
    delete lpTotalNumberOfBytesPtr;
    delete lpTotalNumberOfFreeBytesPtr;
}

void pause(){
    std::wcout<<L"Pause... ";
    std::wcin.get();
}

#ifdef Debug
void printWcharT(wchar_t* str,unsigned size){
    for(unsigned c{0};c<size;c++){
        std::wcout<<str[c]<<'\r'<<'\n';
    }
}
#endif

/* void zeroMemory(wchar_t* buff,unsigned size){
    for(unsigned c{0};c<size;c++){
        buff[c] = L'\0';
    }
} */

void printFormat(std::wstring string){
    std::wcout.width(15);
    std::wcout<<std::left<<string;
}

void printE(std::string text){
    std::cout<<text<<std::endl;
}
/////////////////////////////////Function Definitions/////////////////////////////////////////