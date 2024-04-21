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

    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename, std::ios_base::app);
        if (file.is_open()) {
            file << sender << std::endl;
            file << content << std::endl;
            file << timestamp << std::endl;
            std::cout << "Данные успешно записаны в файл." << std::endl;
        }
        else {
            std::cerr << "Ошибка открытия файла для записи." << std::endl;
        }
    }

    static std::vector<Message> loadFromFile(const std::string& filename) {
        std::vector<Message> messages;
        std::ifstream file(filename);
        if (file.is_open()) {
            std::string sender, content, timestamp;
            while (std::getline(file, sender) && std::getline(file, content) && std::getline(file, timestamp)) {
                messages.emplace_back(sender, content, timestamp);
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
    std::string timestamp; // Теперь храним время как строку
};
