#pragma once

#include "CommandParser.h"

class EndPointInfoCommandParser : public CommandParser<CommandId::CMD_ENDPOINT_INFO_COMPUTER>
{
public:
    EndPointInfoCommandParser(uint32 session_id);
    ~EndPointInfoCommandParser();

    virtual bool HandleBussiness(const std::string& request, std::string* response);
    // 发送方组包, 包括命令号，不包括协议头
    virtual bool Make(std::string* json_data);

    void SetClientVersion(const std::string& client_version);
    std::string GetClientVersion() const;

    void SetClientHash(const std::string& client_hash);
    std::string GetClientHash() const;

    void SetOsVersion(const std::string& os_version);
    std::string GetOsVersion() const;



private:
    std::string client_version_;
    std::string client_hash_;
    std::string os_version_;
};

