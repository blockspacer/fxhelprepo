#include "stdafx.h"
#include "Config.h"

#include <string>
#include <map>

#include "VMProtect/VMProtectSDK.h"
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
    VMProtectBeginUltra(__FUNCTION__);
    path = path.Append(VMProtectDecryptStringW(L"fanxing_config.ini"));
    VMProtectEnd();
    filepath_ = path.value();
}

Config::~Config()
{
}

bool Config::GetUserName(std::wstring* username) const
{
    wchar_t temp[128] = { 0 };
    VMProtectBeginUltra(__FUNCTION__);
    int32 count = GetPrivateProfileString(VMProtectDecryptStringW(L"UserInfo"), 
        VMProtectDecryptStringW(L"UserName"), VMProtectDecryptStringW(L""),
        temp, 128, filepath_.c_str());
    VMProtectEnd();
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
    VMProtectBeginUltra(__FUNCTION__);
    int32 count = GetPrivateProfileString(VMProtectDecryptStringW(L"UserInfo"), 
        VMProtectDecryptStringW(L"Password"), VMProtectDecryptStringW(L""),
        temp, 128, filepath_.c_str());
    VMProtectEnd();
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
    VMProtectBeginUltra(__FUNCTION__);
    int32 count = GetPrivateProfileString(VMProtectDecryptStringW(L"RoomInfo"), 
        VMProtectDecryptStringW(L"RoomId"), VMProtectDecryptStringW(L""),
        temp, 128, filepath_.c_str());
    VMProtectEnd();
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
    VMProtectBeginUltra(__FUNCTION__);
    int result = GetPrivateProfileInt(VMProtectDecryptStringW(L"Program"), 
        VMProtectDecryptStringW(L"Remember"), 0,
        filepath_.c_str());
    VMProtectEnd();
    return result == 1;
}

bool Config::SaveUserInfo(const std::wstring& username,
    const std::wstring& password, bool remember) const
{  
    VMProtectBeginUltra(__FUNCTION__);
    WritePrivateProfileString(VMProtectDecryptStringW(L"UserInfo"), 
        VMProtectDecryptStringW(L"UserName"), Encrypt(username).c_str(),
        filepath_.c_str());
    WritePrivateProfileString(VMProtectDecryptStringW(L"UserInfo"), 
        VMProtectDecryptStringW(L"Password"), Encrypt(password).c_str(),
        filepath_.c_str());

    std::wstring str = remember ? L"1" : L"0";
    WritePrivateProfileString(VMProtectDecryptStringW(L"Program"), 
        VMProtectDecryptStringW(L"Remember"), str.c_str(),
        filepath_.c_str());
    VMProtectEnd();
    return true;
}

bool Config::SaveRoomId(const std::wstring& roomid) const
{
    VMProtectBeginUltra(__FUNCTION__);
    WritePrivateProfileString(VMProtectDecryptStringW(L"RoomInfo"), 
        VMProtectDecryptStringW(L"RoomId"), Encrypt(roomid).c_str(),
        filepath_.c_str());
    VMProtectEnd();
    return true;
}

bool Config::SaveApiKey(const std::wstring& apikey) const
{
    VMProtectBeginUltra(__FUNCTION__);
    WritePrivateProfileString(VMProtectDecryptStringW(L"Robot"), 
        VMProtectDecryptStringW(L"ApiKey"), Encrypt(apikey).c_str(),
        filepath_.c_str());
    VMProtectEnd();
    return true;
}

bool Config::GetApiKey(std::wstring* apikey) const
{
    wchar_t temp[128] = { 0 };
    VMProtectBeginUltra(__FUNCTION__);
    int32 count = GetPrivateProfileString(VMProtectDecryptStringW(L"Robot"), 
        VMProtectDecryptStringW(L"ApiKey"), VMProtectDecryptStringW(L""),
        temp, 128, filepath_.c_str());
    VMProtectEnd();
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    std::wstring tempstr;
    tempstr.assign(temp, temp + count);
    *apikey = Decrypt(tempstr);
    return true;
}

bool Config::SaveNormalWelcome(const std::wstring& content) const
{
    VMProtectBeginUltra(__FUNCTION__);
    WritePrivateProfileString(VMProtectDecryptStringW(L"Normal"), 
        VMProtectDecryptStringW(L"Welcome"), content.c_str(),
        filepath_.c_str());
    VMProtectEnd();
    return true;
}

bool Config::GetNormalWelcome(std::wstring* content)
{
    wchar_t temp[128] = { 0 };
    VMProtectBeginUltra(__FUNCTION__);
    int32 count = GetPrivateProfileString(VMProtectDecryptStringW(L"Normal"), 
        VMProtectDecryptStringW(L"Welcome"), VMProtectDecryptStringW(L""),
        temp, 128, filepath_.c_str());
    VMProtectEnd();
    if (count >= 128 || count <= 0)
    {
        return false;
    }
    *content = temp;
    return true;
}