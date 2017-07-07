#include "stdafx.h"
#include "EndPointInfoCommandParser.h"
#include "Network/EncodeHelper.h"


EndPointInfoCommandParser::EndPointInfoCommandParser(uint32 session_id)
    :CommandParser(session_id)
{
}


EndPointInfoCommandParser::~EndPointInfoCommandParser()
{
}

bool EndPointInfoCommandParser::HandleBussiness(const std::string& request, std::string* response)
{
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(request, rootdata, false))
    {
        return false;
    }

    // 暂时没有必要检测status的值
    uint32 cmd = GetInt32FromJsonValue(rootdata, "cmd");
    uint32 sessionid = GetInt32FromJsonValue(rootdata, "sessionid");
    uint32 unixtime = GetInt32FromJsonValue(rootdata, "unixtime");

    Json::Value default(Json::nullValue);

    Json::Value data = rootdata.get("data", default);

    if (data.isNull())
        return false;

    auto member_names = data.getMemberNames();
    for (auto name : member_names)
    {
        if (name.compare("clientversion") ==0)
        {
            client_version_ = data.get(name, "0").asString();
        }
        else if (name.compare("clienthash") == 0)
        {
            client_hash_ = data.get(name, "0").asString();
        }
        else if (name.compare("os_version") == 0)
        {
            os_version_ = data.get(name, "0").asString();
        }
        else
        {
            DCHECK(false && L"Unsupport parameter");
        }
    }
    *response = "{\"result\":1}";
    return true;
}

bool EndPointInfoCommandParser::Make(std::string* json_data)
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


