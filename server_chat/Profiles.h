#pragma once
#include <iostream>
#include <vector>
#include <string>

class Profile {
public:
    Profile(const std::string& nick, const std::string& macAddr, const std::string& lastActivity)
        : nick(nick), macAddr(macAddr), lastActivity(lastActivity) {}

    std::string getNick() const { return nick; }
    std::string getMacAddr() const { return macAddr; }
    std::string getLastActivity() const { return lastActivity; }

    void setNick(const std::string& newNick) { nick = newNick; }
    void setMacAddr(const std::string& newMacAddr) { macAddr = newMacAddr; }
    void setLastActivity(const std::string& newLastActivity) { lastActivity = newLastActivity; }

    std::string show() const {
        return "Ник: " + nick + ", MAC Адрес: " + macAddr + ", Последняя активность: " + lastActivity;
    }

private:
    std::string nick;
    std::string macAddr;
    std::string lastActivity;
};
