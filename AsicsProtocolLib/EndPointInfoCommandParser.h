#pragma once

#include "CommandParser.h"

class EndPointInfoCommandParser
{
public:
    EndPointInfoCommandParser();
    ~EndPointInfoCommandParser();
};


template<>
class CommandParser<CommandId::CMD_ENDPOINT_INFO_COMPUTER> :public CommandParserInterface
{
public:
    CommandParser(uint32 session_id) :session_id_(session_id){};
    virtual ~CommandParser(){};

    virtual bool HandleBussiness(const std::string& request, std::string* response)
    {
        *response = "{\"result\":1}";
        return true;
    }

    void SetClientVersion(const std::string& client_version);
    std::string GetClientVersion() const;

    void SetClientHash(const std::string& client_hash);
    std::string GetClientHash() const;

    void SetOsVersion(const std::string& os_version);
    std::string GetOsVersion() const;

    // 发送方组包, 包括命令号，不包括协议头
    bool Make(std::string* json_data)
    {
        Json::Value root(Json::objectValue);
        root["cmd"] = CommandId::CMD_ENDPOINT_INFO_COMPUTER;
        root["sessionid"] = session_id_;
        root["unixtime"] = static_cast<uint32>(base::Time::Now().ToDoubleT());

        Json::Value data(Json::objectValue);
        data["clientversion"] = client_version_;
        data["clienthash"] = client_hash_;
        data["os_version"] = os_version_;

        root["data"] = data;

        Json::FastWriter writer;
        *json_data = writer.write(root);

        return true;
    }

private:
    uint32 session_id_;
    std::string client_version_;
    std::string client_hash_;
    std::string os_version_;
};

