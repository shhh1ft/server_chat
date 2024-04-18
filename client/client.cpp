#include <iostream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

#define BUFLEN 512

// Функция для инициализации Winsock
void InitializeWinsock() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) {
        std::cerr << "Ошибка при инициализации Winsock\n";
        exit(EXIT_FAILURE);
    }
}

// Функция для создания сокета и подключения к серверу
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

// Функция для получения данных от сервера
int ReceiveFromServer(SOCKET clientSocket, char* recvBuf) {
    int recvResult = recv(clientSocket, recvBuf, BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении данных от сервера\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
    recvBuf[recvResult] = '\0'; // Добавляем завершающий нулевой символ

    return recvResult;
}

// Функция для отправки данных серверу
void SendToServer(SOCKET clientSocket, const char* data) {
    int sendResult = send(clientSocket, data, strlen(data), 0);
    if (sendResult == SOCKET_ERROR) {
        std::cerr << "Ошибка при отправке данных серверу\n";
        closesocket(clientSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

int main() {
    setlocale(LC_ALL, "rus");
    InitializeWinsock();

    const char* ipAddress = "26.255.228.69";
    int port = 12345;

    SOCKET clientSocket = ConnectToServer(ipAddress, port);
    std::cout << "Подключение к серверу успешно\n";

    char recvBuf[BUFLEN];

    // Получение запроса на ввод имени от сервера
    int recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf; // Выводим запрос на экран

    // Ввод имени и отправка на сервер
    std::string name;
    std::getline(std::cin, name);
    SendToServer(clientSocket, name.c_str());

    // Получение списка комнат от сервера
    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    // Получение запроса на ввод номера комнаты от сервера
    recvResult = ReceiveFromServer(clientSocket, recvBuf);
    std::cout << recvBuf;

    // Ввод номера комнаты и отправка на сервер
    std::string roomNumber;
    std::getline(std::cin, roomNumber);
    SendToServer(clientSocket, roomNumber.c_str());

    // Основной цикл для отправки сообщений
    while (true) {
        // Получение данных от сервера
        recvResult = ReceiveFromServer(clientSocket, recvBuf);
        std::cout << recvBuf << std::endl; // Выводим сообщение на экран

        std::cout << "Введите сообщение: ";
        std::string message;
        std::getline(std::cin, message);
        SendToServer(clientSocket, message.c_str());
    }

    closesocket(clientSocket);
    WSACleanup();

    return 0;
}
