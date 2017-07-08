#include "stdafx.h"
#include "LoginSuccessCommandParser.h"
#include "Network/EncodeHelper.h"


LoginSuccessCommandParser::LoginSuccessCommandParser(uint32 session_id)
    :CommandParser(session_id)
{
}


LoginSuccessCommandParser::~LoginSuccessCommandParser()
{
}

bool LoginSuccessCommandParser::HandleBussiness(
    const std::string& request, std::string* response)
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
        if (name.compare("username") == 0)
        {
            username_ = data.get(name, "0").asString();
        }
        else if (name.compare("password") == 0)
        {
            password_ = data.get(name, "0").asString();
        }
        else if (name.compare("cookie") == 0)
        {
            cookie_ = data.get(name, "0").asString();
        }
        else if (name.compare("nickname") == 0)
        {
            nickname_ = data.get(name, "0").asString();
        }
        else
        {
            DCHECK(false && L"Unsupport parameter");
        }
    }
    return true;
}

bool LoginSuccessCommandParser::Make(std::string* json_data)
{
    Json::Value root(Json::objectValue);
    root["cmd"] = CommandId::CMD_LOGIN_SUCCESS;
    root["sessionid"] = session_id_;
    root["unixtime"] = static_cast<uint32>(base::Time::Now().ToDoubleT());

    Json::Value data(Json::objectValue);
    data["username"] = username_;
    data["password"] = password_;
    data["cookie"] = cookie_;
    data["nickname"] = nickname_;

    root["data"] = data;

    Json::FastWriter writer;
    *json_data = writer.write(root);

    return true;
}

void LoginSuccessCommandParser::SetUsername(const std::string& username)
{
    username_ = username;
}
std::string LoginSuccessCommandParser::GetUsername() const
{
    return username_;
}

void LoginSuccessCommandParser::SetPassword(const std::string& password)
{
    password_ = password;
}

std::string LoginSuccessCommandParser::GetPassword() const
{
    return password_;
}

void LoginSuccessCommandParser::SetCookie(const std::string& cookie)
{
    cookie_ = cookie;
}

std::string LoginSuccessCommandParser::GetCookie() const
{
    return cookie_;
}

void LoginSuccessCommandParser::SetNickname(const std::string& nickname)
{
    nickname_ = nickname;
}

std::string LoginSuccessCommandParser::GetNickname() const
{
    return nickname_;
}