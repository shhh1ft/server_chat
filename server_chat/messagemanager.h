#pragma once
#include <string>
#include <vector>
#include "Message.h"
class MessageManager
{
public:
    void addMessage(const std::string& macAddr, const std::string& content, const std::string& timestamp) {
        messages.emplace_back(macAddr, content, timestamp);
    }

    const std::vector<Message>& getMessages() const {
        return messages;
    }

private:
    std::vector<Message> messages;
};