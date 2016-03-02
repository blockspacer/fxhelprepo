#include "stdafx.h"
#include "Config.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"


Config::Config()
{
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    path = path.Append(L"config.ini");
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
    username->assign(temp, temp + count);
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
    password->assign(temp, temp + count);
    return true;
}

bool Config::GetRemember() const
{
    int result = GetPrivateProfileInt(L"Program", L"Remember", 0, 
        filepath_.c_str());
    return result == 1;
}

bool Config::Save(const std::wstring& username,
    const std::wstring& password, bool remember) const
{
    WritePrivateProfileString(L"UserInfo", L"UserName", username.c_str(),
        filepath_.c_str());
    WritePrivateProfileString(L"UserInfo", L"Password", password.c_str(),
        filepath_.c_str());

    std::wstring str = remember ? L"1" : L"0";
    WritePrivateProfileString(L"Program", L"Remember", str.c_str(),
        filepath_.c_str());
    return true;
}
