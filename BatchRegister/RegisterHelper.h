#pragma once
#include <memory>
#include <vector>
#include <string>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"

class CurlWrapper;
class CookiesHelper;
class RegisterHelper
{
public:
    RegisterHelper();
    ~RegisterHelper();

    bool Initialize();
    void Finalize();

    bool SaveVerifyCodeImage(
        const std::vector<uint8>& image, std::wstring* path);
    bool SaveAccountToFile(const std::wstring& username,
        const std::wstring& password, const std::string& cookies);
    bool LoadAccountFromFile(
        std::vector<std::pair<std::wstring, std::wstring>>* accountinfo);

    std::wstring GetNewName() const;
    std::wstring GetPassword() const;

    // 注册新号码，网络部分功能
    bool RegisterGetVerifyCode(std::vector<uint8>* picture);

    // 在新版本里，这两个函数都改了，但目前刷起来，不需要这两个功能验证用户名密码
    bool RegisterCheckUserExist(const std::wstring& username);
    bool RegisterCheckPassword(const std::wstring& username, 
        const std::wstring& password);

    bool RegisterUser(const std::wstring& username, const std::wstring& password,
        const std::string& verifycode, std::string* cookies);

private:
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
    std::unique_ptr<base::File> accountFile_;
    std::vector<std::string> namepost;
};

