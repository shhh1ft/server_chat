#pragma once
#include <iostream>
#include <vector>
#include <string>
#include "Profiles.h"

class ProfileManager {
public:
    void addProfile(const std::string& macAddr, const std::string& lastActivity) {
        if (profiles.empty()) {
            std::string nick = "Пользователь";
            nick = generateRandomName(macAddr, nick);
            profiles.emplace_back(nick, macAddr, lastActivity);
            return;
        }
        for (const auto& profile : profiles) {
            if (profile.getMacAddr() == macAddr) {
                return;
            }
            else {
                std::string nick = "Пользователь";
                nick = generateRandomName(macAddr, nick);
                profiles.emplace_back(nick, macAddr, lastActivity);
                return;
            }
        }
    }

    void changeProfileName(const std::string& macAddr, const std::string& newNick) {
        std::string nick = newNick;
        for (auto& profile : profiles) {
            if (profile.getMacAddr() == macAddr) {
                nick = generateRandomName(macAddr, newNick);
                profile.setNick(nick);
                return;
            }
        }
        throw std::runtime_error("Профиль не найден");
    }

    void changeLasActivity(const std::string& macAddr, const std::string& LastActivity) {
        for (auto& profile : profiles) {
            if (profile.getMacAddr() == macAddr) {
                profile.setLastActivity(LastActivity);
                return;
            }
        }
        throw std::runtime_error("Профиль не найден");
    }

    std::string getProfileName(const std::string& macAddr) const {
        for (const auto& profile : profiles) {
            if (profile.getMacAddr() == macAddr) {
                return profile.getNick();
            }
        }
        throw std::runtime_error("Профиль не найден");
    }

    std::string showProfile(const std::string& macAddr) {
        for (auto& profile : profiles) {
            if (profile.getMacAddr() == macAddr) {
                return profile.show();
            }
        }
        throw std::runtime_error("Профиль не найден");
    }

private:
    std::string generateRandomName(std::string macAddr, std::string nick) {
        nick += "#";
        nick += macAddr[1];
        nick += macAddr[4];
        nick += macAddr[6];
        nick += macAddr[3];
        return nick;
    }
    std::vector<Profile> profiles;
};