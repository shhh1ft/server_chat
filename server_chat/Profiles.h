#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <cstdlib>

class MACHandler {
private:
    std::string filename;
    std::map<std::string, std::string> idMap;

    std::string generateRandomName(std::string id, std::string nick) {
        std::string name = nick;
        name += "#";
        name += id[1];
        name += id[4];
        name += id[6];
        name += id[3];
        return name;
    }

    void writeToFile() {
        std::ofstream file(filename);
        if (file.is_open()) {
            for (const auto& pair : idMap) {
                file << pair.first << " " << pair.second << std::endl;
            }
            file.close();
        }
        else {
            std::cerr << "Ошибка: Не удалось открыть файл для записи." << std::endl;
        }
    }

public:
    MACHandler(const std::string& filename) : filename(filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            std::string id, name;
            while (file >> id >> name) {
                idMap[id] = name;
            }
            file.close();
        }
    }

    void addID(const std::string& id) {
        if (idMap.find(id) == idMap.end()) {
            std::string nick = "Пользователь"; // Изменение по-умолчанию
            std::string randomName = generateRandomName(id, nick);
            idMap[id] = randomName;

            // Записываем данные в файл
            writeToFile();
        }
    }

    std::string getNameByMAC(const std::string& id) const {
        auto it = idMap.find(id);
        if (it != idMap.end()) {
            return it->second;
        }
        else {
            return "";
        }
    }

    void changeNick(const std::string& id, const std::string& newNick) {
        auto it = idMap.find(id);
        if (it != idMap.end()) {
            it->second = newNick;
            writeToFile();
        }
    }
};
