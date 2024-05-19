#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Message.h"

class Room{
public:
    Room(const std::string& name) : name(name) { }

    void addMember(const std::string & member) {
        members.push_back(member);
    }

    void addMessage(const Message & message) {
        messages.push_back(message);
    }

    const std::string & getName() const {
        return name;
    }

    const std::vector<std::string>&getMembers() const {
        return members;
    }

    const std::vector<Message>&getMessages() const {
        return messages;
    }

    bool findMemberByName(const std::string & memberName) const {
        return std::find(members.begin(), members.end(), memberName) != members.end();
    }

    void removeMember(const std::string& member) {
        members.erase(std::remove(members.begin(), members.end(), member), members.end());
    }


private:
    std::string name;
    std::vector<std::string> members;
    std::vector<Message> messages;
};

