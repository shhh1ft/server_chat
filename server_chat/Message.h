#pragma once
#include <string>
#include <vector>

class Message
{
public:
    Message() {}

    Message(const std::string& sender, const std::string& content, const std::string& timestamp)
        : sender(sender), content(content), timestamp(timestamp) {}

    std::string getSender() const {
        return sender;
    }

    std::string getContent() const {
        return content;
    }

    std::string getTimestamp() const {
        return timestamp;
    }

private:
    std::string sender;
    std::string content;
    std::string timestamp;
};

