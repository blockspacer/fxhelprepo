#include "stdafx.h"
#include "FamilyDaily.h"

#include <memory>
#include <assert.h>
#include <iostream>
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_split.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/rand_util.h"
#include "third_party/json/json.h"


namespace{

    // 改为授权以后，都使用从授权文件传进来，这个参数只为测试方便是使用
    const char* family_url = "http://family.fanxing.kugou.com";

    // cookies 
    const char* header_connection = "Connection:Keep-Alive";
    const char* header_host = "Host: family.fanxing.kugou.com";
    const char* header_accept = "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    const char* header_upgrade = "Upgrade-Insecure-Requests: 1";
    const char* header_useragent = "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36";
    const char* header_language = "Accept-Language: zh-CN,zh;q=0.8";
    const char* header_content = "Content-Type: application/x-www-form-urlencoded";

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

    uint32 OnlineTimeToMinutes(const std::string& online_time)
    {
        assert(online_time.size());
        std::wstring w_online_time = base::UTF8ToWide(online_time);
        std::vector<std::wstring> hours_minutes;
        base::SplitStringUsingSubstr(w_online_time, L"时", &hours_minutes);
        uint32 online_minutes = 0;
        bool result = false;
        if (hours_minutes.size() == 1)
        {
            if (w_online_time.find(L"秒") != std::wstring::npos)// 只有秒
            {
                online_minutes = 0;
            }
            else if (w_online_time.find(L"分")!=std::wstring::npos)// 只有分
            {
                result = base::StringToUint(base::WideToUTF8(w_online_time), &online_minutes);
                //assert(result);
            }
            else // 只有小时
            {               
                uint32 online_hours = 0;
                result = base::StringToUint(base::WideToUTF8(w_online_time), &online_hours);
                online_minutes = online_hours * 60;
            }
        }
        else if (hours_minutes.size() == 2)
        {
            uint32 online_hours = 0;
            result = base::StringToUint(base::WideToUTF8(hours_minutes.at(0)), &online_hours);
            assert(result);
            result = base::StringToUint(base::WideToUTF8(hours_minutes.at(1)), &online_minutes);
            assert(result==false);// 因为有"分"为中文，转换失败，但是前面的数字转出来是对的
            online_minutes += online_hours * 60;
        }
        else
        {
            assert(false);
        }
        return online_minutes;
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
    CurlWrapper::CurlInit();
}
void FamilyDaily::CurlCleanup()
{
    CurlWrapper::CurlCleanup();
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
bool FamilyDaily::Init(const std::string& family_host)
{
    if (family_host.empty())
        return false;

    family_url_ = std::string("http://") + family_host;
    HttpRequest request;
    request.url = std::string(family_url_) + "/admin?act=login";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    CurlWrapper curl_wrapper;
    HttpResponse response;
    if (!curl_wrapper.Execute(request, &response))
        return false;

    for (const auto& cookie : response.cookies)
    {
        cookies_helper_.SetCookies(cookie);
    }
    return true;
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
    HttpRequest request;
    request.url = std::string(family_url_) + "/admin?act=login";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    request.referer = request.url;
    request.cookies = cookies_helper_.GetAllCookies();
    std::string encodeusername = UrlEncode(username);
    std::string encodepassword = UrlEncode(password);
    std::string post_str = "adminName=" + encodeusername + "&password=" + encodepassword;
    request.postdata.assign(post_str.begin(), post_str.end());

    CurlWrapper curl_wrapper;
    HttpResponse response;
    if (!curl_wrapper.Execute(request, &response))
        return false;
    
    if (response.statuscode/100 != 2)
        return false;

    for (const auto& cookie : response.cookies)
    {
        cookies_helper_.SetCookies(cookie);
    }

    return true;
}

bool FamilyDaily::GetServerTime(base::Time* server_time) const
{
    std::string expires = cookies_helper_.GetCookies("expires");
    if (expires.empty())
        return false;

    base::Time expires_time = base::Time::Now();
    base::Time::FromUTCString(expires.c_str(), &expires_time);

    std::string test_time = MakeFormatDateString(expires_time);
    *server_time = expires_time;
    return true;
}

//GET http://family.fanxing.kugou.com/admin?act=dayStarDataList&startDay=2016-03-01&endDay=2016-03-06&starId=116432085 HTTP/1.1
//Host: family.fanxing.kugou.com
//Connection: keep-alive
//Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8
//Upgrade-Insecure-Requests: 1
//User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36
//Referer: http://family.fanxing.kugou.com/admin?act=dayStarDataList
//Accept-Encoding: gzip, deflate, sdch
//Accept-Language: zh-CN,zh;q=0.8
//Cookie: PHPSESSID=828t48vhi1u433g06hgtgk1g00; f_p=f_56d1c9af5aea30.63869374; 
//_fx_r_follow=%5B142314756%2C169907997%2C169000775%2C165996638%2C165025636%2C163083252%2C162498721%2C165754417%2C159616776%2C159531158%2C158459161%2C147385523%2C148041628%2C149259681%2C150377024%2C141023689%2C136981235%2C145901830%2C142879819%2C142078347%2C146052549%2C127624914%2C127618396%2C136883471%2C139839374%2C137880598%2C137180088%2C127277529%2C127277337%2C131405036%2C127277804%2C127277690%2C126280898%2C126014190%2C125360917%2C116432085%2C136625872%2C117438839%2C115609206%2C114721789%2C120357891%2C118257208%2C115043602%2C107301439%2C105209005%2C112791202%2C112569182%2C110469751%2C106559459%2C110384212%2C97362569%2C91428332%2C78125705%2C66239107%2C118774682%2C61517977%2C53722204%2C50999842%2C69260016%2C60843845%2C49592188%2C43385401%2C114748626%2C41028958%2C39310466%2C37713273%2C33986382%2C99241150%2C31889265%2C165569900%2C33313091%2C20081096%2C124625434%2C95259291%2C159730972%5D; pgv_pvi=2486706176; pgv_si=s2470812672; 800057627mid=730_44; 800057627slid=slid_396_77%7C; 800057627slid_396_77=1456921767738; 800057627mh=1456921768746; FANXING=%257B%2522kugouId%2522%253A%2522615887139%2522%252C%2522coin%2522%253A%252226815.00%2522%252C%2522atime%2522%253A1457013595%252C%2522isRemember%2522%253A0%252C%2522sign%2522%253A%2522be974c32cd8401a6ea9fab56d1fb213e%2522%257D; KuGoo=KugooID=615887139&KugooPwd=E54BCC78DA033CB1B0402B4E339757C5&NickName=%u0067%u006c%u006f%u0062%u0061%u006c%u0073%u0074%u0061%u0072%u0030%u0030%u0035&Pic=http://imge.kugou.com/kugouicon/165/20100101/20100101192931478054.jpg&RegState=1&RegFrom=&t=430081cafd2e0d8c1f046744235a50c0bfe64e2f7dcbd2e1a74a8c9b489cb072&a_id=1010&ct=1457152785&UserName=%u0067%u006c%u006f%u0062%u0061%u006c%u0073%u0074%u0061%u0072%u0030%u0030%u0035; fxClientInfo=%7B%22userId%22%3A%22105039763%22%2C%22kugouId%22%3A%22615887139%22%2C%22ip%22%3A%22119.131.77.190%22%7D; Hm_lvt_52e69492bce68bf637c6f3a2f099ae08=1456405260; Hm_lpvt_52e69492bce68bf637c6f3a2f099ae08=1457273231; Hm_lvt_e0a7c5eaf6994884c4376a64da96825a=1456405286; Hm_lpvt_e0a7c5eaf6994884c4376a64da96825a=1457273231
bool FamilyDaily::GetDailyDataBySingerId(uint32 singerid,
    const base::Time& begintime, const base::Time& endtime,
    std::vector<SingerDailyData>* singerdailydata)
{
    // 不支持超过30天的查询
    base::TimeDelta delta = endtime - begintime;
    auto days = delta.InDays();
    if (days > 30)
        return false;
    
    std::string pagedata;
    if (!GetDailyDataBySingerIdAndPage(singerid, begintime, endtime, 1, &pagedata))
    {
        return false;
    }

    uint32 pagecount = 0;
    if (!ParsePageCount(pagedata, &pagecount))
    {
        return false;
    }
    assert(pagecount);

    std::vector<SingerDailyData> tempSingerDailyData;
    if (!ParseSingerDailyData(pagedata, &tempSingerDailyData))
    {
        return false;
    }

    singerdailydata->assign(tempSingerDailyData.begin(), tempSingerDailyData.end());

    for (uint32 page = 2; page <= pagecount; page++)
    {
        if (!GetDailyDataBySingerIdAndPage(singerid, begintime, endtime, page, &pagedata))
            return false;

        tempSingerDailyData.clear();
        if (!ParseSingerDailyData(pagedata, &tempSingerDailyData))
            return false;
        assert(!tempSingerDailyData.empty());
        singerdailydata->insert(singerdailydata->end(),
            tempSingerDailyData.begin(), tempSingerDailyData.end());
    }
    return true;
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

bool FamilyDaily::GetSummaryData(const base::Time& begintime, 
    const base::Time& endtime,
    const base::Callback<void(uint32, uint32)>& progress_callback,
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
    if (pagecount <= 1)
        progress_callback.Run(1, 1);
    else
        progress_callback.Run(1, pagecount);

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
        progress_callback.Run(page, pagecount);
    }
    return result;
}

bool FamilyDaily::GetNormalSingerList(std::vector<uint32>* singerids,
    const base::Callback<bool(const std::string&)>& assign_check_func)
{
    std::string pagedata;

    if (!GetSingerListByPage(1, &pagedata))
        return false;

    uint32 pagecount = 0;
    if (!ParsePageCount(pagedata, &pagecount))
        return false;

    std::vector<SingerSummaryData> singer_summary_data;
    if (!ParseSingerListData(pagedata, &singer_summary_data))
        return false;

    for (const auto& it : singer_summary_data)
    {
        if (assign_check_func.Run(it.assign_date_time))
            singerids->push_back(it.singerid);
    }

    for (uint32 page = 2; page <= pagecount; page++)
    {
        if (!GetSingerListByPage(page, &pagedata))
            continue;

        singer_summary_data.clear();
        if (!ParseSingerListData(pagedata, &singer_summary_data))
            return false;

        for (const auto& it : singer_summary_data)
        {
            if (assign_check_func.Run(it.assign_date_time))
                singerids->push_back(it.singerid);
        }
    }

    return true;
}

bool FamilyDaily::GetSummaryDataByPage(const base::Time& begintime,
    const base::Time& endtime, uint32 pagenumber,
    std::string* pagedata)
{
    std::string beginstring = MakeFormatDateString(begintime);
    std::string endstring = MakeFormatDateString(endtime);
    std::string requesturl = std::string(family_url_) +
        "/admin?act=sumStarDataList&startDay=" +
        beginstring + "&endDay=" + endstring;
    // 第一页数据不需要加页码
    if (pagenumber > 1)
        requesturl += "&page=" + base::IntToString(pagenumber);

    HttpRequest request;
    request.url = requesturl;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.cookies = cookies_helper_.GetAllCookies();
    request.referer = "http://family.fanxing.kugou.com/admin?act=sumStarDataList";

    CurlWrapper curl_wrapper;
    HttpResponse response;
    if (!curl_wrapper.Execute(request, &response))
        return false;

    if (response.statuscode / 100 != 2)
        return false;

    if (!response.content.empty())
    {
        pagedata->assign(response.content.begin(), response.content.end());
    }
    
    return true;
}

bool FamilyDaily::GetDailyDataBySingerIdAndPage(uint32 singerid,
    const base::Time& begintime,
    const base::Time& endtime, uint32 pagenumber,
    std::string* pagedata)
{
    std::string beginstring = MakeFormatDateString(begintime);
    std::string endstring = MakeFormatDateString(endtime);
    std::string requesturl = std::string(family_url_) +
        "/admin?act=dayStarDataList&startDay=" +
        beginstring + "&endDay=" + endstring + "&starId="
        + base::UintToString(singerid);

    // 第一页数据不需要加页码
    if (pagenumber > 1)
        requesturl += "&page=" + base::IntToString(pagenumber);

    HttpRequest request;
    request.url = requesturl;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.cookies = cookies_helper_.GetAllCookies();
    request.referer = "http://family.fanxing.kugou.com/admin?act=dayStarDataList";

    CurlWrapper curl_wrapper;
    HttpResponse response;
    if (!curl_wrapper.Execute(request, &response))
        return false;

    if (response.statuscode / 100 != 2)
        return false;

    if (!response.content.empty())
    {
        pagedata->assign(response.content.begin(), response.content.end());
    }

    return true;
}

bool FamilyDaily::GetSingerListByPage(uint32 pagenumber, std::string* pagedata)
{
    std::string requesturl = std::string(family_url_) + "/admin?act=starList";
    // 第一页数据不需要加页码
    if (pagenumber > 1)
        requesturl += "&page=" + base::IntToString(pagenumber);

    HttpRequest request;
    request.url = requesturl;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.cookies = cookies_helper_.GetAllCookies();
    request.referer = "http://family.fanxing.kugou.com/admin?act=menu";

    CurlWrapper curl_wrapper;
    HttpResponse response;
    if (!curl_wrapper.Execute(request, &response))
        return false;

    if (response.statuscode / 100 != 2)
        return false;

    if (!response.content.empty())
    {
        pagedata->assign(response.content.begin(), response.content.end());
    }
    return true;
}

// colspan
bool FamilyDaily::ParseSummaryData(const std::string& pagedata,    
    std::vector<SingerSummaryData>* summerydata) const
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

        std::string total_hours;    // 累计直播时长(小时)
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &total_hours))
        {
            assert(false);
            continue;
        }
        singerSummaryData.total_hours = total_hours;

        singerSummaryData.onlineminute = 0;
        if (total_hours.compare("0")!=0)
        {
            singerSummaryData.onlineminute = OnlineTimeToMinutes(total_hours);
        }

        std::string pc_hours;
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &pc_hours))
        {
            assert(false);
            continue;
        }
        singerSummaryData.pc_hours = pc_hours;

        std::string phone_hours;
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &phone_hours))
        {
            assert(false);
            continue;
        }
        singerSummaryData.phone_hours = phone_hours;

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

