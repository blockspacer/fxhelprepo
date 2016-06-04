#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"

class IpProxy
{
public:
    enum class PROXY_TYPE
    {
        PROXY_TYPE_NONE = 0,
        PROXY_TYPE_HTTP = 1,
        PROXY_TYPE_SOCKS4 = 2,
        PROXY_TYPE_SOCKS4A = 3,
        PROXY_TYPE_SOCKS5 = 4
    };

    IpProxy();
    ~IpProxy();

    void SetProxyType(PROXY_TYPE proxytype);
    PROXY_TYPE GetProxyType() const;

    void SetProxyIp(const std::string& proxyip);
    std::string GetProxyIp() const;

    void SetProxyPort(uint16 proxyport);
    uint16 GetProxyPort() const;

private:
    PROXY_TYPE proxytype_ = PROXY_TYPE::PROXY_TYPE_NONE;
    std::string proxyip_ = "";
    uint16 proxyport_ = 0;
};

