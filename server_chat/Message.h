#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <iostream>

class Message
{
public:
    Message() {}

    Message(const std::string& sender, const std::string& content) : sender(sender), content(content) {
        time_t now;
        time(&now);
        localtime_s(&timestamp, &now);
    }

    std::string getSender() const {
        return sender;
    }

    std::string getContent() const {
        return content;
    }

    std::string getTimestamp() const {
        char buffer[80];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", &timestamp);
        return std::string(buffer);
    }

    // Метод для записи данных сообщения в файл
    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios_base::app);
        if (file.is_open()) {
            file << sender << std::endl;
            file << content << std::endl;
            file << getTimestamp() << std::endl;
            std::cout << "Данные успешно записаны в файл." << std::endl;
        }
        else {
            std::cerr << "Ошибка открытия файла для записи." << std::endl;
        }
    }

    // Метод для чтения данных сообщения из файла
    static std::vector<Message> loadFromFile(const std::string& filename) {
        std::vector<Message> messages;
        std::ifstream file(filename);
        if (file.is_open()) {
            std::string sender, content, timestamp;
            while (std::getline(file, sender) && std::getline(file, content) && std::getline(file, timestamp)) {
                messages.emplace_back(sender, content);
            }
            file.close();
            std::cout << "Данные успешно загружены из файла." << std::endl;
        }
        else {
            std::cerr << "Ошибка открытия файла для чтения." << std::endl;
        }
        return messages;
    }

private:
    std::string sender;
    std::string content;
    struct tm timestamp;
};
