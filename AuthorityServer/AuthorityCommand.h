#pragma once

#include <string>
#include <vector>
#include <list>
#include "MessageDefine.h"
#include "third_party/chromium/base/basictypes.h"
#undef max
#undef min
#include "third_party/chromium/base/time/time.h"
#include "third_party/json/writer.h"


class CommandParserInterface
{
public:
    virtual bool HandleBussiness(const std::string& request, std::string* response) = 0;
};

template<CommandId cmdid>
class CommandParser :public CommandParserInterface
{
public:
    CommandParser(uint32 session_id) :session_id_(session_id){};
    virtual ~CommandParser(){};
    virtual bool HandleBussiness(const std::string& request, std::string* response)
    {
        *response = "{\"result\":0}";
        return false;
    };
private:
    uint32 session_id_;
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


class CommandParserFactory
{
public:
    static CommandParserInterface* CreateCommandParser(CommandId cmd_id, uint32 session_id)
    {
        CommandParserInterface* ptr = nullptr;
        switch (cmd_id)
        {
        case CMD_ENDPOINT_INFO_COMPUTER:
            ptr = new CommandParser<CMD_ENDPOINT_INFO_COMPUTER>(session_id);
            break;

        default:
            ptr = new CommandParser<CMD_UNDEFINE>(session_id);
        }

        return ptr;
    }
};

