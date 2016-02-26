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
#include "third_party/chromium/base/files/file.h"
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
        p->WriteResponseDataCallback(data);
        return size*nmemb;
    }

    static size_t header_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string data;
        data.assign(ptr, ptr + size*nmemb);
        FamilyDaily* p = static_cast<FamilyDaily*>(userdata);
        p->WriteResponseHeaderCallback(data);
        return size*nmemb;
    }
    
    std::string MakeReasonablePath(const std::string& pathfile)
    {
        auto temp = pathfile;
        auto pos = temp.find(':');
        while (pos != std::string::npos)
        {
            temp.erase(pos, 1);
            pos = temp.find(':');
        }
        return std::move(temp);
    }

    std::string MakeFormatDateString(const base::Time time)
    {
        base::Time::Exploded exploded;
        time.LocalExplode(&exploded);
        std::string month = base::IntToString(exploded.month);
        if (month.length()<2)
        {
            month = "0" + month;
        }
        std::string day_of_month = base::IntToString(exploded.day_of_month);
        if (day_of_month.length() < 2)
        {
            day_of_month = "0" + day_of_month;
        }

        std::string timestring = base::IntToString(exploded.year) + "-" +
            month + "-" + day_of_month;
        return std::move(timestring);
    }


    static const char* mark_table_begin = "<table";
    static const char* mark_table_end = "</table>";
    static const char* mark_tr_begin = "<tr>";
    static const char* mark_tr_end = "</tr>";
    static const char* mark_td_begin = "<td>";
    static const char* mark_td_end = "</td>";

    bool GetMarkData(const std::string& pagedata, size_t beginpos,
        const std::string& beginmark, 
        const std::string& endmark,
        std::string* targetdata,
        size_t *afterendmarkpos)
    {
        beginpos = pagedata.find(beginmark, beginpos);
        if (beginpos == std::string::npos)
            return false;
        beginpos += beginmark.size();

        auto endpos = pagedata.find(endmark, beginpos);
        if (endpos == std::string::npos)
            return false;

        *afterendmarkpos = endpos + endmark.size();
        *targetdata = pagedata.substr(beginpos, endpos - beginpos);
        return true;
    }

    bool GetTableData(const std::string& pagedata, std::string* tabledata)
    {
        size_t tableendpos = 0;
        return GetMarkData(pagedata, 0, mark_table_begin, mark_table_end, 
            tabledata, &tableendpos);
    }

    bool GetTrData(const std::string& pagedata, size_t beginpos, 
        std::string* trdata, size_t* trendpos)
    {
        return GetMarkData(pagedata, beginpos, mark_tr_begin, mark_tr_end, trdata, trendpos);
    }

    bool GetTdData(const std::string& pagedata, size_t beginpos, size_t* tdendpos, std::string* tddata)
    {
        return GetMarkData(pagedata, beginpos, mark_td_begin, mark_td_end, tddata, tdendpos);
    }

    bool GetSingerInfoFromTdData(const std::string& tddata,
        size_t beginpos, std::string* singername, 
        uint32* singerid, uint32* roomid)
    {
        std::string strroomid;
        size_t pos = 0;
        if (!GetMarkData(tddata, beginpos, "http://fanxing.kugou.com/", "\"", &strroomid, &pos))
            return false;

        std::string tempsingername;
        beginpos = pos;
        if (!GetMarkData(tddata, beginpos, R"(target="_blank">)", "</a>", &tempsingername, &pos))
            return false;

        std::string strsingerid;
        beginpos = pos;
        if (!GetMarkData(tddata, beginpos, "(", ")", &strsingerid, &pos))
            return false;

        if (!base::StringToUint(strroomid, roomid))
            return false;

        if (!base::StringToUint(strsingerid, singerid))
            return false;

        *singername = tempsingername;
        return true;
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

bool FamilyDaily::WriteResponseDataCallback(const std::string& data)
{
    currentResponseData_ += data;
    return true;
}

bool FamilyDaily::WriteResponseHeaderCallback(const std::string& data)
{
    currentResponseHeader_ += data;
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

    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

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

    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);
    
    currentResponseData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    // 为了获取cookies的数据
    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (headers)
    {
        curl_slist_free_all(headers);
    }

    /* Check for errors */
    if (res != CURLE_OK)
    {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }

    std::wstring filename = base::UTF8ToWide(MakeReasonablePath(__FUNCTION__) + ".txt");
    base::FilePath logpath = dirPath.Append(filename);
    base::File logfile(logpath,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    logfile.Write(0, currentResponseData_.c_str(), currentResponseData_.size());

    auto pos = currentResponseHeader_.find("PHPSESSID=");
    if (pos != std::string::npos)
    {
        auto begin = pos;
        auto end = currentResponseHeader_.find(';', begin);
        cookies_ = currentResponseHeader_.substr(begin, end - begin);
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
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    assert(result);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    
    std::string url = std::string(familyurl) + "/admin?act=login";
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    // 这里如果跟下去,就无法判断目前结果
    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0L);

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
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies_.c_str());

    std::string encodeusername = UrlEncode(username);
    std::string encodepassword = UrlEncode(password);
    std::string postFields = "adminName=" + encodeusername + "&password=" + encodepassword;

    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, postFields.size());
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    
    currentResponseData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    currentResponseHeader_.clear();
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);

    if (headers)
    {
        curl_slist_free_all(headers);
    }

    /* Check for errors */
    if (res != CURLE_OK)
    {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }

    auto pos = currentResponseHeader_.find("f_p=");
    if (pos != std::string::npos)
    {
        auto begin = pos;
        auto end = currentResponseHeader_.find(';', begin);
        std::string cookies = currentResponseHeader_.substr(begin, end - begin);
        cookies_ += ";" + cookies;
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

    std::wstring filename = base::UTF8ToWide(MakeReasonablePath(__FUNCTION__) + ".txt");
    base::FilePath logpath = dirPath.Append(filename);
    base::File logfile(logpath,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    logfile.Write(0, currentResponseData_.c_str(), currentResponseData_.size());

    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 302)
    {
        return true;
    }

    if (responsecode == 200 && currentResponseData_ =="0")
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

//GET /admin?act=sumStarDataList&startDay=2016-01-01&endDay=2016-01-21 HTTP/1.1
//Host: family.fanxing.kugou.com
//Connection: keep-alive
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://family.fanxing.kugou.com/admin?act=sumStarDataList
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: PHPSESSID=3dfg2i875s2p7hcousceilmag3; f_p=f_56a0f5ea31c3f7.88052429

bool FamilyDaily::GetSummaryData(const base::Time& begintime, const base::Time& endtime,
    std::vector<SingerSummaryData>* summerydata)
{
    std::string pagedata;
    bool result = GetSummaryDataByPage(begintime, endtime, 1, &pagedata);
    assert(result);
    if (!result)
        return false;

    std::vector<SingerSummaryData> pagesummary;
    uint32 pagecount = 0;
    result = ParsePageCount(pagedata, &pagecount);
    assert(result);
    if (!result)
        return false;

    result = ParseSummaryData(pagedata, &pagesummary);
    assert(result);
    if (!result)
        return false;

    assert(pagecount);
    assert(!pagesummary.empty());

    summerydata->assign(pagesummary.begin(), pagesummary.end());

    for (uint32 page = 2; page <= pagecount; page++)
    {
        result = GetSummaryDataByPage(begintime, endtime, page, &pagedata);
        assert(result);
        if (!result)
            return false;

        pagesummary.clear();
        result = ParseSummaryData(pagedata, &pagesummary);
        assert(result);
        if (!result)
            return false;

        assert(!pagesummary.empty());
        summerydata->insert(summerydata->end(), 
            pagesummary.begin(), pagesummary.end());
    }
    return result;
}

bool FamilyDaily::GetSummaryDataByPage(const base::Time& begintime,
    const base::Time& endtime, uint32 pagenumber,
    std::string* pagedata)
{
    std::string beginstring = MakeFormatDateString(begintime);
    std::string endstring = MakeFormatDateString(endtime);

    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);

    std::string requesturl = std::string(familyurl) +
        "/admin?act=sumStarDataList&startDay=" +
        beginstring + "&endDay=" + endstring;

    // 第一页数据不需要加页码
    if (pagenumber > 1)
    {
        requesturl += "&page=" + base::IntToString(pagenumber);
    }

    curl_easy_setopt(curl, CURLOPT_URL, requesturl.c_str());

    // 设置Get方式
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    // 这里如果跟下去,就无法判断目前结果
    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, cookie_connection);
    //headers = curl_slist_append(headers, "Cache-Control: max-age=0");
    headers = curl_slist_append(headers, cookie_language);
    //headers = curl_slist_append(headers, "Origin: http://family.fanxing.kugou.com");
    headers = curl_slist_append(headers, cookie_accept);
    headers = curl_slist_append(headers, cookie_upgrade);
    headers = curl_slist_append(headers, cookie_useragent);
    headers = curl_slist_append(headers, cookie_content);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    //curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);
    //curl_easy_setopt(curl, CURLOPT_REFERER, "http://family.fanxing.kugou.com/admin?act=sumStarDataList");

    //curl_easy_setopt(curl, CURLOPT_COOKIEFILE, cookiespath_.c_str());
    curl_easy_setopt(curl, CURLOPT_COOKIE, cookies_.c_str());
    currentResponseData_.clear();
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);

    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    if (headers)
    {
        curl_slist_free_all(headers);
    }
    
    /* Check for errors */
    if (res != CURLE_OK)
    {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = base::UTF8ToWide(MakeReasonablePath(__FUNCTION__) + ".txt");
    base::FilePath logpath = dirPath.Append(filename);
    base::File logfile(logpath,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    logfile.Write(0, currentResponseData_.c_str(), currentResponseData_.size());

    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 302)
    {
        assert(false && L"GetSummaryData must set CURLOPT_FOLLOWLOCATION to follow redirection");
        return true;
    }

    if (responsecode == 200 && !currentResponseData_.empty())
    {
        *pagedata = currentResponseData_;
        return true;
    }
    return false;
}

