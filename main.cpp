#define WINVER 0x0602
#define _WIN32_WINNT 0x0602

#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <Windows.h>
#include <VersionHelpers.h>
#include <intrin.h>

void getHostName(); void getOSVer(); std::wstring getOSProduct(const DWORD);
std::wstring getProcessorArchName(WORD); void pause();

int wmain(){
    //get client host name
    unsigned long hostNameLen;
    GetComputerNameExW(ComputerNameDnsHostname,NULL,&hostNameLen);
    std::unique_ptr<wchar_t> hostName{new wchar_t[hostNameLen]};
            //if returns 0 error
    if(GetComputerNameExW(ComputerNameDnsHostname,hostName.get(),&hostNameLen)==0){
        std::wcout<<L"Error getting host name"<<L'\n';
    }else std::wcout<<hostName<<L'\n';

    //get os version
    unsigned fileVerInfoSize{GetFileVersionInfoSizeExW(FILE_VER_GET_LOCALISED,L"Kernel32.dll",NULL)};
    //char* fileInfo = new char[fileVerInfoSize];
    std::unique_ptr<char> fileInfo{new char[fileVerInfoSize]};
    void* fileInfoPtr{nullptr};
            //returns bool need to add conditioanl statement to test success
    GetFileVersionInfoExW(FILE_VER_GET_LOCALISED,L"Kernel32.dll",NULL,fileVerInfoSize,fileInfo.get());
    LPCWSTR file{L"\\VarFileInfo\\Translation"};
    PUINT fileLen{0};
    VerQueryValueW(fileInfo.get(),L"\\",&fileInfoPtr,fileLen);
    const VS_FIXEDFILEINFO* fileInfoStruct{static_cast<const VS_FIXEDFILEINFO*>(fileInfoPtr)};
    enum windowsType{Workstation,Server};windowsType osType;
    (IsWindowsServer())?(osType = Server):(osType = Workstation);
    std::wcout << HIWORD(fileInfoStruct->dwFileVersionMS) << '.'
            << LOWORD(fileInfoStruct->dwFileVersionMS) << '.'
            << HIWORD(fileInfoStruct->dwFileVersionLS) << " ";
    (osType == Workstation) ? std::wcout<<L"Workstation"<<L'\n' : std::wcout<<L"Server"<<L'\n';

	DWORD product{};
	GetProductInfo(
		HIWORD(fileInfoStruct->dwFileVersionMS),
		LOWORD(fileInfoStruct->dwFileVersionMS),
		HIWORD(fileInfoStruct->dwFileVersionLS),
		LOWORD(fileInfoStruct->dwFileVersionLS),
		&product			
	);
    
    std::wstring productName{getOSProduct(product)};
    std::wcout<<productName<<L'\n'<<L'\n'<<L'\n';
	//get CPU info

    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);
    std::wstring processorArchName;
    processorArchName = getProcessorArchName(sysInfo.wProcessorArchitecture);
    std::wcout<<processorArchName<<L'\n'
    <<sysInfo.dwNumberOfProcessors<<L'\n';
    // std::wcout<<sysInfo.wProcessorRevision<<L'\n'
    // <<HIWORD(sysInfo.wProcessorRevision)<<'.'
    // <<LOWORD(sysInfo.wProcessorRevision)<<L'\n';
    int processorDetails[4];
    //std::vector<int[4]> processorBrand(3);
    char cpuMaker[13];
    __cpuid(processorDetails,0);
    memcpy(cpuMaker,&processorDetails[1],4);
    memcpy(cpuMaker+4,&processorDetails[3],4);
    memcpy(cpuMaker+8,&processorDetails[2],4);
    cpuMaker[12] = '\0';std::wcout<<cpuMaker<<L'\n';

    std::array<char,49>cpuStr;
    unsigned charIndex{0};
    for(unsigned c{0x80000002};c<0x80000005;c++){
        __cpuid(processorDetails,c);
        memcpy(cpuStr.data()+charIndex,&processorDetails,16);
        charIndex+=16;    
    }
    std::array<char,49>cpuModel;
    unsigned brandIndex{0};
    for(unsigned c{0};c<48;c++){
        if(cpuStr[c]!='\0'&&cpuStr[c]!=' '){
            cpuModel[brandIndex] = cpuStr[c];
            brandIndex++;
            for (unsigned d{c+1};d<48;d++){
                if (cpuStr[d]!='\0'){
                    cpuModel[brandIndex] = cpuStr[d];
                    brandIndex++;
                }else break;
            }
            break;
        }
    }
    std::wcout<<cpuModel.data()<<L'\n';
    
    SYSTEM_LOGICAL_PROCESSOR_INFORMATION processorInfo;
    DWORD piBufferSize;
    GetLogicalProcessorInformation(&processorInfo,&piBufferSize);
    //get memory info


    //std::cout.flush();
    std::wcout.flush();
    pause();
    return 0;
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

void pause(){
    std::wstring temp;
    std::wcout<<L"Pause... ";
    std::getline(std::wcin, temp);
}