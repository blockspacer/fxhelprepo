#pragma once
#include <memory>
#include <map>
#include <string>
#include <functional>
#include <vector>
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/files/file.h"
#include "Network/IpProxy.h"

enum class KICK_TYPE
{
    KICK_TYPE_HOUR = 0,
    KICK_TYPE_MONTH = 1,
};

class HttpResponse;

typedef std::function<void(const HttpResponse&)> AsyncHttpResponseCallback;

class HttpRequest
{
public:
    enum class HTTP_METHOD
    {
        HTTP_METHOD_GET = 0,
        HTTP_METHOD_POST = 1,
        HTTP_METHOD_HTTPPOST = 2,
    };
    HTTP_METHOD method;
    std::vector<uint8> postdata;
    std::string url;
    std::string referer;
    std::string cookies;
    std::string useragent;
    std::map<std::string, std::string> queries;
    std::map<std::string, std::string> headers;
    std::string postfile;
    IpProxy ipproxy;

    // 异步回调
    AsyncHttpResponseCallback asyncHttpResponseCallback = nullptr;
};

class HttpResponse
{
public:
    uint32 curlcode;
    uint32 statuscode;
    std::vector<std::string> cookies;
    std::vector<uint8> content;
    std::string server_ip;

};

class CookiesHelper;
// 提供方便的使用curl接口的执行请求函数。
class CurlWrapper
{
public:
    CurlWrapper();
    ~CurlWrapper();
    bool Initialize(){ return true; };//没有线程管理，直接返回true
    void Finalize(){ return; };//没有线程管理，直接返回

    static bool CurlInit();
    static void CurlCleanup();
    bool WriteCallback(const std::string& data);
    bool WriteResponseHeaderCallback(const std::string& data);
    bool Execute(const HttpRequest& request, HttpResponse* reponse);

private:
    std::string currentWriteData_;
    std::string currentResponseHeader_;
    std::string response_of_GiftService_GiftService_;
    std::string response_of_EnterRoom_;

    std::unique_ptr<CookiesHelper> cookiesHelper_;
};