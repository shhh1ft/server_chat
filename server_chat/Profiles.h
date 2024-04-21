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
    
    std::string generateRandomName(std::string id) {
        std::string name = "User";
        name += " #";
        name += id[1];
        name += id[4];
        name += id[6];
        name += id[3];
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
            std::string randomName = generateRandomName(id);
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
