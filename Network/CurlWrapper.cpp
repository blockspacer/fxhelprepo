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

    std::vector<std::string> SplitString(std::string str, const std::string& pattern)
    {
        std::vector<std::string> result;
        str += pattern;//扩展字符串以方便操作
        int size = str.size();

        for (int i = 0; i < size; i++)
        {
            int pos = str.find(pattern, i);
            if (pos < size)
            {
                std::string s = str.substr(i, pos - i);
                result.push_back(s);
                i = pos + pattern.size() - 1;
            }
        }
        return result;
    }

    void RemoveSpace(std::string* str)
    {
        auto pos = str->find(' ');
        while (pos!=std::string::npos)
        {
            str->erase(pos,1);
            pos = str->find(' ');
        }
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

    bool ExtraSingerIdFrom_EnterRoom_Response_(const std::string& inputstr,
                                             uint32* starId)
    {
        auto pos = inputstr.find("isClanRoom");
        if (pos == std::string::npos)
            return false;

        pos = inputstr.find("starId", pos);
        if (pos == std::string::npos)
            return false;

        pos = inputstr.find("\"", pos);
        if (pos == std::string::npos)
            return false;

        auto begin = pos + 1;
        auto end = inputstr.find("\"", begin);
        if (pos == std::string::npos)
            return false;
        std::string temp = inputstr.substr(begin, end - begin);
        if (!base::StringToUint(temp, starId))
        {
            return false;
        }
        return true;
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

// 测试成功
bool CurlWrapper::EnterRoom(uint32 roomid, uint32* singerid)
{
    std::string cookies = "";
    bool ret = false;
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fx_user");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    cookies = cookiesHelper_->GetCookies(keys);

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
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");


    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    curl_easy_setopt(curl, CURLOPT_REFERER, "http://fanxing.kugou.com/");
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    if (!cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

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

    response_of_EnterRoom_ = currentWriteData_;

    bool result = ExtraSingerIdFrom_EnterRoom_Response_(response_of_EnterRoom_, singerid);
    assert(*singerid);
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
        //std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
        curl_slist_free_all(curllist);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200 && result && *singerid)
    {
        return true;
    }
    return false;
}

bool CurlWrapper::Servies_Uservice_UserService_getCurrentUserInfo(uint32 roomid,
    uint32* userid, std::string* nickname, uint32* richlevel)
{
	return false;
}

bool CurlWrapper::GiftService_GiftService(uint32 roomid,
    const std::string& key_601, std::wstring* responsedata)
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookies = cookiesHelper_->GetCookies(keys);
    bool ret = false;


    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = fanxingurl;
    url += "/Services.php?act=GiftService%2EGiftService&args=%5B%22";
    url += base::UintToString(roomid);
    url += "%22%2C%22";
    url += key_601;
    url += "%22%5D&mtd=tryGetHappyFreeCoin&ran=";
    // 生成一个小数，小数16位，方式比较丑陋，实现就好
    uint32 first = base::RandInt(10000000, 99999999);
    uint32 second = base::RandInt(10000000, 99999999);
    url += "0%2E" + base::UintToString(first) + base::UintToString(second);

    LOG(INFO) << __FUNCTION__ << L" url = " << url;
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    //headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");
    //headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: */*");
    headers = curl_slist_append(headers, "x-flash-version: 12,0,0,77");
    headers = curl_slist_append(headers, "application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/static/swf/award/CommonMoneyGift.swf?version=20141113";
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
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
    response_of_GiftService_GiftService_ = currentWriteData_;

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
        //std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
        curl_slist_free_all(curllist);
    }

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200)
    {
        // 解析返回的数据通知
        //response_of_GiftService_GiftService_;
        std::wstring notifyinfo;
        ParseGiftServiceResponse(response_of_GiftService_GiftService_,
            &notifyinfo);
        *responsedata = L"抢币数据通知";
        return true;
    }
    return false;
}

