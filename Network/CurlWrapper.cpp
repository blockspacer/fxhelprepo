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


#include "../Network/EncodeHelper.h"
#include "CookiesManager.h"

namespace
{
    const char* fanxingurl = "http://fanxing.kugou.com";
    const char* loginuserurl = "https://login-user.kugou.com";
    const char* kugouurl = "http://kugou.com";
    const char* useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
    const char* acceptencode = "deflate";//目前都不应该接收压缩数据，免得解压麻烦

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

    // 获取13位的当前时间字符串
    std::string GetNowTimeString()
    {
        return base::Uint64ToString(
            static_cast<uint64>(base::Time::Now().ToDoubleT() * 1000));
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

    bool ExtractUsefulInfo_RoomService_enterRoom_(const std::string&inputstr,
        uint32* userid, std::string* nickname, uint32* richlevel)
    {
        if (inputstr.empty())
        {
            return false;
        }

        const std::string& data = inputstr;
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

        Json::Value dataObject(Json::objectValue);
        dataObject = rootdata.get(std::string("data"), dataObject);
        if (dataObject.empty())
        {
            return false;
        }

        Json::Value fxUserInfoObject(Json::objectValue);
        fxUserInfoObject = dataObject.get("fxUserInfo", fxUserInfoObject);
        if (fxUserInfoObject.empty())
        {
            return false;
        }

        std::string struserid = fxUserInfoObject["userId"].asString();
        if (!base::StringToUint(struserid, userid))
        {
            return false;
        }

        *nickname = fxUserInfoObject["nickName"].asString();
        std::string strrichLevel = fxUserInfoObject["richLevel"].asString();
        if (!base::StringToUint(strrichLevel, richlevel))
        {
            return false;
        }
        return true;
    }

    // 测试通过
    bool ExtractStarfulInfo_RoomService_enterRoom_(
        const std::string& inputstr, uint32* staruserid,
        std::string* key, std::string* ext)
    {
        if (inputstr.empty())
        {
            return false;
        }

        const std::string& data = inputstr;
        //解析json数据
        Json::Reader reader;
        Json::Value rootdata(Json::objectValue);
        if (!reader.parse(data, rootdata, false))
        {
            return false;
        }

        // 暂时没有必要检测status的值
        Json::Value dataObject(Json::objectValue);
        dataObject = rootdata.get(std::string("data"), dataObject);
        if (dataObject.empty())
        {
            return false;
        }

        Json::Value liveDataObject(Json::objectValue);
        liveDataObject = dataObject.get("liveData", liveDataObject);
        if (liveDataObject.empty())
        {
            return false;
        }

        std::string recInfo = liveDataObject["recInfo"].asString();
        auto beginPos = recInfo.find(R"("userId":")");
        if (beginPos == std::string::npos)
        {
            assert(false);
            return false;
        }
        beginPos += strlen(R"("userId":")");
        auto endPos = recInfo.find(R"(",)", beginPos);
        std::string strstarUserId = recInfo.substr(beginPos, endPos - beginPos);
        if (!base::StringToUint(strstarUserId, staruserid))
        {
            return false;
        }

        Json::Value socketConfigObject(Json::objectValue);
        socketConfigObject = dataObject.get("socketConfig", socketConfigObject);
        if (socketConfigObject.empty())
        {
            return false;
        }

        // 这是flash的tcp连接的服务器的ip(由url解析出来)和端口信息，后续可能用到，目前不解析
        std::string strsokt = socketConfigObject["sokt"].asString();

        // 重要数据，进入房间后连接tcp所使用的key
        *key = socketConfigObject["enter"].asString();
        *ext = socketConfigObject["ext"].asString();

        return true;
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
    :currentWriteData_(""),
    response_of_RoomService_RoomService_enterRoom_(""),
    response_of_Services_UserService_UserService_getMyUserDataInfo_("")
{
}

CurlWrapper::~CurlWrapper()
{
}

void CurlWrapper::CurlInit()
{
    curl_global_init(CURL_GLOBAL_ALL);
}

void CurlWrapper::CurlCleanup()
{
    curl_global_cleanup();
}

bool CurlWrapper::LoginRequestWithCookies()
{
    std::string cookies = "";
    bool ret = false;

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
    headers = curl_slist_append(headers, "Connection:Keep-Alive");
    headers = curl_slist_append(headers,  "Accept-Language:zh-CN");
    headers = curl_slist_append(headers, "Accept:text/html,application/xhtml+xml,*/*");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
    curl_easy_setopt(curl, CURLOPT_REFERER, "www.fanxing.kugou.com");

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

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

// 测试成功
bool CurlWrapper::LoginRequestWithUsernameAndPassword(const std::string& username,
    const std::string& password)
{
    bool ret = false;

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = loginuserurl;
    url += "/v1/login/?appid=1010";
    url += "&username=" + UrlEncode(username);
    url += "&pwd=" + MakeMd5FromString(password);
    url += "&code=";
    uint32 nowtime = static_cast<uint32>(base::Time::Now().ToDoubleT());
    url += "&clienttime=" + base::UintToString(nowtime);
    url += "&expire_day=3";//这里先写死
    url += "&autologin=false";
    url += "&redirect_uri=";
    url += "&state=";
    url += "&callback=loginSuccessCallback";
    url += "&login_ver=1";
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
    headers = curl_slist_append(headers, "Accept:*/*");
    headers = curl_slist_append(headers, "Host:login-user.kugou.com");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
    curl_easy_setopt(curl, CURLOPT_REFERER, fanxingurl);
    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    

    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
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


    response_of_LoginWithUsernameAndPassword_ = currentWriteData_;
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = base::UTF8ToWide(MakeReasonablePath(__FUNCTION__) + ".txt");
    base::FilePath logpath = dirPath.Append(filename);
    base::File logfile(logpath,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    logfile.Write(0, response_of_LoginWithUsernameAndPassword_.c_str(), response_of_LoginWithUsernameAndPassword_.size());

    auto pos = currentResponseHeader_.find("KuGoo");
    if (pos != std::string::npos)
    {
        auto begin = pos;
        auto end = currentResponseHeader_.find(';', begin);
        if (end != std::string::npos)
        {
            cookiesmanager_.SetCookies("KuGoo", currentResponseHeader_.substr(begin, end - begin));
        }
    }

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

// 测试通过
bool CurlWrapper::Services_UserService_UserService_getMyUserDataInfo()
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookies = cookiesmanager_.GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = R"(http://fanxing.kugou.com/Services.php?act=UserService.UserService&mtd=getMyUserDataInfo&args=[]&_=)";
    url += GetNowTimeString();
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
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    
    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

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

    response_of_Services_UserService_UserService_getMyUserDataInfo_ = currentWriteData_;

    auto pos = currentResponseHeader_.find("fxClientInfo");
    if (pos != std::string::npos)
    {
        auto begin = pos;
        auto end = currentResponseHeader_.find(';', begin);
        cookiesmanager_.SetCookies("fxClientInfo",currentResponseHeader_.substr(begin, end - begin));
    }
    
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

// 测试通过
bool CurlWrapper::Services_IndexService_IndexService_getUserCenter()
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookies = cookiesmanager_.GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string url = R"(http://fanxing.kugou.com/Services.php?act=IndexService.IndexService&mtd=getUserCenter&args=[""]&_=)";
    url += GetNowTimeString();
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
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__) + ".txt";
    curl_easy_setopt(curl, CURLOPT_COOKIEJAR, path.c_str());

    currentWriteData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);
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


    bool result = true;
    result &= SetCookieFromString("FANXING", currentResponseHeader_);
    result &= SetCookieFromString("FANXING_COIN", currentResponseHeader_);
    result &= SetCookieFromString("_fx_coin", currentResponseHeader_);
    result &= SetCookieFromString("_fxNickName", currentResponseHeader_);
    result &= SetCookieFromString("_fxRichLevel", currentResponseHeader_);
    result &= SetCookieFromString("_fx_user", currentResponseHeader_);

    assert(result);
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
    cookies = cookiesmanager_.GetCookies(keys);

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
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookies = cookiesmanager_.GetCookies(keys);

	bool ret = false;

	CURL *curl;
	CURLcode res;
	curl = curl_easy_init();

	if (!curl)
		return false;

    std::string strtime = GetNowTimeString();
	std::string url = std::string(fanxingurl);
	url += "/Services.php?d="+strtime+"&act=UserService.UserService&mtd=getCurrentUserInfo&args=%5B%5D&test=3";
	curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
	curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

	/* example.com is redirected, so we tell libcurl to follow redirection */
	curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

	curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

	struct curl_slist *headers = 0;
	headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
	headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
	headers = curl_slist_append(headers, "Connection: Keep-Alive");
	headers = curl_slist_append(headers, "Accept-Language: zh-CN");
	headers = curl_slist_append(headers, "Host: fanxing.kugou.com");

	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
	curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
	std::string referer = "http://fanxing.kugou.com/" + base::IntToString(roomid);
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

    if (!ExtractUsefulInfo_RoomService_enterRoom_(currentWriteData_, userid, nickname, richlevel))
    {
        return false;
    }
    
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

// 测试通过
bool CurlWrapper::RoomService_RoomService_enterRoom(uint32 roomid)
{
    LOG(INFO) << __FUNCTION__ << L" roomid = " << base::UintToString(roomid);
    //GET /Services.php?act=RoomService.RoomService&mtd=enterRoom&args=["1017131","","","web"]&_=1439814776455 HTTP/1.1

    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookies = cookiesmanager_.GetCookies(keys);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    std::string strroomid = base::IntToString(static_cast<int>(roomid));
    std::string url = std::string(fanxingurl);
    url += "/Services.php?act=RoomService.RoomService&mtd=enterRoom&args=[%22";
    url += strroomid + "%22,%22%22,%22%22,%22web%22]&_=";
    url += GetNowTimeString();
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    //headers = curl_slist_append(headers, "Accept: text/html, application/xhtml+xml, */*");
    headers = curl_slist_append(headers, "X-Requested-With: XMLHttpRequest");
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: application/json;text/javascript,*/*;q=0.01");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
    std::string referer = "http://fanxing.kugou.com/" + strroomid;
    curl_easy_setopt(curl, CURLOPT_REFERER, referer.c_str());
    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);

    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies.c_str());
    // 把请求返回来时设置的cookie保存起来
    std::string path = "d:/cookie_";
    path += MakeReasonablePath(__FUNCTION__)+ ".txt";
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
    response_of_RoomService_RoomService_enterRoom_ = currentWriteData_;

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

bool CurlWrapper::ExtractStarfulInfo_RoomService_enterRoom(uint32* staruserid,
    std::string* key, std::string* ext)
{
    return ExtractStarfulInfo_RoomService_enterRoom_(
        response_of_RoomService_RoomService_enterRoom_, staruserid, key, ext);
}

// GET /Services.php?act=GiftService.GiftService&args=["1020846","1a392af9cd9a2c9276c956bbb09f9676_vs0601"]&mtd=tryGetHappyFreeCoin&ran=0.3374897129466474 HTTP/1.1
// GET /Services.php?act=GiftService%2EGiftService&args=%5B%221020846%22%2C%221a392af9cd9a2c9276c956bbb09f9676_vs0601%22%5D&mtd=tryGetHappyFreeCoin&ran=0%2E3377602129466474 HTTP/1.1
bool CurlWrapper::GiftService_GiftService(uint32 roomid,
    const std::string& key_601, std::wstring* responsedata)
{
    std::vector<std::string> keys;
    keys.push_back("KuGoo");
    std::string cookies = cookiesmanager_.GetCookies(keys);
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
    headers = curl_slist_append(headers, "Accept-Language: zh-CN");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");
    //headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: */*");
    headers = curl_slist_append(headers, "x-flash-version: 12,0,0,77");
    headers = curl_slist_append(headers, "application/x-www-form-urlencoded");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
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
    std::string cookies = cookiesmanager_.GetCookies(keys);

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
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: fanxing.kugou.com");
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: application/json;text/javascript,*/*;q=0.01");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
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
    std::string cookies = cookiesmanager_.GetCookies(keys);

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
    headers = curl_slist_append(headers, "Connection: Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language: zh-CN,en,*");
    headers = curl_slist_append(headers, "Host: visitor.fanxing.kugou.com");//这个值与别的不一样
    headers = curl_slist_append(headers, "Accept-Charset: GBK,utf-8;q=0.7,*;q=0.3");
    headers = curl_slist_append(headers, "Accept: application/json;text/javascript,*/*;q=0.01");
    headers = curl_slist_append(headers, "Content-Type: application/json;charset=utf-8");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    // 这里不要接受压缩的数据包，免得解压麻烦
    //curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, acceptencode);
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
    bool result = cookiesmanager_.SetCookies(key, cookiestring.substr(begin, end - begin));
    return result;
}

