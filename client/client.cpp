#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <Windows.h>
#include <cstdlib>
#include <fstream>
#include <regex>
#include <vector>
#include "ConfigClient.h"

#pragma comment(lib, "ws2_32.lib")

#define BUFLEN 512

void InitializeWinsock() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) {
        std::cerr << "Ошибка при инициализации Winsock\n";
        exit(EXIT_FAILURE);
    }
}

SOCKET ConnectToServer(const char* ipAddress, int port) {
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка при создании сокета\n";
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);

    if (inet_pton(AF_INET, ipAddress, &serverAddress.sin_addr) <= 0) {
        std::cerr << "Некорректный IP-адрес\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (connect(clientSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Ошибка при подключении к серверу\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    return clientSocket;
}

int ReceiveFromServer(SOCKET clientSocket, char* recvBuf) {
    int recvResult = recv(clientSocket, recvBuf, BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении данных от сервера\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    recvBuf[recvResult] = '\0';

    return recvResult;
}

void SendToServer(SOCKET clientSocket, const char* data) {
    int sendResult = send(clientSocket, data, strlen(data), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Ошибка при отправке данных серверу\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

void SendMessageThread(SOCKET clientSocket) {
    while (true) {
        std::string message;
        std::getline(std::cin, message);
        SendToServer(clientSocket, message.c_str());

    }
}

void InputName(SOCKET clientSocket) {
    std::string name;
    std::getline(std::cin, name);
    SendToServer(clientSocket, name.c_str());
}

void InputRoom(SOCKET clientSocket) {
    std::string roomNumber;
    std::getline(std::cin, roomNumber);
    SendToServer(clientSocket, roomNumber.c_str());
}

int main() {
    SetConsoleCP(1251);
    setlocale(LC_ALL, "rus");
    InitializeWinsock();
    std::string filenameConfig = "client_info.txt";
    ConfigClient networkInfo(filenameConfig);
    std::ifstream file(filenameConfig);

    bool fileEmpty = !file || file.peek() == std::ifstream::traits_type::eof();
    file.close();

    if (fileEmpty) {

        networkInfo.writeInfo("127.0.0.1", 12345);
    }


    std::string ipAddress;
    int port;
    std::string macAddress;

    networkInfo.readInfo(ipAddress, macAddress, port);

    std::string choice;
    SOCKET clientSocket;;

    do {
        std::cout << "Выберите операцию:\n"
                     " 1. Подключиться\n"
                     " 2. Ввести кастомный IP\n"
                     " 3. Ввести кастомный порт\n"
                     " 4. Вывести мой конфиг подключения\n"
                     " 5. Изменить мой конфиг\n"
                     " q. Выход.\n"
                     "Выбор: ";
        std::getline(std::cin, choice);

        switch (choice[0]) {
            case '1': {
                break;
            }
            case '2': {
                system("cls");
                std::cout << "Введите IP адрес сервера: ";
                std::getline(std::cin, ipAddress);

                // Проверка правильности формата IP-адреса
                std::regex ip_regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
                if (!std::regex_match(ipAddress, ip_regex)) {
                    std::cout << "Неправильный формат IP-адреса\n";
                    break;
                }
                break;
            }
            case '3': {
                system("cls");
                std::string str_port;
                std::cout << "Введите порт сервера: ";
                std::getline(std::cin, str_port);

                // Проверка правильности формата порта
                try {
                    port = std::stoi(str_port);
                    if (port < 1 || port > 65535) {
                        throw std::invalid_argument("Недопустимый порт");
                    }
                }
                catch (std::invalid_argument& e) {
                    std::cout << "Неправильный формат порта: " << e.what() << std::endl;
                    break;
                }
                break;
            }
            case '4': {
                system("cls");
                networkInfo.printInfo(ipAddress, port);
                break;
            }
            case '5': {
                system("cls");
                std::string Input;
                std::cout << "Введите IP адрес сервера (enter - оставить прошлое значение " << ipAddress << "): ";
                std::getline(std::cin, Input);
                if (Input != "" && Input != "0") {
                    std::regex ip_regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
                    if (!std::regex_match(Input, ip_regex)) {
                        std::cout << "Неправильный формат IP-адреса\n";
                        break;
                    }
                    ipAddress = Input;
                }
                Input = "";
                std::cout << "Введите порт сервера (enter - оставить прошлое значение " << port << "): ";
                std::getline(std::cin, Input);
                if (Input != "" && Input != "0") {

                    try {
                        int newPort = std::stoi(Input);
                        if (newPort < 1 || newPort > 65535) {
                            throw std::invalid_argument("Недопустимый порт");
                        }
                        port = newPort;
                    }
                    catch (std::invalid_argument& e) {
                        std::cout << "Неправильный формат порта: " << e.what() << std::endl;
                        break;
                    }
                }
                Input = "";
                ConfigClient networkInfo(filenameConfig);
                networkInfo.writeInfo(ipAddress, port);
                break;
            }

            case 'q': {
                return 0;
            }
            default: {
                std::cout << "Неправильный выбор\n";
                break;
            }
        }
    } while (choice[0] != '1');

    clientSocket = ConnectToServer(ipAddress.c_str(), port);
    std::cout << "Подключение к серверу успешно\n";
    SendToServer(clientSocket, macAddress.c_str());
    char recvBuf[BUFLEN];
    std::vector<std::string> messageHistory;

    int recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    InputRoom(clientSocket);

    
    while (true) {
        recvResult = ReceiveFromServer(clientSocket, recvBuf);
        messageHistory.push_back(recvBuf);
        system("cls");
        for (int i = 0; i < messageHistory.size(); ++i) {
            std::cout << messageHistory[i] << std::endl;
        }

        if (messageHistory.size() > 0) {
            std::thread sendThread(SendMessageThread, clientSocket);
            sendThread.detach();
            break;
        }
    }

    
    while (true) {
        recvResult = ReceiveFromServer(clientSocket, recvBuf);
        messageHistory.push_back(recvBuf);
        system("cls");
        for (int i = 0; i < messageHistory.size(); ++i) {
            std::cout << messageHistory[i] << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}