//GET /Services.php?act=RoomService.RoomManageService&mtd=kickOut&d=1457847702101&args=["110468466","120831944","1053637",3600,"星星点点oo",0] HTTP/1.1
bool CurlWrapper::KickoutUser(uint32 singerid,
    KICK_TYPE kicktype, const EnterRoomUserInfo& enterRoomUserInfo)
{
    LOG(INFO) << __FUNCTION__ << L" singerid = " << base::UintToString(singerid);

    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string strroomid = base::IntToString(static_cast<int>(enterRoomUserInfo.roomid));
    std::string url = std::string(fanxingurl);
    url += "/Services.php?act=RoomService.RoomManageService&mtd=kickOut&d=";
    url += GetNowTimeString();
    url += R"(&args=)";
    std::string jsonstr;
    jsonstr += std::string(R"([")");
    jsonstr += base::UintToString(singerid);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.userid);
    jsonstr += R"(",")";
    jsonstr += base::UintToString(enterRoomUserInfo.roomid);
    jsonstr += R"(",3600,")";
    jsonstr += enterRoomUserInfo.nickname;
    if (kicktype == KICK_TYPE::KICK_TYPE_HOUR)
    {
        jsonstr += R"(",0])";
    }
    else
    {
        jsonstr += R"(",0,2])";
    }
    jsonstr = UrlEncode(jsonstr);

    url += jsonstr;

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: application/json;text/javascript,*/*;q=0.01");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/" + strroomid;
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        LOG(INFO) << __FUNCTION__ << L" curl_easy_perform failed";
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    LOG(INFO) << __FUNCTION__ << L" responsecode = " << base::IntToString(responsecode);

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
        //std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
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

// GET /VServices/GiftService.GiftService.getGiftList/1014619/
bool CurlWrapper::GetGiftList(uint32 roomid, std::string* outputstr)
{
    LOG(INFO) << __FUNCTION__ << L" roomid = " << base::UintToString(roomid);

    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    keys.push_back("_fx_coin");
    keys.push_back("_fxNickName");
    keys.push_back("_fxRichLevel");
    keys.push_back("FANXING_COIN");
    keys.push_back("FANXING");
    keys.push_back("fxClientInfo");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string strroomid = base::IntToString(static_cast<int>(roomid));
    std::string url = std::string(fanxingurl);
    url += "/VServices/GiftService.GiftService.getGiftList/";
    url += strroomid+"/";

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: visitor.fanxing.kugou.com");//这个值与别的不一样
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: application/json;text/javascript,*/*;q=0.01");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/" + strroomid;
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    if (!cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        LOG(INFO) << __FUNCTION__ << L" curl_easy_perform failed";
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    *outputstr = currentWriteData_;

    LOG(INFO) << __FUNCTION__ << L" responsecode = " << base::IntToString(responsecode);

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
        //std::cout << "CURLINFO_COOKIELIST get cookie: " << retCookies;
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

bool CurlWrapper::ParseGiftServiceResponse(const std::string& responsedata,
    std::wstring* notifyinfo)
{
    return false;
}


bool CurlWrapper::SetCookieFromString(const std::string& key, const std::string& cookiestring)
{
    std::string tempkey = key + "=";
    auto pos = cookiestring.find(tempkey);
    if (pos == std::string::npos)
    {
        return false;
    }
    auto begin = pos;
    auto end = cookiestring.find(';', begin);
    bool result = cookiesHelper_->SetCookies(key, cookiestring.substr(begin, end - begin));
    return result;
}

// 请求注册的验证码，返回函数会设置Set-Cookie: RegCheckCode
//GET http://verifycode.service.kugou.com/v1/get_img_code?type=RegCheckCode&appid=1010&codetype=1&t=1458830896851 HTTP/1.1
//Host: verifycode.service.kugou.com
//Connection: keep-alive
//Accept: image/webp,*/*;q=0.8
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://fanxing.kugou.com/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
bool CurlWrapper::RegisterGetVerifyCode(std::vector<uint8>* picture)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = "http://verifycode.service.kugou.com";
    url += "/v1/get_img_code?type=RegCheckCode&appid=1010&codetype=1&t=";
    url += GetNowTimeString();

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: verifycode.service.kugou.com");//这个值与别的不一样
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: image/webp,*/*;q=0.8"); // 接收图片
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/";
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        LOG(INFO) << __FUNCTION__ << L" curl_easy_perform failed";
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    auto pos = currentResponseHeader_.find("RegCheckCode");
    if (pos != std::string::npos)
    {
        auto begin = pos;
        auto end = currentResponseHeader_.find(';', begin);
        if (end != std::string::npos)
        {
            cookiesHelper_->SetCookies("RegCheckCode", currentResponseHeader_.substr(begin, end - begin));
        }
    }

    picture->assign(currentWriteData_.begin(), currentWriteData_.end());

    LOG(INFO) << __FUNCTION__ << L" responsecode = " << base::IntToString(responsecode);
    if (responsecode != 200)
    {
        return false;
    }
    return true;
}

//GET http://fanxing.kugou.com/UServices/UserService/UserService/isExists?args=[%22fanxingtest007%22,1]&_=1458830905099 HTTP/1.1
//Host: fanxing.kugou.com
//Connection: keep-alive
//Accept: application/json, text/javascript, */*; q=0.01
//X-Requested-With: XMLHttpRequest
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://fanxing.kugou.com/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: RegCheckCode=599f85ea9acb57d2c50492130330f782
// 检查用户是否已经存在，未确认不请求会不会有问题
bool CurlWrapper::RegisterCheckUserExist(const std::string& username)
{
    LOG(INFO) << __FUNCTION__ << L" username = " << username;

    std::vector<std::string> keys;
    keys.push_back("RegCheckCode");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = std::string(fanxingurl);
    url += "/UServices/UserService/UserService/isExists?args=[%22";
    url += username + "%22,1]&_=";
    url += GetNowTimeString();

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");//这个值与别的不一样
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: application/json;text/javascript,*/*;q=0.01");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/";
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    if (!cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        LOG(INFO) << __FUNCTION__ << L" curl_easy_perform failed";
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);
    LOG(INFO) << __FUNCTION__ << L" responsecode = " << base::IntToString(responsecode);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode != 200)
    {
        return false;
    }

    const std::string& data = currentWriteData_;
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

    // 有必要检测status的值
    uint32 status = rootdata.get(std::string("status"), 0).asInt();
    if (status == 0)
    {
        return false;
    }

    return true;
}

// 检查用户和密码有效性，未确认不请求会不会有问题
bool CurlWrapper::RegisterCheckUserInfo(const std::string& username, 
    const std::string& password)
{
    std::string pwdMd5 = MakeMd5FromString(password);

    LOG(INFO) << __FUNCTION__ << L" username = " << username;

    std::vector<std::string> keys;
    keys.push_back("RegCheckCode");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = "http://userinfo.user.kugou.com";
    url += "/check_str/?appid=1010&str=" + pwdMd5;
    url += "&username=" + username + "&callback=checkPwdSuccessCallback";

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    //headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: userinfo.user.kugou.com");//这个值与别的不一样
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: */*");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/";
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    if (!cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        LOG(INFO) << __FUNCTION__ << L" curl_easy_perform failed";
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);
    LOG(INFO) << __FUNCTION__ << L" responsecode = " << base::IntToString(responsecode);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode != 200)
    {
        return false;
    }

    std::string& data = currentWriteData_;
    std::string checkPwdSuccessCallback = "checkPwdSuccessCallback";
    auto pos = data.find("checkPwdSuccessCallback");
    if (pos == std::string::npos)
    {
        return false;
    }

    // checkPwdSuccessCallback({"status":1,"error_code":0,"data":0})
    // 取出json数据
    data = data.substr(checkPwdSuccessCallback.length() + 1, 
        data.length()-checkPwdSuccessCallback.length()-2);

    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(data, rootdata, false))
    {
        return false;
    }

    // 有必要检测status的值
    uint32 status = rootdata.get(std::string("status"), 0).asInt();
    if (status == 0)
    {
        return false;
    }

    return true;
}

