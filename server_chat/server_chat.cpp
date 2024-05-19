#include <iostream>
#include <winsock2.h>
#include <vector>
#include <map>
#include <thread>
#include <algorithm>
#include <Windows.h>
#include <ctime>
#include <iomanip>
#include <regex>
#include "Message.h"
#include "Profiles.h"
#include "profilemanager.h"
#include "room.h"
#include "roommanager.h"

#pragma comment(lib, "ws2_32.lib")

#define BUFLEN 4096

ProfileManager manager;
RoomManager roomManager;

struct ClientInfo {
    SOCKET socket;
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

    std::cout << "Сервер успешно запущен\n";

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

void SendPreviousMessagesToClient(ClientInfo* client, const std::string& room) {
    std::vector<std::string> messages = roomManager.getMessagesFromRoom(room);
    for (size_t i = 0; i < messages.size(); ++i) {
        std::string message = messages[i];
        if (i != messages.size() - 1) {
            message += "\n";
        }
        send(client->socket, message.c_str(), message.length(), 0);
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

bool isValidNickname(const std::string& nickname) {
    std::regex pattern("^[a-zA-Z0-9_-]+$");
    return std::regex_match(nickname, pattern);
}

void joinToRoom(ClientInfo* client, std::map<std::string, std::vector<ClientInfo>>& rooms) {
    std::string joinMessage = "Пользователь '" + manager.getProfileName(client->macAddress) + "' присоединился к комнате '" + client->room + "'";
    for (auto& roomClient : rooms[client->room]) {
        if (roomClient.socket != client->socket) {
            send(roomClient.socket, joinMessage.c_str(), joinMessage.length(), 0);
        }
    }
    SendPreviousMessagesToClient(client, client->room);
}

void SendMembers(ClientInfo* client, const std::string& room) {
    const auto& rooms = roomManager.getRooms();
    for (const auto& room : rooms) {
        std::string msg = "Комнаты: " + room.getName() + "\n" + "Участники:\n";
        send(client->socket, msg.c_str(), msg.length(), 0);
        for (const auto& member : room.getMembers()) {
            msg = " - " + member + "\n";
            send(client->socket, msg.c_str(), msg.length(), 0);
        }
    }
}

void ReceiveAndSendMessages(ClientInfo* client, std::map<std::string, std::vector<ClientInfo>>& rooms) {
    char recvBuf[BUFLEN];
    int recvResult;

    recvResult = recv(client->socket, recvBuf, BUFLEN, 0);
    if (recvResult <= 0) {
        std::cerr << "Ошибка при получении MAC адреса клиента\n";
        closesocket(client->socket);
        delete client;
        return;
    }

    recvBuf[recvResult] = '\0';
    client->macAddress = recvBuf;

    manager.addProfile(client->macAddress, " - ");
    client->room = "Hub";
    rooms[client->room].push_back(*client);
    roomManager.addMemberToRoom(client->room, manager.getProfileName(client->macAddress));
    std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " присоединился к серверу. MAC адрес: " << client->macAddress << ", Сокет: " << client->socket << ", Комната : " << client->room << '\n';
    while (true) {
        recvResult = recv(client->socket, recvBuf, BUFLEN, 0);
        if (recvResult > 0) {
            recvBuf[recvResult] = '\0';
            if (strcmp(recvBuf, "/hub") == 0) {
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " был в комнате " << client->room << '\n';
                roomManager.removeMemberFromRoom(client->room, manager.getProfileName(client->macAddress));
                client->room = "Hub";
                rooms[client->room].push_back(*client);
                roomManager.addMemberToRoom(client->room, manager.getProfileName(client->macAddress));
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " вышел в " << client->room << '\n';
            }
            else if (strcmp(recvBuf, "/disconnect") == 0) {
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " отключился" << '\n';
                std::string disconnectMsg = "Вы отключены от чата.";
                send(client->socket, disconnectMsg.c_str(), disconnectMsg.length(), 0);
                closesocket(client->socket);
                rooms[client->room].erase(std::remove_if(rooms[client->room].begin(), rooms[client->room].end(),
                    [client](const ClientInfo& c) { return c.socket == client->socket; }), rooms[client->room].end());
                delete client;
                return;
            }
            else if (strcmp(recvBuf, "/ROOM1") == 0) {
                roomManager.removeMemberFromRoom(client->room, manager.getProfileName(client->macAddress));
                client->room = "Room 1";
                rooms[client->room].push_back(*client);
                roomManager.addMemberToRoom(client->room, manager.getProfileName(client->macAddress));
                joinToRoom(client, rooms);
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " вошел в комнату " << client->room << '\n';
            }
            else if (strcmp(recvBuf, "/ROOM2") == 0) {
                roomManager.removeMemberFromRoom(client->room, manager.getProfileName(client->macAddress));
                client->room = "Room 2";
                rooms[client->room].push_back(*client);
                roomManager.addMemberToRoom(client->room, manager.getProfileName(client->macAddress));
                joinToRoom(client, rooms);
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " вошел в комнату " << client->room << '\n';
            }
            else if (strcmp(recvBuf, "/ROOM3") == 0) {
                roomManager.removeMemberFromRoom(client->room, manager.getProfileName(client->macAddress));
                client->room = "Room 3";
                rooms[client->room].push_back(*client);
                roomManager.addMemberToRoom(client->room, manager.getProfileName(client->macAddress));
                joinToRoom(client, rooms);
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " вошел в комнату " << client->room << '\n';
            }
            else if (strcmp(recvBuf, "/PRINTONLINE") == 0) {
                SendMembers(client, client->room);
            }
            else if (strcmp(recvBuf, "/PRINTPROFILE") == 0) {
                std::string msg;
                msg = "  ";
                send(client->socket, msg.c_str(), msg.length(), 0);
                msg = manager.showProfile(client->macAddress);
                send(client->socket, msg.c_str(), msg.length(), 0);
                std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " посмотрел профиль " << '\n';
            }
            else if (strcmp(recvBuf, "/CHANGENAME") == 0) {
                std::string newNick;
                recvResult = recv(client->socket, recvBuf, BUFLEN, 0);
                recvBuf[recvResult] = '\0';
                newNick = recvBuf;
                if (isValidNickname(newNick)) {
                    manager.changeProfileName(client->macAddress, newNick);
                    std::cout << "Пользователь " << manager.getProfileName(client->macAddress) << " изменил имя на " << newNick << '\n';
                }
                else {
                    std::string invalidNameMsg = "Некорректное имя пользователя.";
                    send(client->socket, invalidNameMsg.c_str(), invalidNameMsg.length(), 0);
                }
            }
            else {
                std::cout << "Сообщение от клиента '" << manager.getProfileName(client->macAddress) << "' в комнате '" << client->room << "': " << recvBuf << std::endl;
                roomManager.addMessageToRoom(client->room, Message(manager.getProfileName(client->macAddress), recvBuf, getTimeserv()));
                std::vector<std::string> messages = roomManager.getMessagesFromRoom(client->room);
                std::string messagelast = messages.back();
                manager.changeLasActivity(client->macAddress, getTimeserv());
                for (auto& roomClient : rooms[client->room]) {
                    send(roomClient.socket, messagelast.c_str(), messagelast.length(), 0);
                }
            }
        }
        else if (recvResult == 0) {
            std::cerr << "Соединение закрыто клиентом\n";
            rooms[client->room].erase(std::remove_if(rooms[client->room].begin(), rooms[client->room].end(),
                [client](const ClientInfo& c) { return c.socket == client->socket; }), rooms[client->room].end());
            roomManager.removeMemberFromRoom(client->room, manager.getProfileName(client->macAddress));
            closesocket(client->socket);
            delete client;
            break;
        }
        else {
            roomManager.removeMemberFromRoom(client->room, manager.getProfileName(client->macAddress));
            std::cerr << "Клиент " << manager.getProfileName(client->macAddress) << " отключился" << '\n';
            break;
        }
    }
}
int main() {
    SetConsoleCP(1251);
    setlocale(LC_ALL, "rus");
    InitializeWinsock();

    SOCKET serverSocket = CreateServerSocket(12345);

    std::map<std::string, std::vector<ClientInfo>> rooms;
    const auto& roomcls = roomManager.getRooms();
    for (const auto& room : roomcls) {
        rooms[room.getName()] = std::vector<ClientInfo>();
    }
    while (true) {
        SOCKET clientSocket = AcceptClientConnection(serverSocket);
        ClientInfo* newClient = new ClientInfo{ clientSocket, "Hub" };
        std::thread receiveThread(ReceiveAndSendMessages, newClient, std::ref(rooms));
        receiveThread.detach();
    }

    closesocket(serverSocket);
    WSACleanup();

    return 0;
}
