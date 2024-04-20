#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>
#include <Windows.h>
#include <cstdlib>
#include <vector>

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
    std::string ipAddress = "26.255.228.69";
    int port = 12345;
    std::string choice;
    SOCKET clientSocket;

    do {
        std::cout << "Выберите операцию:\n 1. Подключиться \n 2. Кастомный ip \n 3. Кастомный порт \n q. Выход.\n Выбор: ";
        std::getline(std::cin, choice);

        if (choice == "1") {
            break;
        }
        else if (choice == "2") {
            std::cout << "Введите IP адрес сервера: ";
            std::getline(std::cin, ipAddress);
        }
        else if (choice == "3") {
            std::string str_port;
            std::cout << "Введите Порт сервера: ";
            std::getline(std::cin, str_port);
            port = std::stoi(str_port);
        }
        else if (choice == "q") {
            return 0;
        }
        else {
            std::cout << "Неправильный выбор \n";
        }
    } while (choice != "1");

    clientSocket = ConnectToServer(ipAddress.c_str(), port);
    std::cout << "Подключение к серверу успешно\n";

    char recvBuf[BUFLEN];
    std::vector<std::string> messageHistory;

    int recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    InputName(clientSocket);

    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    InputRoom(clientSocket);

    // Ожидание получения сообщений от сервера перед запуском потока отправки сообщений
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

    // После получения и отображения предыдущих сообщений, клиент продолжает ожидать и отображать новые сообщения
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