bool FamilyDaily::ParseSingerDailyData(const std::string& pagedata,
    std::vector<SingerDailyData>* singerdailydatavector) const
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
        trvector.erase(trvector.end() - 1);
    }

    std::vector<SingerDailyData> tempvector;
    for (const auto& it : trvector)
    {
        std::string trdata = it;
        size_t beginpos = 0;
        size_t endpos = 0;
        std::string tddata;
        SingerDailyData singerdailydata;

        // 日期
        beginpos = endpos;
        result = GetTdData(trdata, beginpos, &endpos, &tddata);
        singerdailydata.date = tddata;

        // 开播次数, 不处理
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
        singerdailydata.onlinecount = onlinecount;

        // 累计直播时长（分钟
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        singerdailydata.onlineminute = OnlineTimeToMinutes(tddata);

        // PC有效直播时长 不处理
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        singerdailydata.pc_minute = OnlineTimeToMinutes(tddata);

        // 手机有效直播时长 不处理
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        singerdailydata.phone_minute = OnlineTimeToMinutes(tddata);

        // 有效直播次数（大于1个小时），重点数据
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
        // 这个值不准确，不能做为参考
        singerdailydata.effectivecount = effectivecount;

        // 直播间最高人气，不处理
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

        // 星豆收入，不处理
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
        singerdailydata.revenue = revenue;

        // 周期累计扣分，不处理
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        uint32 blame = 0;
        if (!base::StringToUint(tddata, &blame))
        {
            assert(false);
            return false;
        }
        tempvector.push_back(singerdailydata);
    }

    *singerdailydatavector = std::move(tempvector);
    return true;
}

