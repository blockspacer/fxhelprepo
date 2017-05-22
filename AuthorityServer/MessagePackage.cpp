#include "stdafx.h"

#include <memory>
#include "MessagePackage.h"
#include "third_party/json/json.h"
#include "Network/EncodeHelper.h"
#include "AuthorityCommand.h"

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
    Json::Value jvCmd(Json::ValueType::intValue);
    uint32 cmd = GetInt32FromJsonValue(rootdata, "cmd");
    uint32 sessionid = GetInt32FromJsonValue(rootdata, "sessionid");
    uint32 unixtime = GetInt32FromJsonValue(rootdata, "unixtime");

    if (cmd <= static_cast<uint32>(CommandId::CMD_UNDEFINE) ||
        cmd >= static_cast<uint32>(CommandId::CMD_END))
    {
        // 超出命令范围
        return false;
    }
    CommandId cmdid = static_cast<CommandId>(cmd);
    // 正常处理命令
    EndPointInfoComputer endpoint;
    std::string response_data;
    Header response_header;
    if (endpoint.HandleBussiness(request_data_, &response_data))
        return false;

    response_header = request_header_;
    response_header.sequence_number = sequence_number_;
    response_header.ack_number = request_header_.sequence_number + 1;
    response_header.timestamp = static_cast<uint32>(base::Time::Now().ToDoubleT());

    uint8* header_stream = new uint8[sizeof(Header)];
    if (!response_header.Write(&header_stream, sizeof(Header)))
        return false;

    response_package->assign(header_stream, header_stream + sizeof(Header));
    response_package->insert(response_package->end(), response_data.begin(), response_data.end());

    return true;
}

bool Package::GetPackage(std::vector<uint8>* request_package)
{
    return false;
}