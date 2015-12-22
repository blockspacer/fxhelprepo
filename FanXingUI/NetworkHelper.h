#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>

class CurlWrapper;
class GiftNotifyManager;
typedef std::function<void(const std::wstring&)> notifyfn;
class NetworkHelper
{
public:
    NetworkHelper();
    ~NetworkHelper();

    bool Initialize();// 启动线程
    void Finalize();// 结束线程

    void SetNotify(notifyfn fn);
    void RemoveNotify();
    bool EnterRoom(const std::wstring& strroomid);
    bool Login(const std::wstring& username, const std::wstring& password);
private:
    void NotifyCallback(const std::wstring& message);
    void NotifyCallback601(const std::string& data);

    std::unique_ptr<CurlWrapper> curlWrapper_;
    std::unique_ptr<GiftNotifyManager> giftNotifyManager_;
    notifyfn notify_;
};