//GET http://verifycode.service.kugou.com/v1/check_img_code/?appid=1010&ct=1458830934&code=5987&type=RegCheckCode&callback=KgUser.CheckSudoCode HTTP/1.1
//Host: verifycode.service.kugou.com
//Connection: keep-alive
//Accept: */*
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://fanxing.kugou.com/
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: RegCheckCode=599f85ea9acb57d2c50492130330f782
// 提交验证码数据
bool CurlWrapper::RegisterCheckVerifyCode(const std::string& verifycode)
{
    LOG(INFO) << __FUNCTION__ << L" verifycode = " << verifycode;

    std::vector<std::string> keys;
    keys.push_back("RegCheckCode");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = "http://verifycode.service.kugou.com";
    url += "/v1/check_img_code/?appid=1010&ct=" + GetNowTimeString();
    url += "&code=" + verifycode + "&type=RegCheckCode&callback=KgUser.CheckSudoCode";

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: verifycode.service.kugou.com");//这个值与别的不一样
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: */*");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/";
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    if (!cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        LOG(INFO) << __FUNCTION__ << L" curl_easy_perform failed";
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);
    LOG(INFO) << __FUNCTION__ << L" responsecode = " << base::IntToString(responsecode);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode != 200)
    {
        return false;
    }

    std::string& data = currentWriteData_;
    std::string CheckSudoCode = R"(KgUser.CheckSudoCode({"msg":"success"}))";
    auto pos = data.find(CheckSudoCode);
    if (pos == std::string::npos)
    {
        std::string pre = "KgUser.CheckSudoCode(";
        std::string json = data.substr(pre.length(), data.length() - pre.length() - 1);
        Json::Reader reader;
        Json::Value rootdata(Json::objectValue);
        if (!reader.parse(json, rootdata, false))
        {
            return false;
        }

        // 有必要检测msg的值
        std::string msg = rootdata.get(std::string("errorMsg"), "").asString();
        std::string utf8msg;
        UnicodeToUtf8(msg, &utf8msg);
        return false;
    }
    return true;
}

// 提交注册功能
bool CurlWrapper::RegisterUser(const std::string& username,
    const std::string& password,
    const std::string& verifycode)
{
    LOG(INFO) << __FUNCTION__ << L" verifycode = " << verifycode;

    std::vector<std::string> keys;
    keys.push_back("RegCheckCode");
    std::string cookies = cookiesHelper_->GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;
    std::string url = "https://reg-user.kugou.com";
    url += "/v2/reg/?regtype=username&appid=1010&code="+verifycode;
    url += "&expire_day=1&username=" + UrlEncode(username);
    url += "&sex=1&password=" + MakeMd5FromString(password);
    url += "&nickname=" + UrlEncode(username);
    url += "&security_email=&id_card=&truename=&callback=regSuccessCallback&codetype=1";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Connection:Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Accept:*/*");
    headers = curl_slist_append(headers, "Host:reg-user.kugou.com");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
    curl_easy_setopt(curl, CURLOPT_REFERER, fanxingurl);
    if (!cookies.empty())
    {
        curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    }

    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

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
    if (responsecode!=200)
    {
        return false;
    }

    std::string regSuccessCallback = "regSuccessCallback";
    std::string data = currentWriteData_;
    auto pos = data.find(regSuccessCallback);
    if (pos == std::string::npos)
        return false;

    data = data.substr(regSuccessCallback.length() + 1, 
        data.length() - regSuccessCallback.length() - 3);
    
    std::string json = data;
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(json, rootdata, false))
    {
        return false;
    }
    Json::Value::Members members = rootdata.getMemberNames();
    for (auto member : members)
    {
        if (member == "errorCode")
        {
            LOG(INFO) << __FUNCTION__ << L"注册失败";
            return false;
        }   
    }

    return true;
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

    switch (request.method)
    {
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_GET:
        curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
        break;
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_POST:
        curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1L);
        assert(false && L"未实现post数据功能");
        break;
    default:
        assert(false);
        break;
    }
    
    // 设置代理
    //curl_easy_setopt(curl, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
    //curl_easy_setopt(curl, CURLOPT_PROXY, "127.0.0.1");
    //curl_easy_setopt(curl, CURLOPT_PROXYPORT, 8888);

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

