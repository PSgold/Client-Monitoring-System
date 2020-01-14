#pragma once

#include <iostream>
#include <fstream>
#include <iomanip>
#include <memory>
#include <string>
#include <cstring>
#include <vector>
#include <array>
#include <thread>
#include "Windows.h"
#include "WinSock2.h"
#include "ws2tcpip.h"
#include "VersionHelpers.h"
#include "intrin.h"
#include "ShaiG.h"

int startWork(ShaiG::logFile& logFile);