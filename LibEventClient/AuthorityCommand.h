#pragma once

#include <string>
#include <vector>
#include <list>
#include "MessageDefine.h"
#include "third_party/chromium/base/basictypes.h"

template<CommandId cmd>
interface CommandParserInterface
{
public:
    virtual bool HandleBussiness(const std::string& request, std::string* response) = 0;
};

class EndPointInfoComputer 
    : public CommandParserInterface<CommandId::CMD_ENDPOINT_INFO_COMPUTER>
{
public:
    EndPointInfoComputer(){};
    virtual ~EndPointInfoComputer(){};

    virtual bool HandleBussiness(const std::string& request, std::string* response) override
    {
        *response = "{result:1}";
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
        return true;
    }

private:
    std::string client_version_;
    std::string client_hash_;
    std::string os_version_;
};

