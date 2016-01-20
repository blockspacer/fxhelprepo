#include "FamilyDaily.h"

#include <memory>
#include <assert.h>
#include <iostream>
#include "Network/EncodeHelper.h"
#include "third_party/libcurl/curl/curl.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/rand_util.h"
#include "third_party/json/json.h"


namespace{
    const char* familyurl = "http://family.fanxing.kugou.com";

    // cookies 
    const char* cookie_connection = "Connection:Keep-Alive";
    const char* cookie_host = "Host: family.fanxing.kugou.com";
    const char* cookie_accept = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    const char* cookie_upgrade = "Upgrade-Insecure-Requests: 1";
    const char* cookie_useragent = "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36";
    const char* cookie_language = "Accept-Language: zh-CN,zh;q=0.8";
    const char* cookie_content = "Content-Type: application/x-www-form-urlencoded";

    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string data;
        data.assign(ptr, ptr + size*nmemb);
        FamilyDaily* p = static_cast<FamilyDaily*>(userdata);
        p->WriteCallback(data);
        return size*nmemb;
    }
}

FamilyDaily::FamilyDaily()
{
}


FamilyDaily::~FamilyDaily()
{
}

void FamilyDaily::CurlInit()
{
    curl_global_init(CURL_GLOBAL_WIN32);
}
void FamilyDaily::CurlCleanup()
{
    curl_global_cleanup();
}

bool FamilyDaily::WriteCallback(const std::string& data)
{
    currentWriteData_ += data;
    return true;
}

//GET /admin?act=login HTTP/1.1
//Host: family.fanxing.kugou.com
//Connection: keep-alive
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
bool FamilyDaily::Init()
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    assert(result);
    base::FilePath cookiespath = dirPath.Append(L"family.kugou.com.cookies.txt");
    cookiespath_ = base::WideToUTF8(cookiespath.value());
    assert(!cookiespath_.empty());

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = std::string(familyurl) + "/admin?act=login";
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, cookie_host);
    headers = curl_slist_append(headers, cookie_connection);
    headers = curl_slist_append(headers, cookie_language);
    headers = curl_slist_append(headers, cookie_accept);
    headers = curl_slist_append(headers, cookie_upgrade);
    headers = curl_slist_append(headers, cookie_useragent);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, cookiespath_.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);


    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }

    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    if (responsecode == 200)
    {
        return true;
    }
    return false;
}
// http://family.fanxing.kugou.com/admin?act=login
//POST /admin?act=login HTTP/1.1
//Host: family.fanxing.kugou.com
//Connection: keep-alive
//Content-Length: 47
//Cache-Control: max-age=0
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Origin: http://family.fanxing.kugou.com
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Content-Type: application/x-www-form-urlencoded
//Referer: http://family.fanxing.kugou.com/admin?act=login
//Accept-Encoding: gzip, deflate
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: PHPSESSID=lqbq66gtuv6rof4vb324112m94; f_p=f_569f993bd3f2c5.14993273; _family_login_time=1
bool FamilyDaily::Login(const std::string& username, const std::string& password)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, familyurl);

    // 这里如果跟下去,就无法判断目前结果
    /* example.com is redirected, so we tell libcurl to follow redirection */
    //curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, cookie_connection);
    headers = curl_slist_append(headers, "Cache-Control: max-age=0");
    headers = curl_slist_append(headers, cookie_language);
    headers = curl_slist_append(headers, "Origin: http://family.fanxing.kugou.com");
    headers = curl_slist_append(headers, cookie_accept);
    headers = curl_slist_append(headers, cookie_upgrade);
    headers = curl_slist_append(headers, cookie_useragent);
    headers = curl_slist_append(headers, cookie_content);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_REFERER, "http://family.fanxing.kugou.com/admin?act=login");

    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiespath_.c_str());

    // 设置post数据
    curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);

    std::string encodeusername = UrlEncode(username);
    std::string encodepassword = UrlEncode(password);
    std::string postFields = "adminName=" + encodeusername + "&password=" + encodepassword;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 302)
    {
        return true;
    }

    if (responsecode == 200 && currentWriteData_ =="0")
    {
        return true;
    }
    return false;
}

bool FamilyDaily::GetDailyDataBySingerId(uint32 singerid,
    std::vector<SingerDailyData> dailydata)
{
    return false;
}

bool FamilyDaily::GetSummaryData(const base::Time& begintime, const base::Time& endtime,
    std::vector<SingerSummaryData>* summerydata)
{
    return false;
}
