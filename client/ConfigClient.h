#ifndef NETWORK_INFO_H
#define NETWORK_INFO_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#endif

class ConfigClient {
private:
    std::string filename;

public:
    ConfigClient(const std::string& filename) : filename(filename) {}

    void writeInfo(const std::string& ipAddress, int port) {
        std::ofstream outFile(filename, std::ios::trunc);
        if (outFile.is_open()) {
            std::string macAddress = getMacAddress();
            outFile << ipAddress << "," << macAddress << "," << port << std::endl;
            outFile.close();
            std::cout << "Конфиг записан " << filename << std::endl;
        }
        else {
            std::cerr << "Ошибка открытия файла  " << filename << std::endl;
        }
    }


    void printInfo(const std::string& ipAddress, int port) {
        std::cout << "IP Адрес: " << ipAddress
            << ", Порт: " << port << "\n\n";
    }

    bool isFileEmpty() {
        std::ifstream file(filename);
        return file.peek() == std::ifstream::traits_type::eof();
    }

    void readInfo(std::string& ipAddress, std::string& macAddress, int& port) {
        std::ifstream inFile(filename);
        if (inFile.is_open()) {
            std::string line;
            if (std::getline(inFile, line)) {
                std::istringstream iss(line);
                if (std::getline(iss, ipAddress, ',') && std::getline(iss, macAddress, ',') &&
                    (iss >> port)) {

                }
                else {
                    std::cerr << "Ошибка: " << line << std::endl;
                }
            }
            inFile.close();
        }
        else {
            std::cerr << ": " << filename << std::endl;
        }
    }


    std::string getMacAddress() {
        std::string macAddress;
#ifdef _WIN32
        IP_ADAPTER_INFO AdapterInfo[16];
        DWORD dwBufLen = sizeof(AdapterInfo);

        DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
        if (dwStatus == ERROR_SUCCESS) {
            PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
            macAddress = convertMacAddress(pAdapterInfo->Address, pAdapterInfo->AddressLength);
        }
        else {
            std::cerr << "Ошиька не можем получить MAC адрес." << std::endl;
        }
#else
        std::cerr << "Error: Unsupported operating system." << std::endl;
#endif
        return macAddress;
        }

private:
    std::string convertMacAddress(const BYTE* macBytes, ULONG macLength) {
        std::string macAddress;
        char hex[3];
        for (ULONG i = 0; i < macLength; ++i) {
            snprintf(hex, sizeof(hex), "%02X", macBytes[i]);
            macAddress += hex;
            if (i != macLength - 1) {
                macAddress += ":";
            }
        }
        return macAddress;
    }
    };

#endif
