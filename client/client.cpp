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

int main() {
    SetConsoleCP(1251);
    setlocale(LC_ALL, "rus");
    InitializeWinsock();

    const char* ipAddress = "26.162.182.51";
    int port = 12345;

    SOCKET clientSocket = ConnectToServer(ipAddress, port);
    std::cout << "Подключение к серверу успешно\n";

    char recvBuf[BUFLEN];
    std::vector<std::string> messageHistory;

    int recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    std::string name;
    std::getline(std::cin, name);
    SendToServer(clientSocket, name.c_str());

    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    std::string roomNumber;
    std::getline(std::cin, roomNumber);
    SendToServer(clientSocket, roomNumber.c_str());

    std::thread sendThread(SendMessageThread, clientSocket);
    sendThread.detach();
    system("cls");
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
