#include <iostream>
#include <winsock2.h>
#include <vector>
#include <map>
#include <thread>
#include <algorithm>

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512

struct ClientInfo {
    SOCKET socket;
    std::string name;
    std::string room;
};

void InitializeWinsock() {
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);
    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) {
        std::cerr << "Ошибка при инициализации Winsock\n";
        exit(EXIT_FAILURE);
    }
}

SOCKET CreateServerSocket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка при создании сокета\n";
        exit(EXIT_FAILURE);
    }

    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (sockaddr*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << "Ошибка при привязке адреса\n";
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Ошибка при прослушивании порта\n";
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    return serverSocket;
}

SOCKET AcceptClientConnection(SOCKET serverSocket) {
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket = accept(serverSocket, (sockaddr*)&clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Ошибка при принятии соединения\n";
        closesocket(serverSocket);
        WSACleanup();
        exit(EXIT_FAILURE);
    }

    return clientSocket;
}

void ReceiveAndSendMessages(ClientInfo* client, std::map<std::string, std::vector<ClientInfo>>& rooms) {
    char recvBuf[DEFAULT_BUFLEN];
    int recvResult;

    // Отправляем запрос на ввод имени клиента
    send(client->socket, "Введите ваше имя: ", 20, 0);
    recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении имени клиента\n";
        closesocket(client->socket);
        delete client; // Освобождаем память
        return;
    }
    recvBuf[recvResult] = '\0';
    client->name = recvBuf;

    // Отправляем список комнат клиенту
    std::string roomList = "Список комнат:\n";
    for (const auto& room : rooms) {
        roomList += room.first + "\n";
    }
    send(client->socket, roomList.c_str(), roomList.length(), 0);

    // Получаем номер комнаты от клиента
    send(client->socket, "Введите номер комнаты: ", 30, 0);
    recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении номера комнаты от клиента\n";
        closesocket(client->socket);
        delete client; // Освобождаем память
        return;
    }
    recvBuf[recvResult] = '\0';
    std::string chosenRoom = recvBuf;

    // Проверяем, существует ли выбранная комната
    if (rooms.find(chosenRoom) != rooms.end()) {
        client->room = chosenRoom;
        rooms[chosenRoom].push_back(*client);
        std::cout << "Клиент '" << client->name << "' подключен к комнате '" << chosenRoom << "'\n";

        // Отправляем сообщение о начале чата
        send(client->socket, "Чат начался!\n", 14, 0);
    }
    else {
        std::cerr << "Комната с номером '" << chosenRoom << "' не найдена\n";
        closesocket(client->socket);
        delete client; // Освобождаем память
        return;
    }

    // Бесконечный цикл приема и отправки сообщений
    while (true) {
        recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
        if (recvResult > 0) {
            recvBuf[recvResult] = '\0';
            std::cout << "Сообщение от клиента '" << client->name << "' в комнате '" << client->room << "': " << recvBuf << std::endl;
            // Отправляем сообщение всем клиентам в этой комнате, кроме отправителя
            for (auto& roomClient : rooms[client->room]) {
                if (roomClient.socket != client->socket) {
                    send(roomClient.socket, (client->name + ": " + recvBuf).c_str(), recvResult + client->name.length() + 2, 0);
                }
            }
        }
        else if (recvResult == 0) {
            std::cerr << "Соединение закрыто клиентом\n";
            // Удаляем клиента из списка комнаты
            for (auto& room : rooms) {
                auto& clients = room.second;
                clients.erase(std::remove_if(clients.begin(), clients.end(),
                    [client](const ClientInfo& c) { return c.socket == client->socket; }), clients.end());
            }
            closesocket(client->socket);
            delete client; // Освобождаем память
            break;
        }
        else {
            std::cerr << "Ошибка при получении данных от клиента\n";
            break;
        }
    }
}

int main() {
    setlocale(LC_ALL, "rus");
    InitializeWinsock();

    SOCKET serverSocket = CreateServerSocket(12345);

    std::cout << "Ждем подключения...\n";

    std::map<std::string, std::vector<ClientInfo>> rooms;
    rooms["room1"] = std::vector<ClientInfo>();
    rooms["room2"] = std::vector<ClientInfo>();
    rooms["room3"] = std::vector<ClientInfo>();

    while (true) {
        SOCKET clientSocket = AcceptClientConnection(serverSocket);
        ClientInfo* newClient = new ClientInfo{ clientSocket, "", "Hub" }; // Выделяем память для нового клиента

        std::thread receiveThread(ReceiveAndSendMessages, newClient, std::ref(rooms));
        receiveThread.detach(); // Позволяет потоку работать асинхронно
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
