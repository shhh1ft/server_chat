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

#define BUFLEN 2048

std::vector<std::string> messageHistory;

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

void displayMenu2() {
    std::cout << "\nЧаты:\n"
        " 1. Комната 1\n"
        " 2. Комната 2\n"
        " 3. Комната 3\n"
        "\n"
        " =-= Настройки =-=\n"
        " o. Показать онлайн\n"
        " p. Посмотреть свой профиль\n"
        " c. Изменить имя\n"
        " q. Отключиться\n"
        "Выбор: ";
}

void MenuChoise2(SOCKET clientSocket) {
    std::string choice;
    while (true) {
        displayMenu2();
        std::getline(std::cin, choice);
        if (choice == "1") {
            messageHistory.clear();
            SendToServer(clientSocket, "/ROOM1");
            system("cls");
            break;
        }
        else if (choice == "2") {
            messageHistory.clear();
            SendToServer(clientSocket, "/ROOM2");
            system("cls");
            break;
        }
        else if (choice == "3") {
            messageHistory.clear();
            SendToServer(clientSocket, "/ROOM3");
            system("cls");
            break;
        }
        else if (choice == "p") {
            SendToServer(clientSocket, "/PRINTPROFILE");
            messageHistory.clear();
            system("cls");
            Sleep(150);
        }
        else if (choice == "o") {
            SendToServer(clientSocket, "/PRINTONLINE");
            messageHistory.clear();
            system("cls");
            Sleep(150);
        }
        else if (choice == "c") {
            SendToServer(clientSocket, "/CHANGENAME");
            std::string name;
            std::cout << "Введите новый ник: " << std::endl;
            std::getline(std::cin, name);
            SendToServer(clientSocket, name.c_str());
            system("cls");
        }
        else if (choice == "q") {
            exit(0);
        }
    }
}

void SendMessageThread(SOCKET clientSocket) {
    SendToServer(clientSocket, "/hub");
    system("cls");
    MenuChoise2(clientSocket);
    messageHistory.clear();
    while (true) {
        std::string message;
        std::getline(std::cin, message);
        if (message == "/hub") {
            SendToServer(clientSocket, message.c_str());
            system("cls");
            Sleep(40);
            MenuChoise2(clientSocket);
            messageHistory.clear();
        }
        else {
            SendToServer(clientSocket, message.c_str());
        }
    }
}

void displayMenu1() {
    std::cout << "^^^^^^^^^^^^^^^^^Клиент локального чата v0.7^^^^^^^^^^^^^^^^^\n"
        "Выберите операцию:\n"
        " 1. Подключиться\n"
        " 2. Ввести кастомный IP\n"
        " 3. Ввести кастомный порт\n"
        " 4. Вывести мой конфиг подключения\n"
        " 5. Изменить мой конфиг\n"
        " q. Выход.\n"
        "Выбор: ";
}

void MenuChoice1(std::string choice, std::string& ipAddress, int& port, const std::string& filenameConfig, ConfigClient& networkInfo) {
    switch (choice[0]) {
    case '1': {
        break;
    }
    case '2': {
        system("cls");
        std::cout << "Введите IP адрес сервера: ";
        std::string input;
        std::getline(std::cin, input);
        std::regex ip_regex("^\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}$");
        if (!std::regex_match(input, ip_regex)) {
            std::cout << "Неправильный формат IP-адреса\n";
            break;
        }
        else {
            ipAddress = input;
        }
        break;
    }
    case '3': {
        system("cls");
        std::string str_port;
        std::cout << "Введите порт сервера: ";
        std::getline(std::cin, str_port);

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
        exit(0);
    }
    default: {
        std::cout << "Неправильный выбор\n";
        system("cls");
        break;
    }
    }
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
    macAddress = networkInfo.getMacAddress();
    std::string choise;
    SOCKET clientSocket;
    do {
        displayMenu1();
        std::getline(std::cin, choise);
        MenuChoice1(choise, ipAddress, port, filenameConfig, networkInfo);
    } while (choise[0] != '1');
    system("cls");
    clientSocket = ConnectToServer(ipAddress.c_str(), port);

    SendToServer(clientSocket, macAddress.c_str());
    std::thread sendThread(SendMessageThread, clientSocket);
    sendThread.detach();
    char recvBuf[BUFLEN];
    int recvResult;

    while (true) {
        recvResult = ReceiveFromServer(clientSocket, recvBuf);
        messageHistory.push_back(recvBuf);
        system("cls");
        for (const auto& msg : messageHistory) {
            std::cout << msg << std::endl;
        }
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
