#pragma once

#include <string>
#include <vector>

class ServerHelper
{
public:
    ServerHelper();
    ~ServerHelper();
    static bool GetChatServerIp(std::vector<std::string>* hostips);
};

