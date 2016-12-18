#include "stdafx.h"
#include <fstream>
#include "EnterRoomStrategy.h"

#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/json/json.h"

#include "Network/EncodeHelper.h"
#include "Network/MessageNotifyManager.h"


EnterRoomStrategy::EnterRoomStrategy()
{

}

EnterRoomStrategy::~EnterRoomStrategy()
{

}

void EnterRoomStrategy::SetWelcomeFlag(bool enable)
{
    welcome_flag_ = enable;
}

void EnterRoomStrategy::SetWelcomeLevel(uint32 level)
{
    welcome_level_ = level;
}

void EnterRoomStrategy::SetWelcomeVipV(bool enable)
{
    vip_v_ = enable;
}

void EnterRoomStrategy::SetSpecialWelcomeContent(
    const std::map<uint32, WelcomeInfo>& special_welcome)
{
    base::AutoLock lock(welcome_lock_);
    welcome_info_map_ = special_welcome;
    SaveSpecialWelcomeContent(welcome_info_map_);
}

void EnterRoomStrategy::GetSpecialWelcomeContent(
    std::map<uint32, WelcomeInfo>* special_welcome)
{
    if (!welcome_info_map_.empty())
    {
        *special_welcome = welcome_info_map_;
    }
    LoadSpecialWelcomeContent(special_welcome);
    welcome_info_map_ = *special_welcome;
}

void EnterRoomStrategy::SetNormalWelcomeContent(
    const std::wstring& normal_welcome)
{
    normal_welcome_ = normal_welcome;

    std::wstring user_pos = L"[玩家]";
    auto find_name = normal_welcome_.find(user_pos);
    if (find_name!=std::wstring::npos)
    {
        pre_ = normal_welcome_.substr(0, find_name);
        post_ = normal_welcome_.substr(find_name + user_pos.length());
    }

    SaveNormalWelcomeContent(normal_welcome_);
}

bool EnterRoomStrategy::GetNormalWelcomeContent(
    std::wstring* normal_welcome)
{
    if (!normal_welcome_.empty())
    {
        *normal_welcome = normal_welcome_;
        return true;
    }

    if (!LoadNormalWelcomeContent(normal_welcome))
    {
        *normal_welcome = L"欢迎[玩家]进入直播间";
    }
    
    std::wstring user_pos = L"[玩家]";
    auto find_name = normal_welcome_.find(user_pos);
    if (find_name != std::wstring::npos)
    {
        pre_ = normal_welcome_.substr(0, find_name);
        post_ = normal_welcome_.substr(find_name + user_pos.length());
    }

    normal_welcome_ = *normal_welcome;
    return true;
}

bool EnterRoomStrategy::SaveSpecialWelcomeContent(
    const std::map<uint32, WelcomeInfo>& welcome_info_map) const
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"welcome_content.txt";
    base::FilePath pathname = dirPath.Append(filename);

    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& welcome_info : welcome_info_map)
    {
        Json::Value jvWelcomeInfo(Json::objectValue);
        jvWelcomeInfo["fanxingid"] = welcome_info.second.fanxing_id;
        jvWelcomeInfo["name"] = base::WideToUTF8(welcome_info.second.name);
        jvWelcomeInfo["content"] = base::WideToUTF8(welcome_info.second.content);
        root.append(jvWelcomeInfo);
    }

    std::string writestring = writer.write(root);

    std::ofstream ofs(pathname.value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << writestring;
    ofs.flush();
    ofs.close();

    return true;
}

bool EnterRoomStrategy::LoadSpecialWelcomeContent(
    std::map<uint32, WelcomeInfo>* special_welcome)
{
    if (!special_welcome)
        return false;

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"welcome_content.txt";
    base::FilePath pathname = dirPath.Append(filename);

    std::ifstream ovrifs;
    ovrifs.open(pathname.value());
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;

    std::string data = ss.str();
    try
    {
        Json::Reader reader;
        Json::Value root(Json::objectValue);
        if (!Json::Reader().parse(data.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isArray())
        {
            assert(false && L"root is not array");
            return false;
        }

        for (const auto& value : root)//for data
        {
            Json::Value temp;
            uint32 fanxingid = GetInt32FromJsonValue(value, "fanxingid");
            std::string name = value.get("name", "").asString();
            std::string content = value.get("content", "").asString();
            WelcomeInfo welcome_info = { fanxingid, base::UTF8ToWide(name), base::UTF8ToWide(content) };
            (*special_welcome)[fanxingid] = welcome_info;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool EnterRoomStrategy::SaveNormalWelcomeContent(const std::wstring& normal_welcome) const
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"normal_welcome_content_V6.txt";
    base::FilePath pathname = dirPath.Append(filename);

    Json::FastWriter writer;
    Json::Value root(Json::objectValue);

    root["normal_welcome"] = base::WideToUTF8(normal_welcome);

    std::string writestring = writer.write(root);

    std::ofstream ofs(pathname.value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << writestring;
    ofs.flush();
    ofs.close();

    return true;
}
bool EnterRoomStrategy::LoadNormalWelcomeContent(std::wstring* normal_welcome)
{
    if (!normal_welcome)
        return false;

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"normal_welcome_content_V6.txt";
    base::FilePath pathname = dirPath.Append(filename);

    std::ifstream ovrifs;
    ovrifs.open(pathname.value());
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;

    std::string data = ss.str();
    try
    {
        Json::Reader reader;
        Json::Value root(Json::objectValue);
        if (!Json::Reader().parse(data.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isObject())
        {
            assert(false && L"root is not object");
            return false;
        }

        std::string content = root.get("normal_welcome", "").asString();
        if (content.empty())
        {
            return false;
        }
        *normal_welcome = base::UTF8ToWide(content);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool EnterRoomStrategy::GetEnterWelcome(const EnterRoomUserInfo& enterinfo,
    std::wstring* chatmessage)
{
    if (!welcome_flag_)
        return false;

    LOG(INFO) << __FUNCTION__ << L"rich level = " << base::UintToString16(enterinfo.richlevel) << L" / " << base::UintToString16(welcome_level_) << L"]";

    if (enterinfo.richlevel < welcome_level_)//等级低于指定等级的不设置欢迎
        return false;

    if (!vip_v_ && enterinfo.vip_v) // 如果用户是白金vip,并且主播没有设置识别隐身用户,则不欢迎.
    {
        *chatmessage = L"欢迎神秘嘉宾";
        return false;
    }

    std::wstring msg;
    base::AutoLock lock(welcome_lock_);
    auto find = welcome_info_map_.find(enterinfo.userid);
    if (find != welcome_info_map_.end())
    {
        msg = find->second.content;
    }
    else
    {
        msg = pre_ + base::UTF8ToWide(enterinfo.nickname) + post_;
    }

    chatmessage->assign(msg.begin(), msg.end());
    return true;
}
