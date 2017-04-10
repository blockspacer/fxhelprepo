#pragma once

#include "third_party/chromium/base/basictypes.h"
#include <string>
#include <vector>

enum class CommandId
{
    CMD_UNDEFINE = 0,

    CMD_ENDPOINT_INFO_COMPUTER = 1001,
    CMD_ENDPOINT_INFO_EXIT = 1002,
    CMD_ENDPOINT_INFO_DUMP = 1003,

    CMD_LOGIN_SUCCESS = 2001,

    CMD_AUTHORITY_REQUEST = 3001,

    CMD_OPERATION_LOG = 4001,
    CMD_HTTP_LOG = 4002,
};

template<typename ValueType>
ValueType ReadValue(char** data, unsigned int* left)
{
    ValueType value;
    value = *(decltype(value)*)(*data);
    *data += sizeof(decltype(value));
    *left -= sizeof(decltype(value));
    return value;
};

#pragma pack(push, 1)
struct Header
{
    uint16 protocol_version;
    uint16 header_length; // 单位：32bit对齐
    uint32 total_length;  // 单位：字节
    uint32 sequence_number; // 单个session中使用的序列号
    uint32 ack_number; // 回复中使用的序列号

    bool Read(char** data, unsigned int* left)
    {
        if (*left < sizeof(Header))
            return false;

        protocol_version = ReadValue<uint16>(data, left);
        header_length = ReadValue<uint16>(data, left);
        total_length = ReadValue<uint32>(data, left);
        sequence_number = ReadValue<uint32>(data, left);
        ack_number = ReadValue<uint32>(data, left);
        return true;
    }
};
#pragma pack(pop)

interface CommandParserInterface
{
public:
    virtual bool Parse(const std::string& json_data) = 0;

    virtual bool HandleBussiness() = 0;
};

class BufferParser
{
public:
    BufferParser(){};
    ~BufferParser(){};

    // 只要格式不出错，都返回true
    bool HandleBuffer(const std::vector<uint8>& data)
    {
        switch (status_)
        {
        case BufferParser::ParseStatus::INIT:
            buffer_.insert(buffer_.end(), data.begin(), data.end());
            if (buffer_.size() < sizeof(Header))
                continue;

            uint32 left = buffer_.size();
            header_.Read(&buffer_.at(0), &left);

            break;
        case BufferParser::ParseStatus::HEADER:
            break;
        case BufferParser::ParseStatus::DATA:
            break;
        default:
            DCHECK(false);
            return false;
            break;
        }
        // 读取header，凑够header指定数据，利用Parse解析json数据

        return true;
    };

    bool GetResponses(std::vector<std::vector<uint8>>* responses);

private:
    enum class ParseStatus
    {
        INIT = 0,
        HEADER = 1,
        DATA = 2
    };
    bool Parse(const std::vector<uint8>& data, CommandId* command_id,
        const std::string* json_data);

    bool Make(CommandId command_id, const std::string& json_data, 
        std::vector<uint8>* data);

    // 未凑够数据格式的缓存数据
    std::vector<uint8> buffer_;

    ParseStatus status_;
    Header header_;
    
    // 处理完以后可能有多条回复数据, 已经用数据头包装起来
    std::vector<std::vector<uint8>> responses_;
};

class EndPointInfoComputer
{
public:
    EndPointInfoComputer();
    ~EndPointInfoComputer();

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

    // 接收方解析
    bool Parse(const std::string& json_data)
    {
        return true;
    }

private:
    std::string client_version_;
    std::string client_hash_;
    std::string os_version_;
};