// colspan
bool FamilyDaily::ParseSummaryData(const std::string& pagedata,    
    std::vector<SingerSummaryData>* summerydata)
{
    bool result = false;
    std::string tabledata;
    result = GetTableData(pagedata, &tabledata);
    std::string trdata;
    size_t trendpos = 0;
    size_t beginpos = 0;
    // 第一个数据是表头
    result = GetTrData(tabledata, beginpos, &trdata, &trendpos);

    std::vector<std::string> trvector;
    while (result)
    {
        beginpos = trendpos;
        result = GetTrData(tabledata, beginpos, &trdata, &trendpos);
        if (result)
            trvector.push_back(trdata);
    }

    // 最后一个是表格下方页数数据
    if (!trvector.empty())
    {
        trvector.erase(trvector.end()-1);
    }
    
    
    std::vector<SingerSummaryData> singerSummaryDataVector;
    for (const auto& it : trvector)
    {
        std::string trdata = it;
        size_t beginpos = 0;
        size_t endpos = 0;
        std::string tddata;      
        SingerSummaryData singerSummaryData;

        // 主播字段
        // <a href="http://fanxing.kugou.com/1060982" target="_blank">陌路shiley</a>(10215159)
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        uint32 singerid = 0;
        uint32 roomid = 0;
        std::string singername;
        if (!GetSingerInfoFromTdData(tddata, beginpos, &singername, &singerid, &roomid))
        {
            assert(false);
            continue;
        }

        singerSummaryData.singerid = singerid;
        singerSummaryData.roomid = roomid;
        singerSummaryData.nickname = singername;

        // 这个数据是直播海报字段，不使用
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }

        // 主播等级
        beginpos = endpos;
        result = GetTdData(trdata, beginpos, &endpos, &tddata);
        singerSummaryData.singerlevel = tddata;

        // 开播次数
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        uint32 onlinecount = 0;
        if (!base::StringToUint(tddata, &onlinecount))
        {
            assert(false);
            return false;
        }

        singerSummaryData.onlinecount = onlinecount;

        // 累计直播时长（分钟）
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        
        uint32 onlineminute = 0;
        if (!base::StringToUint(tddata, &onlineminute))
        {
            assert(false);
            return false;
        }

        singerSummaryData.onlineminute = onlineminute;

        // 有效直播次数（大于1个小时）
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        uint32 effectivecount = 0;
        if (!base::StringToUint(tddata, &effectivecount))
        {
            assert(false);
            return false;
        }
        singerSummaryData.effectivecount = effectivecount;

        // 直播间最高人气
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        uint32 maxusers = 0;
        if (!base::StringToUint(tddata, &maxusers))
        {
            assert(false);
            return false;
        }
        singerSummaryData.maxusers = maxusers;

        // 星豆收入
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        double revenue = 0;
        if (!base::StringToDouble(tddata, &revenue))
        {
            assert(false);
            return false;
        }
        singerSummaryData.revenue = revenue;
        singerSummaryDataVector.push_back(singerSummaryData);
    }

    *summerydata = std::move(singerSummaryDataVector);
    return true;
}

bool FamilyDaily::ParsePageCount(const std::string& pagedata, uint32* pagenumber)
{
    static const char* colspan = R"(<tr><td colspan=")";
    static const char* doublequote = R"(")";
    std::string str = colspan;
    auto pos = pagedata.find(str);
    if (pos == pagedata.npos)
        return false;

    pos += strlen(colspan);
    auto endpos = pagedata.find(doublequote, pos);
    std::string count = pagedata.substr(pos, endpos - pos);
    uint32 countperpage = 0;
    if (!base::StringToUint(count, &countperpage))
        return false;

    if (!countperpage)
        return false;

    std::string totalbegin = base::WideToUTF8(L"<span>总共");
    std::string totalend = base::WideToUTF8(L"条记录");

    pos = pagedata.find(totalbegin);
    if (pos == std::string::npos)
        return false;

    pos += totalbegin.size();
    endpos = pagedata.find(totalend);
    std::string strtotalcount = pagedata.substr(pos, endpos - pos);
    uint32 totalcount = 0;
    if (!base::StringToUint(strtotalcount, &totalcount))
        return false;

    *pagenumber = totalcount / countperpage;
    *pagenumber += (totalcount % countperpage) > 0 ? 1 : 0;
    return true;
}
