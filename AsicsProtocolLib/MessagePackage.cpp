#include "stdafx.h"


#include <memory>
#include "MessagePackage.h"

#include "Network/EncodeHelper.h"
#include "CommandParser.h"
#include "CommandParserFactory.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/memory/scoped_ptr.h"

Package::Package(uint32 sequence_number)
    :sequence_number_(sequence_number)
{
}

Package::~Package()
{
}

void Package::SetHeader(const Header& header)
{
    request_header_ = header;
}
void Package::SetData(const std::string& data)
{
    request_data_ = data;
}

bool Package::HandlePackage(std::vector<uint8>* response_package)
{
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(request_data_, rootdata, false))
    {
        return false;
    }

    // 暂时没有必要检测status的值
    uint32 cmd = GetInt32FromJsonValue(rootdata, "cmd");
    uint32 sessionid = GetInt32FromJsonValue(rootdata, "sessionid");
    uint32 unixtime = GetInt32FromJsonValue(rootdata, "unixtime");

    if (!IsValidCmd(cmd))
        return false;

    CommandId cmdid = static_cast<CommandId>(cmd);
    scoped_ptr<CommandParserInterface> parser_ptr(CommandParserFactory::CreateCommandParser(cmdid, sessionid));
    if (!parser_ptr.get())
        return false;

    std::string response_data;
    if (!parser_ptr->HandleBussiness(request_data_, &response_data))
        return false;

    Header response_header;
    response_header = request_header_;
    response_header.sequence_number = sequence_number_;
    response_header.ack_number = request_header_.sequence_number + 1;
    response_header.timestamp = static_cast<uint32>(base::Time::Now().ToDoubleT());

    std::unique_ptr<uint8[]> header_stream(new uint8[sizeof(Header)]);
    uint8* temp_ptr = header_stream.get();
    if (!response_header.Write(&temp_ptr, sizeof(Header)))
        return false;

    response_package->assign(header_stream.get(), header_stream.get() + sizeof(Header));
    response_package->insert(response_package->end(), response_data.begin(), response_data.end());
    return true;
}

bool Package::GetPackage(std::vector<uint8>* request_package)
{
    std::unique_ptr<uint8[]> header_stream(new uint8[sizeof(Header)]);
    uint8* temp_ptr = header_stream.get();
    if (!request_header_.Write(&temp_ptr, sizeof(Header)))
        return false;

    request_package->assign(header_stream.get(), header_stream.get() + sizeof(Header));
    request_package->insert(request_package->end(), request_data_.begin(), request_data_.end());

    return true;
}

bool Package::IsValidCmd(uint32 cmd) const
{
    if (cmd <= static_cast<uint32>(CommandId::CMD_UNDEFINE) ||
        cmd >= static_cast<uint32>(CommandId::CMD_END))
    {
        // 超出命令范围
        return false;
    }
    return true;
}