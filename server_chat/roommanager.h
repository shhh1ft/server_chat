#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Message.h"
#include "room.h"
class RoomManager {
public:
    RoomManager() {
        rooms.emplace_back("Hub");
        rooms.emplace_back("Room 1");
        rooms.emplace_back("Room 2");
        rooms.emplace_back("Room 3");
    }

    void addMemberToRoom(const std::string& roomName, const std::string& member) {
        Room* room = findRoomByName(roomName);
        if (room) {
            room->addMember(member);
        }
    }

    void addMessageToRoom(const std::string& roomName, const Message& message) {
        Room* room = findRoomByName(roomName);
        if (room) {
            room->addMessage(message);
        }
    }

    std::vector<std::string> getMessagesFromRoom(const std::string& roomName) const {
        const Room* room = findRoomByName(roomName);
        std::vector<std::string> roomMessages;
        if (room) {
            for (const auto& message : room->getMessages()) {
                roomMessages.push_back("[" + message.getTimestamp() + "] " + message.getSender() + ": " + message.getContent());
            }
        }
        return roomMessages;
    }

    void removeMemberFromRoom(const std::string& roomName, const std::string& member) {
        Room* room = findRoomByName(roomName);
        if (room) {
            room->removeMember(member);
        }
    }

    const std::vector<Room>& getRooms() const {
        return rooms;
    }

private:
    std::vector<Room> rooms;

    Room* findRoomByName(const std::string& name) {
        for (auto& room : rooms) {
            if (room.getName() == name) {
                return &room;
            }
        }
        return nullptr;
    }

    const Room* findRoomByName(const std::string& name) const {
        for (const auto& room : rooms) {
            if (room.getName() == name) {
                return &room;
            }
        }
        return nullptr;
    }
};
