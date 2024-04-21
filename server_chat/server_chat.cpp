#include <iostream>
#include <winsock2.h>
#include <vector>
#include <map>
#include <thread>
#include <algorithm>
#include <Windows.h>
#include <ctime>
#include <iomanip>
#include "Message.h"

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

void SendPreviousMessagesToClient(ClientInfo* client, std::string chosenRoom) {
    std::string roomName = chosenRoom;
    std::string filename = roomName + "_messages.txt";
    std::vector<Message> messages = Message::loadFromFile(filename);
    for (size_t i = 0; i < messages.size(); ++i) {
        const auto& message = messages[i];
        std::string messageFromFile = "[" + message.getTimestamp() + "] " + message.getSender() + ": " + message.getContent();
        if (i != messages.size() - 1) {
            messageFromFile += "\n";
        }
        send(client->socket, messageFromFile.c_str(), messageFromFile.length(), 0);
    }
}


std::string getTimeserv() {
    std::time_t now = std::time(nullptr);
    char buffer[80];
    std::tm timeinfo;
    localtime_s(&timeinfo, &now);
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

void ReceiveAndSendMessages(ClientInfo* client, std::map<std::string, std::vector<ClientInfo>>& rooms, std::map<std::string, std::vector<std::string>>& roomMessages) {
    char recvBuf[DEFAULT_BUFLEN];
    int recvResult;

    send(client->socket, "Введите ваше имя: ", 20, 0);
    recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении имени клиента\n";
        closesocket(client->socket);
        delete client;
        return;
    }
    recvBuf[recvResult] = '\0';
    client->name = recvBuf;

    std::string roomList = "Список комнат:\n";
    for (const auto& room : rooms) {
        roomList += room.first + "\n";
    }
    send(client->socket, roomList.c_str(), roomList.length(), 0);

    send(client->socket, "Введите номер комнаты: ", 30, 0);
    recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении номера комнаты от клиента\n";
        closesocket(client->socket);
        delete client;
        return;
    }
    recvBuf[recvResult] = '\0';
    std::string chosenRoom = recvBuf;

    if (rooms.find(chosenRoom) != rooms.end()) {
        client->room = chosenRoom;
        rooms[chosenRoom].push_back(*client);
        std::cout << "Пользователь '" << client->name << "' подключен к комнате '" << chosenRoom << "'\n";
        std::string joinMessage = "Пользователь '" + client->name + "' присоединился к комнате";
        for (auto& roomClient : rooms[chosenRoom]) {
            if (roomClient.socket != client->socket) {
                send(roomClient.socket, joinMessage.c_str(), joinMessage.length(), 0);
            }
        }
        std::string msgroom = "=================== " + chosenRoom + " ===================";
        send(client->socket, msgroom.c_str(), msgroom.length(), 0);
        SendPreviousMessagesToClient(client, chosenRoom);
    }
    else {
        std::cerr << "Комната с номером '" << chosenRoom << "' не найдена\n";
        closesocket(client->socket);
        delete client;
        return;
    }

    while (true) {
        recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
        if (recvResult > 0) {
            recvBuf[recvResult] = '\0';

            std::cout << "Сообщение от клиента '" << client->name << "' в комнате '" << client->room << "': " << recvBuf << std::endl;

            Message newMessage(client->name, recvBuf, getTimeserv());
            std::string filename = client->room + "_messages.txt";
            newMessage.saveToFile(filename);
            roomMessages[client->room].push_back(recvBuf);
            for (auto& roomClient : rooms[client->room]) {
                std::string messageWithSender = "[" + getTimeserv() + "] " + client->name + ": " + recvBuf;
                send(roomClient.socket, messageWithSender.c_str(), messageWithSender.length() + 30, 0);
            }
        }
        else if (recvResult == 0) {
            std::cerr << "Соединение закрыто клиентом\n";
            for (auto& room : rooms) {
                auto& clients = room.second;
                clients.erase(std::remove_if(clients.begin(), clients.end(),
                    [client](const ClientInfo& c) { return c.socket == client->socket; }), clients.end());
            }
            closesocket(client->socket);
            delete client;
            break;
        }
        else {
            std::cerr << "Ошибка при получении данных от клиента\n";
            break;
        }
    }
}


int main() {
    SetConsoleCP(1251);
    setlocale(LC_ALL, "rus");
    InitializeWinsock();

    SOCKET serverSocket = CreateServerSocket(12345);

    std::cout << "Ждем подключения...\n";

    std::map<std::string, std::vector<ClientInfo>> rooms;
    std::map<std::string, std::vector<std::string>> roomMessages; // Для хранения сообщений комнаты

    rooms["room1"] = std::vector<ClientInfo>();
    rooms["room2"] = std::vector<ClientInfo>();
    rooms["room3"] = std::vector<ClientInfo>();

    while (true) {
        SOCKET clientSocket = AcceptClientConnection(serverSocket);
        ClientInfo* newClient = new ClientInfo{ clientSocket, "", "Hub" }; // Выделяем память для нового клиента

        std::thread receiveThread(ReceiveAndSendMessages, newClient, std::ref(rooms), std::ref(roomMessages));
        receiveThread.detach(); // Позволяет потоку работать асинхронно
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
