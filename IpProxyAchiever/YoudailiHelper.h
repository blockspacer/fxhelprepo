#pragma once
#include <memory>
#include <vector>
#include "Network/IpProxy.h"

class CurlWrapper;
class YoudailiHelper
{
public:
    YoudailiHelper();
    ~YoudailiHelper();

    bool GetAllProxyInfo(std::vector<IpProxy>* ipproxys);
    bool LoadIpProxy(std::vector<IpProxy>* proxyinfo);
    bool SaveIpProxy(const std::vector<IpProxy>& ipproxys);
    bool ParseWebFile(const std::wstring& filepath, std::vector<IpProxy>* ipproxy);

private:
    bool GetPageUrls(std::vector<std::string>* urls);
    bool GetDailyProxys(const std::string& url, std::vector<IpProxy>* ipproxy);
    bool ParseDailyProxys(const std::string& webdata, std::vector<IpProxy>* ipproxy);
    std::unique_ptr<CurlWrapper> curlWrapper_;
};

