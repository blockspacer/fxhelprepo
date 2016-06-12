#include "CurlWrapper.h"
#include <memory>
#include <assert.h>
#include <iostream>
#include "third_party/libcurl/curl/curl.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/rand_util.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_path.h"

#include "Network/CookiesHelper.h"
#include "Network/EncodeHelper.h"

namespace
{
    const char* fanxingurl = "http://fanxing.kugou.com";
    
    const char* kugouurl = "http://kugou.com";
    const char* useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
    const char* acceptencode = "gzip";//目前都不应该接收压缩数据，免得解压麻烦

    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string data;
        data.assign(ptr, ptr + size*nmemb);
        CurlWrapper* p = static_cast<CurlWrapper*>(userdata);
        p->WriteCallback(data);
        return size*nmemb;
    }

    static size_t header_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string data;
        data.assign(ptr, ptr + size*nmemb);
        CurlWrapper* p = static_cast<CurlWrapper*>(userdata);
        p->WriteResponseHeaderCallback(data);
        return size*nmemb;
    }

    std::string MakeReasonablePath(const std::string& pathfile)
    {
        auto temp = pathfile;
        auto pos = temp.find(':');
        while (pos != std::string::npos)
        {
            temp.erase(pos,1);
            pos = temp.find(':');
        }
        return std::move(temp);
    }

    bool GetCookiesFromHeader(const std::string& header, std::vector<std::string>* cookies)
    {
        std::vector<std::string> splitestrings = SplitString(header, "\r\n");

        std::string setcookietag = "Set-Cookie:";
        for (const auto& it : splitestrings)
        {
            auto pos = it.find("Set-Cookie:");
            if (pos!=std::string::npos)
            {
                std::string temp = it.substr(setcookietag.size(), 
                    it.size() - setcookietag.size());
                // 去除多余的空格
                RemoveSpace(&temp);
                auto splitstr = SplitString(temp, ";");
                cookies->insert(cookies->end(), splitstr.begin(), splitstr.end());
            }
        }
        return true;
    }
}

bool CurlWrapper::WriteCallback(const std::string& data)
{
    currentWriteData_ += data;
    return true;
}

bool CurlWrapper::WriteResponseHeaderCallback(const std::string& data)
{
    currentResponseHeader_ += data;
    return true;
}


CurlWrapper::CurlWrapper()
    :currentWriteData_("")
    , cookiesHelper_(new CookiesHelper)
{
}

CurlWrapper::~CurlWrapper()
{
}

bool CurlWrapper::CurlInit()
{
    return CURLE_OK == curl_global_init(CURL_GLOBAL_ALL);
}

void CurlWrapper::CurlCleanup()
{
    curl_global_cleanup();
}

bool CurlWrapper::Execute(const HttpRequest& request, HttpResponse* response)
{
    LOG(INFO) << __FUNCTION__;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;
    std::string url = request.url;
    if (!request.queries.empty())
    {
        bool first = true;
        for (const auto& it : request.queries)
        {
            url += first ? "?" : "&";
            first = false;
            url += it.first + "=" + it.second;
        }
    }

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    if (url.find("https")!=std::string::npos)
    {
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    }

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Connection:Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Accept:*/*");
    for (const auto& it : request.headers)
    {
        std::string header = it.first + ":" + it.second;
        headers = curl_slist_append(headers, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);

    if (!request.useragent.empty())
        curl_easy_setopt(curl, CURLOPT_USERAGENT, request.useragent.c_str());
    else
        curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
    
    if (!request.referer.empty())
    {
        curl_easy_setopt(curl, CURLOPT_REFERER, request.referer.c_str());
    }
    
    if (!request.cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, request.cookies.c_str());
    }

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    struct curl_httppost* post = nullptr;
    struct curl_httppost* last = nullptr;

    switch (request.method)
    {
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_GET:
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        break;
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_POST:       
        assert(!request.postdata.empty());
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.postdata.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, &request.postdata[0]);
        break;
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_HTTPPOST:
        assert(!request.postfile.empty());
        if (!request.postfile.empty())
        {
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "file",
                         CURLFORM_FILE, request.postfile.c_str(), CURLFORM_END);
            curl_easy_setopt(curl, CURLOPT_HTTPPOST, post);
        }
        break;
    default:
        assert(false);
        break;
    }
    
    // 设置代理
    if (request.ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        uint32 proxytype = CURLPROXY_HTTP;
        switch (request.ipproxy.GetProxyType())
        {
        case IpProxy::PROXY_TYPE::PROXY_TYPE_HTTP:
            proxytype = CURLPROXY_HTTP;
            break;
        case IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS4:
            proxytype = CURLPROXY_SOCKS4;
            break;
        case IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS4A:
            proxytype = CURLPROXY_SOCKS4A;
            break;
        case IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS5:
            proxytype = CURLPROXY_SOCKS5;
            break;
        default:
            assert(false && L"参数错误");
            break;
        }
        std::string proxyip = request.ipproxy.GetProxyIp();
        uint16 proxyport = request.ipproxy.GetProxyPort();

        curl_easy_setopt(curl, CURLOPT_PROXYTYPE, proxytype);
        curl_easy_setopt(curl, CURLOPT_PROXY, proxyip.c_str());
        curl_easy_setopt(curl, CURLOPT_PROXYPORT, proxyport);
    }

    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 12L);

    res = curl_easy_perform(curl);
    response->curlcode = res;
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        LOG(ERROR) << curl_easy_strerror(res);       
        return false;
    }

    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);
    response->statuscode = responsecode;
    if (responsecode != 200)
    {
        fprintf(stderr, "reponsecode: %ld\n", responsecode);
        return false;
    }

    response->content.assign(currentWriteData_.begin(), currentWriteData_.end());
    std::vector<std::string> setcookies;
    if (GetCookiesFromHeader(currentResponseHeader_, &setcookies))
    {
        response->cookies = setcookies;
    }

    return true;
}