bool FamilyDaily::ParseSingerListData(const std::string& pagedata,
    std::vector<SingerSummaryData>* singer_summary_data) const
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
        trvector.erase(trvector.end() - 1);
    }

    for (const auto& it : trvector)
    {
        std::string trdata = it;
        size_t beginpos = 0;
        size_t endpos = 0;
        std::string tddata;
        SingerSummaryData singerSummaryData;

        // 主播ID字段
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        uint32 singerid = 0;
        if (!base::StringToUint(tddata, &singerid))
        {
            assert(false);
            continue;
        }
        singerSummaryData.singerid = singerid;

        // 直播主播昵称
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }

        // 签约时间, 2016-06-03 13:16:32 要用来判断是否当月签约
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        singerSummaryData.assign_date_time = tddata;

        // 最近开播时间,不处理
        beginpos = endpos;
        result = GetTdData(trdata, beginpos, &endpos, &tddata);
        singerSummaryData.last_online = tddata;

        // 主播等级,不处理
        beginpos = endpos;
        result = GetTdData(trdata, beginpos, &endpos, &tddata);
        singerSummaryData.singerlevel = tddata;

        // 主播身份，区分主播数据的字段
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        std::wstring singer_role = base::UTF8ToWide(tddata);
        if (singer_role == L"正式艺人")
        {
            singer_summary_data->push_back(singerSummaryData);
        }

        // 结算密码,不处理
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
        
        // 操作,不处理
        beginpos = endpos;
        if (!GetTdData(trdata, beginpos, &endpos, &tddata))
        {
            assert(false);
            continue;
        }
    }

    return true;
}

bool FamilyDaily::ParsePageCount(const std::string& pagedata, 
    uint32* pagenumber) const
{
    static const char* colspan = R"(colspan)";
    static const char* doublequote = R"(")";
    std::string str = colspan;
    auto pos = pagedata.find(str);
    if (pos == pagedata.npos)
        return false;

    pos += strlen(colspan);
    auto beginpos = pagedata.find(doublequote, pos);
    if (beginpos == pagedata.npos)
        return false;

    auto endpos = pagedata.find(doublequote, beginpos+1);
    std::string count = pagedata.substr(beginpos+1, endpos - beginpos-1);
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
