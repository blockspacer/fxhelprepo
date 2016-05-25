#pragma once
#include <memory>
#include <string>

class CurlWrapper;
class CookiesHelper;
class HuyaLogic
{
public:
    HuyaLogic();
    ~HuyaLogic();

    bool HuyaLoginStep1(CurlWrapper* curlWrapper, std::string* url, std::string* ttoken);
    bool HuyaLoginStep2(CurlWrapper* curlWrapper, const std::string& url, 
                        const std::string& ttoken);
    bool HuyaLoginStep3(CurlWrapper* curlWrapper, const std::string& referer,
                        const std::string& ttoken, const std::string& username,
                        const std::string password);

    bool HuyaLoginTest();

private:
    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

