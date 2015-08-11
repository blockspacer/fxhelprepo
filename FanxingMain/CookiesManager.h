#pragma once
#include <string>

// CookiesManager提供获取本地fanxing的http请求相关的cookies数据的功能
// 获取到可直接使用的cookies数据
class CookiesManager
{
public:
    CookiesManager();
    ~CookiesManager();

    static bool GetCookies(std::string* cookies);

};

