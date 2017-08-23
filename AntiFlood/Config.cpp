#include "stdafx.h"
#include <string>
#include <memory>
#include <map>
#include "Config.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace{

    std::wstring Encrypt(const std::wstring& plain)
    {
        if (plain.empty())
            return L"";

        std::wstring cipher;
        for (const auto& it : plain)
        {
            wchar_t c = it + 1;
            cipher.push_back(c);
        }
        return cipher;
    }
    std::wstring Decrypt(const std::wstring& cipher)
    {
        if (cipher.empty())
            return L"";

        std::wstring plain;
        for (const auto& it : cipher)
        {
            wchar_t c = it - 1;
            //wchar_t c = it ;
            plain.push_back(c);
        }
        return plain;
    }
}
Config::Config()
{
    base::FilePath path;
    PathService::Get(base::DIR_HOME, &path);
    path = path.Append(L"fanxing_config.ini");
    filepath_ = path.value();
}

Config::~Config()
{
}

bool Config::GetUserName(std::wstring* username) const
{
    wchar_t temp[128] = { 0 };
    int32 count =GetPrivateProfileString(L"UserInfo", L"UserName", L"", 
        temp, 128, filepath_.c_str());
    if (count>=128 || count<=0)
    {
        return false;
    }
    std::wstring tempstr;
    tempstr.assign(temp, temp + count);
    *username = Decrypt(tempstr);
    return true;
}

bool Config::GetPassword(std::wstring* password) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"UserInfo", L"Password", L"",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring tempstr;
    tempstr.assign(temp, temp + count);
    *password = Decrypt(tempstr);
    return true;
}

bool Config::GetCookies(std::wstring* cookies) const
{
    int32 count = 4096;
    std::unique_ptr<wchar_t[]> buffer(new wchar_t[4096]);
    int ret = GetPrivateProfileString(L"UserInfo", L"Cookies", L"",
        buffer.get(), count, filepath_.c_str());

    DCHECK(count > ret);

    std::wstring tempstr;
    tempstr.assign(buffer.get(), buffer.get() + count);
    *cookies = Decrypt(tempstr);
    return true;
}

bool Config::GetRoomid(std::wstring* roomid) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"RoomInfo", L"RoomId", L"",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring tempstr;
    tempstr.assign(temp, temp + count);
    *roomid = Decrypt(tempstr);
    return true;
}

bool Config::GetRemember() const
{
    int result = GetPrivateProfileInt(L"Program", L"Remember", 0, 
        filepath_.c_str());
    return result == 1;
}

bool Config::SaveUserInfo(const std::wstring& username,
    const std::wstring& password, const std::wstring& cookies, bool remember) const
{  
    WritePrivateProfileString(L"UserInfo", L"UserName", Encrypt(username).c_str(),
        filepath_.c_str());
    WritePrivateProfileString(L"UserInfo", L"Password", Encrypt(password).c_str(),
        filepath_.c_str());

    WritePrivateProfileString(L"UserInfo", L"Cookies", Encrypt(cookies).c_str(),
        filepath_.c_str());

    std::wstring str = remember ? L"1" : L"0";
    WritePrivateProfileString(L"Program", L"Remember", str.c_str(),
        filepath_.c_str());
    return true;
}

bool Config::SaveRoomId(const std::wstring& roomid) const
{
    WritePrivateProfileString(L"RoomInfo", L"RoomId", Encrypt(roomid).c_str(),
        filepath_.c_str());
    return true;
}

bool Config::SaveRobot(bool enable, const std::wstring& apikey) const
{
    std::wstring str = enable ? L"1" : L"0";
    WritePrivateProfileString(L"Robot", L"Enable", str.c_str(), filepath_.c_str());
    WritePrivateProfileString(L"Robot", L"ApiKey", Encrypt(apikey).c_str(),
                              filepath_.c_str());
    return true;
}

bool Config::GetRobot(bool* enable, std::wstring* apikey) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"Robot", L"Enable", L"0",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring str = temp;
    *enable = (str.compare(L"1") == 0);

    count = GetPrivateProfileString(L"Robot", L"ApiKey", L"",
                                    temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring tempstr;
    tempstr.assign(temp, temp + count);
    *apikey = Decrypt(tempstr);
    return true;
}

bool Config::SaveGiftThanks(bool enable, uint32 gift_value) const
{
    std::wstring str = enable ? L"1" : L"0";
    WritePrivateProfileString(L"Thanks", L"Enable", str.c_str(), filepath_.c_str());
    str = base::UintToString16(gift_value);
    WritePrivateProfileString(L"Thanks", L"GiftValue", str.c_str(), filepath_.c_str());
    return true;
}
bool Config::GetGiftThanks(bool* enable, uint32* gift_value) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"Thanks", L"Enable", L"0",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring str = temp;
    *enable = (str.compare(L"1") == 0);

    count = GetPrivateProfileString(L"Thanks", L"GiftValue", L"0",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    str = temp;
    return base::StringToUint(base::WideToUTF8(str), gift_value);
}

bool Config::SaveEnterRoomWelcome(bool enable, uint32 rich_level) const
{
    std::wstring str = enable ? L"1" : L"0";
    WritePrivateProfileString(L"Welcome", L"Enable", str.c_str(), filepath_.c_str());
    str = base::UintToString16(rich_level);
    WritePrivateProfileString(L"Welcome", L"RichLevel", str.c_str(), filepath_.c_str());
    return true;
}

bool Config::GetEnterRoomWelcome(bool* enable, uint32* rich_level) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"Welcome", L"Enable", L"0",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring str = temp;
    *enable = (str.compare(L"1") == 0);

    count = GetPrivateProfileString(L"Welcome", L"RichLevel", L"0",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    str = temp;
    return base::StringToUint(base::WideToUTF8(str), rich_level);
}

bool Config::SaveRepeatChat(bool enable, 
                            const std::wstring& content, 
                            const std::wstring& seconds) const
{
    std::wstring str = enable ? L"1" : L"0";
    WritePrivateProfileString(L"Repeat", L"Enable", str.c_str(), filepath_.c_str());
    WritePrivateProfileString(L"Repeat", L"ChatContent", content.c_str(),
        filepath_.c_str());
    WritePrivateProfileString(L"Repeat", L"Seconds", seconds.c_str(),
        filepath_.c_str());
    return true;
}

bool Config::GetRepeatChat(bool* enable, std::wstring* content, std::wstring* seconds) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"Repeat", L"Enable", L"0",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring str = temp;
    *enable = (str.compare(L"1") == 0);

    count = GetPrivateProfileString(L"Repeat", L"ChatContent", L"",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    *content = temp;

    count = GetPrivateProfileString(L"Repeat", L"Seconds", L"",
        temp, 128, filepath_.c_str());
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    *seconds = temp;
    return true;
}