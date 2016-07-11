#include "stdafx.h"
#include "Config.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"

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
    const std::wstring& password, bool remember) const
{  
    WritePrivateProfileString(L"UserInfo", L"UserName", Encrypt(username).c_str(),
        filepath_.c_str());
    WritePrivateProfileString(L"UserInfo", L"Password", Encrypt(password).c_str(),
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

bool Config::SaveApiKey(const std::wstring& apikey) const
{
    WritePrivateProfileString(L"Robot", L"ApiKey", Encrypt(apikey).c_str(),
                              filepath_.c_str());
    return true;
}

bool Config::GetApiKey(std::wstring* apikey) const
{
    wchar_t temp[128] = { 0 };
    int32 count = GetPrivateProfileString(L"Robot", L"ApiKey", L"",
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
