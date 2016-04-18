#include "CookiesHelper.h"

#include <memory>
#include <windows.h>
#include <assert.h>
#include "EncodeHelper.h"

CookiesHelper::CookiesHelper()
{
}


CookiesHelper::~CookiesHelper()
{
}

bool CookiesHelper::SetCookies(const std::string& key, const std::string& cookie)
{
    cookies_[key] = cookie;
    return true;
}

bool CookiesHelper::SetCookies(const std::string& keyvalue)
{
    auto pos = keyvalue.find("=");
    if (pos==std::string::npos)
    {
        return false;
    }

    std::string key = keyvalue.substr(0, pos);
    return SetCookies(key, keyvalue);
}

std::string CookiesHelper::GetCookies(const std::string& key) const
{
    std::map<std::string,std::string>::const_iterator result = 
        cookies_.find(key);
    if (result != cookies_.end())
    {
        return result->second;
    }
    return std::string("");
}
std::string CookiesHelper::GetCookies(const std::vector<std::string>& keys) const
{
    std::string temp;
    for (const auto& key : keys)
    {
        auto result = cookies_.find(key);
        if (result!=cookies_.end())
        {
            temp += result->second;
            temp += ";";
        }
    }
    if (temp.empty())
    {
        return temp;
    }

    temp = temp.substr(0, temp.length() - 1);
    return temp;
}
