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

    bool HuyaLogin(const std::wstring& account, const std::wstring& password,
                   std::wstring* errormsg);

private:
    bool HuyaLoginStep1(CurlWrapper* curlWrapper, std::string* url,
                        std::string* ttoken) const;
    bool HuyaLoginStep2(CurlWrapper* curlWrapper, const std::string& url,
                        const std::string& ttoken) const;
    bool HuyaLoginStep3(CurlWrapper* curlWrapper, const std::string& referer,
                        const std::string& ttoken, const std::string& username,
                        const std::string password, std::wstring* errmsg) const;

    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

