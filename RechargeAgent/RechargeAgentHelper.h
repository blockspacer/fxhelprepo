#pragma once
#include "afxdialogex.h"
#include <memory>
#include <string>
#include <vector>
#include "third_party/chromium/base/basictypes.h"

class CurlWrapper;
class CookiesHelper;
class RechargeAgentHelper
{
public:
    RechargeAgentHelper();
    ~RechargeAgentHelper();

    bool Initialize();
    void Finalize();

    bool Login(const std::string& account, const std::string& password,
               const std::string& verifycode,
               std::wstring* errorcode);

    bool GetVerifyCode(std::vector<uint8>* picture);
    bool SaveVerifyCodeImage(const std::vector<uint8>& image,
                             std::wstring* path);
private:
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
};

