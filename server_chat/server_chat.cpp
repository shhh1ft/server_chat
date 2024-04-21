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
#include "Profiles.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512

struct ClientInfo {
    SOCKET socket;
    std::string name;
    std::string macAddress;
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

void ShowMenuAndSelectRoom(ClientInfo* client, std::map<std::string, std::vector<ClientInfo>>& rooms) {
    std::string msgMenu = "Чаты:\n"
        " 1. Комната 1\n"
        " 2. Комната 2\n"
        " 3. Комната 3\n"
        "\n"
        " =-= Настройки =-=\n"
        " p. Посмотреть свой профиль\n"
        " c. Изменить имя\n"
        "Выбор: ";
    send(client->socket, msgMenu.c_str(), msgMenu.length(), 0);

    char recvBuf[DEFAULT_BUFLEN];
    int recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении выбора комнаты от клиента\n";
        closesocket(client->socket);
        delete client;
        return;
    }
    recvBuf[recvResult] = '\0';
    std::string chosenOption = recvBuf;

    if (chosenOption == "1" || chosenOption == "2" || chosenOption == "3") {
        std::string roomName = "room" + chosenOption;
        client->room = roomName;
        rooms[roomName].push_back(*client);
        std::string joinMessage = "Пользователь '" + client->name + "' присоединился к комнате '" + roomName + "'";
        for (auto& roomClient : rooms[roomName]) {
            if (roomClient.socket != client->socket) {
                send(roomClient.socket, joinMessage.c_str(), joinMessage.length(), 0);
            }
        }
        std::string msgroom = "===================== " + roomName + " =====================\n";
        send(client->socket, msgroom.c_str(), msgroom.length(), 0);
        SendPreviousMessagesToClient(client, roomName);
    }
    else if(chosenOption == "c") {
        std::string msgnick = "Новый ник: ";
        send(client->socket, msgnick.c_str(), msgnick.length(), 0);
        MACHandler idHandler("profiles.txt");
        recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
        if (recvResult <= 0) {
            std::cerr << "Ошибка при получении выбора комнаты от клиента\n";
            closesocket(client->socket);
            delete client;
            return;
        }
        recvBuf[recvResult] = '\0';
        idHandler.changeNick(client->macAddress, recvBuf);
        ShowMenuAndSelectRoom(client, rooms);
    }
    else {
        send(client->socket, "Неверный выбор. Нажмите enter...", 30, 0);
        ShowMenuAndSelectRoom(client, rooms);
    }
}


void ReceiveAndSendMessages(ClientInfo* client, std::map<std::string, std::vector<ClientInfo>>& rooms, std::map<std::string, std::vector<std::string>>& roomMessages) {
    char recvBuf[DEFAULT_BUFLEN];
    int recvResult;
    recvResult = recv(client->socket, recvBuf, DEFAULT_BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении MAC адреса клиента\n";
        closesocket(client->socket);
        delete client;
        return;
    }
    recvBuf[recvResult] = '\0';

    client->macAddress = recvBuf;
    MACHandler idHandler("profiles.txt");
    idHandler.addID(client->macAddress);
    client->name = idHandler.getNameByMAC(client->macAddress);
    std::cout <<"Пользователь " << client->name << " присоединился к серверу. MAC адрес: " << client->macAddress << '\n';

    ShowMenuAndSelectRoom(client, rooms);


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

    std::cout << "Сервер успешно запущен\n";

    std::map<std::string, std::vector<ClientInfo>> rooms;
    std::map<std::string, std::vector<std::string>> roomMessages;

    rooms["room1"] = std::vector<ClientInfo>();
    rooms["room2"] = std::vector<ClientInfo>();
    rooms["room3"] = std::vector<ClientInfo>();

    while (true) {
        SOCKET clientSocket = AcceptClientConnection(serverSocket);
        ClientInfo* newClient = new ClientInfo{ clientSocket, "", "Hub" };
        std::thread receiveThread(ReceiveAndSendMessages, newClient, std::ref(rooms), std::ref(roomMessages));
        receiveThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
