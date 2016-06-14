#pragma once
#include <memory>
#include <vector>
#include <string>
#include <functional>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/threading/thread.h"

#include "Network/IpProxy.h"

class CurlWrapper;
class CookiesHelper;
class RegisterHelper
{
public:
    RegisterHelper();
    ~RegisterHelper();

    static void AddRef() {}
    static void Release() {}

    bool Initialize();
    void Finalize();

    bool SaveVerifyCodeImage(
        const std::vector<uint8>& image, std::wstring* path);
    bool SaveAccountToFile(const std::wstring& username,
        const std::wstring& password, const std::string& cookies);
    bool LoadAccountFromFile(
        std::vector<std::pair<std::wstring, std::wstring>>* accountinfo);
    bool LoadIpProxy(std::vector<IpProxy>* ipproxys);
    bool SaveIpProxy(const std::vector<IpProxy>& ipproxys);

    std::wstring GetNewName() const;
    std::wstring GetPassword() const;

    // 注册新号码，网络部分功能
    bool RegisterGetVerifyCode(
        const IpProxy& ipproxy, std::vector<uint8>* picture);

    bool RegisterGetVerifyCodeByEmail(
        const IpProxy& ipproxy, std::vector<uint8>* picture);

    // 在新版本里，这两个函数都改了，但目前刷起来，不需要这两个功能验证用户名密码
    bool RegisterCheckUserExist(
        const IpProxy& ipproxy, const std::wstring& username);
    bool RegisterCheckPassword(
        const IpProxy& ipproxy, const std::wstring& username,
        const std::wstring& password);

    void DoRegisterUser(
        const IpProxy& ipproxy,
        const std::wstring& username, const std::wstring& password,
        const std::string& verifycode,
        std::function<void(const std::wstring& message)> callback);

    bool AsyncRegisterUser(
        const IpProxy& ipproxy,
        const std::wstring& username, const std::wstring& password,
        const std::string& verifycode,
        std::function<void(const std::wstring& message)> callback);

    bool RegisterUserByEmail(
        const IpProxy& ipproxy,
        const std::wstring& email, const std::wstring& password,
        const std::string& verifycode, std::string* cookies, std::wstring* errormsg);

    bool AsyncCheckIpProxy(const std::vector<IpProxy>& ipproxys,
                           std::function<void(bool, const IpProxy&, const std::wstring&)> callback);

    void DoCheckIpProxy(const IpProxy& ipproxy,
                        std::function<void(bool, const IpProxy&, const std::wstring&)> callback);

private:
    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::unique_ptr<CookiesHelper> cookiesHelper_;
    bool cancelflag_ = false;
    base::Thread workerThread_;
    std::unique_ptr<base::File> accountFile_;
    std::vector<std::string> namepost;
};

