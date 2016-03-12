#pragma once
#include <string>
#include <map>
#include <vector>

// CookiesManager提供获取本地fanxing的http请求相关的cookies数据的功能
// 获取到可直接使用的cookies数据
class CookiesManager
{
public:
    CookiesManager();
    ~CookiesManager();

    bool SetCookies(const std::string& key, const std::string& cookie);
    std::string GetCookies(const std::vector<std::string>& keys) const;

private:
    std::map<std::string, std::string> cookies_;
};

