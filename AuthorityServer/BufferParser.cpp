#include "stdafx.h"
#include "BufferParser.h"
#include "AsicsProtocolLib/MessagePackage.h"


BufferParser::BufferParser()
    :status_(BufferParser::ParseStatus::INIT)
{
};
BufferParser::~BufferParser(){};

    // 只要格式不出错，都返回true
bool BufferParser::HandleBuffer(const std::vector<uint8>& data)
{
    switch (status_)
    {
    case BufferParser::ParseStatus::INIT:
    {
        buffer_.insert(buffer_.end(), data.begin(), data.end());
        if (buffer_.size() < sizeof(Header))
            return true;

        status_ = BufferParser::ParseStatus::HEADER;
        // 不跳出switch,是故意的
    }
    case BufferParser::ParseStatus::HEADER:
    {
        uint32 left = buffer_.size();
        uint8* buffer_begin = &buffer_[0];
        if (!header_.Read(&buffer_begin, &left))
            return false;

        status_ = BufferParser::ParseStatus::DATA;
        // 不跳出switch,是故意的
    }
    case BufferParser::ParseStatus::DATA:
    {
        if (buffer_.size() <= header_.total_length - header_.header_length)
            break;

        Package package(GetSquenceNumber());
        package.SetHeader(header_);
        std::string data;
        data.assign(buffer_.begin() + header_.header_length,
            buffer_.begin() + header_.total_length);

        package.SetData(data);

        std::vector<uint8> response;
        package.HandlePackage(&response);
        responses_.push_back(response);

        buffer_.erase(buffer_.begin(), buffer_.begin() + header_.total_length);
        status_ = BufferParser::ParseStatus::INIT;
        HandleBuffer(std::vector<uint8>());
        break;
    }

    default:
        //DCHECK(false);
        return false;
        break;
    }

    return true;
};

bool BufferParser::GetResponses(std::vector<std::vector<uint8>>* responses)
{
    if (responses_.empty())
        return false;

    responses->swap(responses_);
    return true;
};

bool BufferParser::Parse(const std::vector<uint8>& data, CommandId* command_id,
    const std::string* json_data)
{
    return false;
}

bool BufferParser::Make(CommandId command_id, const std::string& json_data,
    std::vector<uint8>* data)
{
    return false;
}

uint32 BufferParser::GetSquenceNumber()
{
    return sequence_number_.GetNext();
}