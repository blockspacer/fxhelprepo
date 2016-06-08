#include "stdafx.h"
#include <regex>
#include <fstream>
#include "YoudailiHelper.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"

#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

YoudailiHelper::YoudailiHelper()
{
    curlWrapper_.reset(new CurlWrapper);
}


YoudailiHelper::~YoudailiHelper()
{
}

bool YoudailiHelper::GetAllProxyInfo(std::vector<IpProxy>* ipproxys)
{
    std::vector<std::string> urls;
    if (!GetPageUrls(&urls))
        return false;

    for (const auto& url : urls)
    {
        GetDailyProxys(url, ipproxys);
    }
    return true;
}

bool YoudailiHelper::GetPageUrls(std::vector<std::string>* urls)
{
    HttpRequest request;
    request.url = "http://www.youdaili.net/Daili/Socks/";
    //request.url = "http://fanxing.kugou.com/";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    // 这个网站有点特殊，需要用这种useragent才能正常访问
    request.useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
    request.referer = "http://www.youdaili.net/Daili/Socks/";
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
        return false;

    std::string rootdata(response.content.begin(), response.content.end());
    std::regex pattern(R"(http://www\.youdaili.net/Daili/Socks/[0-9]*\.html)");

    std::string s = rootdata;
    std::smatch match;
    while (std::regex_search(s, match, pattern))
    {
        for (auto x : match)
            urls->push_back(x);
        s = match.suffix().str();
    }
    return true;
}

bool YoudailiHelper::GetDailyProxys(const std::string& url, 
                                    std::vector<IpProxy>* ipproxy)
{
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    // 这个网站有点特殊，需要用这种useragent才能正常访问
    request.useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
    request.referer = "http://www.youdaili.net/Daili/Socks/";
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
        return false;

    std::string rootdata(response.content.begin(), response.content.end());
    std::regex pattern(R"((?:(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9]):[0-9]*@SOCKS\d)");

    std::string s = rootdata;
    std::smatch match;
    while (std::regex_search(s, match, pattern))
    {
        for (auto x : match)
        {
            IpProxy proxy;
            std::string str = x;
            size_t beginpos = 0;
            size_t endpos = str.find(":");
            if (beginpos == std::string::npos)
                continue;
            proxy.SetProxyIp(str.substr(beginpos, endpos - beginpos));
            beginpos = endpos;
            endpos = str.find("@");
            uint32 port = 0;
            base::StringToUint(str.substr(beginpos+1, endpos-beginpos), &port);
            proxy.SetProxyPort(static_cast<uint16>(port));
            beginpos = endpos;
            std::string sockstype = str.substr(beginpos+1);
            if (sockstype.compare("SOCKS5") == 0)
            {
                proxy.SetProxyType(IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS5);
            }
            else if (sockstype.compare("SOCKS4") == 0)
            {
                proxy.SetProxyType(IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS4);
            }
            else
            {
                assert(false && L"不正确的代理类型");
            }
            ipproxy->push_back(proxy);
        }
        s = match.suffix().str();
    }
    return true;
}

bool YoudailiHelper::LoadIpProxy(std::vector<IpProxy>* proxyinfo)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"Proxy.txt";
    base::FilePath pathname = dirPath.Append(filename);

    std::ifstream ovrifs;
    ovrifs.open(pathname.value());
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;

    std::string data = ss.str();
    try
    {
        Json::Reader reader;
        Json::Value root(Json::objectValue);
        if (!Json::Reader().parse(data.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isArray())
        {
            assert(false && L"root is not array");
            return false;
        }

        for (const auto& value : root)//for data
        {
            Json::Value temp;
            uint32 proxytype = GetInt32FromJsonValue(value, "proxytype");
            uint32 proxyport = GetInt32FromJsonValue(value, "port");
            std::string proxyip = value.get("ip", "").asString();
            IpProxy proxy;
            proxy.SetProxyType(static_cast<IpProxy::PROXY_TYPE>(proxytype));
            proxy.SetProxyIp(proxyip);
            proxy.SetProxyPort(proxyport);
            proxyinfo->push_back(proxy);

            //RowData row;
            //row.push_back(base::UintToString16(proxytype));
            //row.push_back(base::UTF8ToWide(proxyip));
            //row.push_back(base::UintToString16(proxyport));
            //proxyinfo->push_back(row);
        }
    }
    catch (...)
    {
        assert(false && L"读取错误");
        return false;
    }
    return true;
}

bool YoudailiHelper::SaveIpProxy(const std::vector<IpProxy>& ipproxys)
{

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"Proxy.txt";
    base::FilePath pathname = dirPath.Append(filename);

    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& ipproxy : ipproxys)
    {
        Json::Value jvProxyInfo(Json::objectValue);
        jvProxyInfo["proxytype"] = static_cast<uint32>(ipproxy.GetProxyType());
        jvProxyInfo["ip"] = ipproxy.GetProxyIp();
        jvProxyInfo["port"] = ipproxy.GetProxyPort();
        root.append(jvProxyInfo);
    }

    std::string writestring = writer.write(root);
    std::ofstream ofs(pathname.value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << writestring;
    ofs.flush();
    ofs.close();

    return true;
}

