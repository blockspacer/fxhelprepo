#include "CurlWrapper.h"
#include <assert.h>
#include <iostream>
#include "third_party/libcurl/curl/curl.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/time/time.h"

#include "CookiesManager.h"

namespace
{
    const char* fanxingurl = "http://fanxing.kugou.com";
    const char* kugouurl = "http://kugou.com";
    const char* useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
    const char* acceptencode = "gzip, deflate";

    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string data;
        data.assign(ptr, ptr + size*nmemb);
        CurlWrapper* p = static_cast<CurlWrapper*>(userdata);
        p->WriteCallback(data);
        return size*nmemb;
    }
}

bool CurlWrapper::WriteCallback(const std::string& data)
{
    std::cout << data <<std::endl;
    return true;
}

CurlWrapper::CurlWrapper()
{
}

CurlWrapper::~CurlWrapper()
{
}

void CurlWrapper::CurlInit()
{
    curl_global_init(CURL_GLOBAL_WIN32);
}

void CurlWrapper::CurlCleanup()
{
    curl_global_cleanup();
}

// 测试成功
bool CurlWrapper::LoginRequest()
{
    std::string cookies = "";
    bool ret = false;
    if (!CookiesManager::GetCookies(fanxingurl, &cookies))
    {
        return false;
    }

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, fanxingurl);

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers,  "Accept - Language: zh - CN");
    headers = curl_slist_append(headers, "Accept : text / html, application / xhtml + xml, */*");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
    curl_easy_setopt(curl, CURLOPT_REFERER, "www.fanxing.kugou.com");

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "d:/cookie.txt");

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    // 获取服务器时间
    long servertime = 0;
    res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &servertime);

    char* redirecturl = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirecturl);
    if (redirecturl)
    {
        assert(false);
        std::string redirect = redirecturl;
    }

    long headersize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &headersize);

    long requestsize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &requestsize);

    char* localip = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &localip);

    long localport = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &localport);

    // 获取本次请求cookies
    struct curl_slist* curllist = 0;
    res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &curllist);
    if (curllist)
    {
        struct curl_slist* temp = curllist;
        std::string retCookies;
        while (temp)
        {
            retCookies += std::string(temp->data);
            temp = temp->next;
        }
        std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
        curl_slist_free_all(curllist);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200)
    {
        return true;
    }
    return false;
}

// 测试成功
bool CurlWrapper::Services_UserService_UserService_getMyUserDataInfo()
{
    std::string cookies = "";
    bool ret = false;
    if (!CookiesManager::GetCookies(fanxingurl, &cookies))
    {
        return false;
    }

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = R"(http://fanxing.kugou.com/Services.php?act=UserService.UserService&mtd=getMyUserDataInfo&args=[]&_=)";
    url +=  base::IntToString(static_cast<int>(base::Time::Now().ToDoubleT())) + "144";
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Accept: application/json, text/javascript, */*; q=0.01");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");

    
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    curl_easy_setopt(curl, CURLOPT_REFERER, "http://fanxing.kugou.com/");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "d:/cookie.txt");

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);


    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    // 获取服务器时间
    long servertime = 0;
    res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &servertime);

    char* redirecturl = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirecturl);
    if (redirecturl)
    {
        assert(false);
        std::string redirect = redirecturl;
    }

    long headersize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &headersize);

    long requestsize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &requestsize);

    char* localip = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &localip);

    long localport = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &localport);

    // 获取本次请求cookies
    struct curl_slist* curllist = 0;
    res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &curllist);
    if (curllist)
    {
        struct curl_slist* temp = curllist;
        std::string retCookies;
        while (temp)
        {
            retCookies += std::string(temp->data);
            temp = temp->next;
        }
        std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
        curl_slist_free_all(curllist);
    }
    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200)
    {
        return true;
    }
    return false;
}

// 测试成功
bool CurlWrapper::Services_IndexService_IndexService_getUserCenter()
{
    std::string cookies = "";
    bool ret = false;
    if (!CookiesManager::GetCookies(fanxingurl, &cookies))
    {
        return false;
    }

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = R"(http://fanxing.kugou.com/Services.php?act=IndexService.IndexService&mtd=getUserCenter&args=[""]&_=)";
    url += base::IntToString(static_cast<int>(base::Time::Now().ToDoubleT())) + "144";
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Accept: application/json, text/javascript, */*; q=0.01");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");


    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    curl_easy_setopt(curl, CURLOPT_REFERER, "http://fanxing.kugou.com/");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "d:/cookie.txt");

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    // 获取服务器时间
    long servertime = 0;
    res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &servertime);

    char* redirecturl = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirecturl);
    if (redirecturl)
    {
        assert(false);
        std::string redirect = redirecturl;
    }

    long headersize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &headersize);

    long requestsize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &requestsize);

    char* localip = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &localip);

    long localport = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &localport);

    // 获取本次请求cookies
    struct curl_slist* curllist = 0;
    res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &curllist);
    if (curllist)
    {
        struct curl_slist* temp = curllist;
        std::string retCookies;
        while (temp)
        {
            retCookies += std::string(temp->data);
            temp = temp->next;
        }
        std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
        curl_slist_free_all(curllist);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200)
    {
        return true;
    }
    return false;
}

// 测试成功
bool CurlWrapper::EnterRoom(uint32 roomid)
{
    std::string cookies = "";
    std::string temp = "";
    bool ret = false;
    if (!CookiesManager::GetCookies(fanxingurl, &temp))
    {
        return false;
    }
    cookies += temp;
    if (!CookiesManager::GetCookies(kugouurl, &temp))
    {
        return false;
    }
    cookies += ";"+temp;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = std::string(fanxingurl);
    url += "/" + base::IntToString(roomid);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    //headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");


    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    curl_easy_setopt(curl, CURLOPT_REFERER, "http://fanxing.kugou.com/");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "d:/cookie.txt");

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    // 获取服务器时间
    long servertime = 0;
    res = curl_easy_getinfo(curl, CURLINFO_FILETIME, &servertime);

    char* redirecturl = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REDIRECT_URL, &redirecturl);
    if (redirecturl)
    {
        assert(false);
        std::string redirect = redirecturl;
    }

    long headersize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_HEADER_SIZE, &headersize);

    long requestsize = 0;
    res = curl_easy_getinfo(curl, CURLINFO_REQUEST_SIZE, &requestsize);

    char* localip = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_IP, &localip);

    long localport = 0;
    res = curl_easy_getinfo(curl, CURLINFO_LOCAL_PORT, &localport);

    // 获取本次请求cookies
    struct curl_slist* curllist = 0;
    res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &curllist);
    if (curllist)
    {
        struct curl_slist* temp = curllist;
        std::string retCookies;
        while (temp)
        {
            retCookies += std::string(temp->data);
            temp = temp->next;
        }
        std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
        curl_slist_free_all(curllist);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200)
    {
        return true;
    }
    return false;
}
