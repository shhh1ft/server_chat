#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <ctime>
#include <cstdlib>

class MACHandler {
private:
    std::string filename;
    std::map<std::string, std::string> idMap;

    // Генерация случайного имени
    std::string generateRandomName() {
        std::string name = "User";
        int randomNum = std::rand() % 1000;
        name += std::to_string(randomNum);
        return name;
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
            std::string randomName = generateRandomName();
            idMap[id] = randomName;

            // Записываем данные в файл
            std::ofstream file(filename, std::ios_base::app);
            if (file.is_open()) {
                file << id << " " << randomName << std::endl;
                file.close();
            }
            else {
                std::cerr << "Error: Unable to open file for writing." << std::endl;
            }
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
};
