#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _CRTDBG_MAP_ALLOC

#include <iostream>
#include <string>
#include <fstream>
#include <functional>
#include <vector>
#include <thread>
#include <unordered_map>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")  

#include <stdlib.h>
#include <crtdbg.h>

#include <brotli/encode.h>
#include <brotli/decode.h>

#include <openssl/sha.h>

#include <qrencode.h>

#include <nlohmann/json.hpp>

using nlohmann::json;

#define DEFAULT_BUFFER_SIZE 4096

#ifdef _DEBUG
#define LOG(x) std::cout << x << '\n';
#else
#define LOG(x)
#endif

void EnableConsole();

std::string sha1(const std::string& input);


