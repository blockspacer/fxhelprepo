#include "SearchHelper.h"
#include <memory>
#include <map>
#include <assert.h>
#include <codecvt>
#include <iostream>
#include "third_party/libcurl/curl/curl.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/rand_util.h"
#include "third_party/json/json.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace
{
    const char* useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
    std::string responsedata = "";
    static size_t write_callback(char *ptr, size_t size, size_t nmemb, void *userdata)
    {
        std::string data;
        data.assign(ptr, ptr + size*nmemb);
        responsedata += data;
        return size*nmemb;
    }

    // 请求数据的时候，通常会多出来一个前缀数据和()
    static bool RemoveNouseString(const std::string& src, std::string* dst)
    {
        auto beginpos = src.find(R"(()");
        auto endpos = src.rfind(R"())");
        if ((beginpos == std::string::npos) ||
            (endpos == std::string::npos))
        {
            return false;
        }
        *dst = src.substr(beginpos + 1, endpos - beginpos-1);
        return true;
    }
}

SearchHelper::SearchHelper()
    :lastactive_(0)
    , pagecount_(0)
{
}


SearchHelper::~SearchHelper()
{
}

void SearchHelper::SetFilter(uint32 lastactive, uint32 pagecount)
{
    lastactive_ = lastactive;
    pagecount_ = pagecount;
}

void SearchHelper::CurlInit()
{
    curl_global_init(CURL_GLOBAL_WIN32);
}

void SearchHelper::CurlCleanup()
{
    curl_global_cleanup();
}

bool SearchHelper::GetAllFamilyInfo(std::map<uint32, FamilyInfo>* familyInfoMap)
{
    std::string responsedata;
    std::vector<std::string> familyIds;
    for (uint32 i = 1; i < pagecount_; i++)
    {
        std::string pageid = base::IntToString(i);
        std::string pageurl = "http://visitor.fanxing.kugou.com/VServices/Clan.ClanServices.getClanList/";
        pageurl += pageid + "-24/";
        bool ret = GetUrlData(pageurl, &responsedata);
        assert(ret);
        if (!ret)
        {
            continue;
        }

        // 可行的简单获取家族id方案
        //const std::string target = R"("clanId":")";
        //auto beginpos = responsedata.find(target);
        //int endpos = 0;
        //while (beginpos != std::string::npos)
        //{
        //    beginpos += target.size();
        //    endpos = responsedata.find(R"(")", beginpos);
        //    std::string familyid = responsedata.substr(beginpos, endpos - beginpos);
        //    familyIds.push_back(familyid);
        //    beginpos = responsedata.find(R"("clanId":")", endpos);
        //}
        //*familyIdList = std::move(familyIds);

        // 为解析更详细信息，用json解析
        std::string datastr;
        if (!RemoveNouseString(responsedata, &datastr))
        {
            assert(false);
            return false;
        }
        Json::Reader reader;
        Json::Value root(Json::ValueType::objectValue);
        if (!reader.parse(datastr, root, false))
        {
            assert(false);
            continue;
        }

        Json::Value jvData(Json::ValueType::objectValue);
        Json::Value data = root.get("data", jvData);
        if (data.isNull())
        {
            assert(false);
            continue;
        }

        // 这个值是全部家族数量,先解析，暂时不用
        Json::Value jvAllCount(Json::ValueType::intValue);
        Json::Value allcount = data.get("count", jvAllCount);
        if (allcount.isInt())
        {
            uint32 allfamilycount = allcount.asUInt();
        }

        Json::Value jvList(Json::ValueType::arrayValue);
        Json::Value clanList = data.get("list", jvList);
        if (!clanList.isArray())
        {
            assert(false);
            continue;
        }
        for (auto clan : clanList)
        {
            FamilyInfo familyInfo;
            std::string str;
            str = clan.get("clanId", "0").asString();
            if (!base::StringToUint(str, &familyInfo.clanid))
            {
                assert(false);
                continue;
            }
            str = clan.get("clanName", "").asString();
            familyInfo.clanname = str;

            str = clan.get("badgeName", "").asString();
            familyInfo.badgename = str;

            // 需要unicode编码转换
            str = clan.get("clanLeaderUserNickName", "0").asString();
            familyInfo.clanleadernickname = str;

            str = clan.get("clanLevel", "0").asString();
            if (!base::StringToUint(str,&familyInfo.clanlevel))
            {
                assert(false);
                continue;
            }

            str = clan.get("coinTotal", "0").asString();
            double dCoinTotal = 0.0;
            if (!base::StringToDouble(str,&dCoinTotal))
            {
                assert(false);
                continue;
            }
            familyInfo.cointotal = static_cast<uint64>(dCoinTotal);

            str = clan.get("clanLeaderUserId", "0").asString();
            if (!base::StringToUint(str,&familyInfo.clanleaderuserid))
            {
                assert(false);
                continue;
            }

            str = clan.get("userCount", "0").asString();
            if (!base::StringToUint(str,&familyInfo.usercount))
            {
                assert(false);
                continue;
            }

            str = clan.get("starCount", "0").asString();
            if (!base::StringToUint(str, &familyInfo.starcount))
            {
                assert(false);
                continue;
            }

            str = clan.get("managerCount", "0").asString();
            if (!base::StringToUint(str, &familyInfo.managercount))
            {
                assert(false);
                continue;
            }

            str = clan.get("subLeaderCount", "0").asString();
            if (!base::StringToUint(str, &familyInfo.subleadercount))
            {
                assert(false);
                continue;
            }

            str = clan.get("clanRoomId", "0").asString();
            if (!base::StringToUint(str, &familyInfo.clanroomid))
            {
                assert(false);
                continue;
            }

            str = clan.get("addTime", "0").asString();
            if (!base::StringToUint64(str, &familyInfo.addtime))
            {
                assert(false);
                continue;
            }

            familyInfo.iscompany = clan.get("isCompany", false).asBool();

            familyInfoMap->insert(std::make_pair(familyInfo.clanid, familyInfo));
        }
    }
    
    return true;
}
bool SearchHelper::GetAllSingers(uint32 clanid, std::vector<std::string>* singerIdList)
{
    std::string clanstr = base::UintToString(clanid);
    std::string responsedata;
    std::string url = "http://fanxing.kugou.com/index.php?action=clan&id=" + clanstr;
    bool ret = GetUrlData(url, &responsedata);
    assert(ret);
    if (!ret)
        return false;

    std::string starNumberTarget = R"(<p class="liveNumColor"><span class="liveGirlNum">)";
    std::string starNumber;
    auto beginpos = responsedata.find(starNumberTarget);
    if (beginpos == std::string::npos)
        return false;

    beginpos += starNumberTarget.size();
    auto endpos = responsedata.find("<", beginpos);
    starNumber = responsedata.substr(beginpos, endpos - beginpos);

    uint32 starcount = 0;
    if (!base::StringToUint(starNumber, &starcount))
    {
        return false;
    }
    
    std::string starlisturl = "http://visitor.fanxing.kugou.com/VServices/Clan.ClanServices.getClanStarListPaging/";
    starlisturl += clanstr + "-1-" + starNumber;
    //ret = GetUrlData(R"(http://visitor.fanxing.kugou.com/VServices/Clan.ClanServices.getClanStarListPaging/2780-1-159/)",&responsedata);
    ret = GetUrlData(starlisturl, &responsedata);
    assert(ret);
    beginpos = responsedata.find(R"(()");
    endpos = responsedata.rfind(R"())");
    std::string datastr1 = responsedata.substr(beginpos+1, endpos - beginpos);

    std::string datastr;
    if (!RemoveNouseString(responsedata, &datastr))
    {
        assert(false);
        return false;
    }
    
    if (datastr1.compare(datastr)==0)
    {
        assert(false);
    }

    // 解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(datastr, rootdata, false))
    {
        return false;
    }

    // 暂时没有必要检测status的值
    Json::Value jvData(Json::ValueType::objectValue);
    Json::Value data = rootdata.get(std::string("data"), jvData);
    if (data.isNull())
    {
        assert(false);
        return false;
    }
    Json::Value jvList(Json::ValueType::arrayValue);
    Json::Value list = data.get(std::string("list"), jvList);
    for (auto star:list)
    {
        const Json::Value& temp = star;
        uint32 userid = temp.get("userId", 0).asUInt();       
        singerIdList->push_back(base::UintToString(userid));
        std::string roomid = temp.get("roomId", "0").asString();
    }

    return true;
}
bool SearchHelper::GetSingsInfos(const std::string& singerId, SingerInfo* singerinfo)
{
    //bool ret = GetUrlData("http://fanxing.kugou.com/index.php?action=user&id=112173148", &responsedata);
    std::string singerurl = "http://fanxing.kugou.com/index.php?action=user&id=" + singerId;
    std::string responsedata;
    bool ret = GetUrlData(singerurl, &responsedata);
    assert(ret);

    // 获取上次直播超过某指定天数的主播
    uint32 minlastday = lastactive_;
    uint32 lastday = 0;
    std::wstring wstr = L"上次直播";
    std::string target = base::WideToUTF8(wstr);
    auto beginpos = responsedata.find(target);
    beginpos = responsedata.find(R"(<dd>)", beginpos);
    beginpos += std::string(R"(<dd>)").size();
    auto endpos = responsedata.find(R"(</dd>)", beginpos);
    std::string str = responsedata.substr(beginpos, endpos - beginpos);
    wstr = base::UTF8ToWide(str);
    auto pos = str.find(base::WideToUTF8(L"天前"));
    if (pos!=std::string::npos)
    {
        str.erase(pos);
        if (base::StringToUint(str, &lastday))
        {
            singerinfo->lastactiionString = str+"day";
        }
    }

    pos = str.find(base::WideToUTF8(L"月前"));
    if (pos != std::string::npos)
    {
        str.erase(pos);
        int month = 0;
        if (base::StringToInt(str, &month))
        {
            lastday = month * 30;
            singerinfo->lastactiionString = str + "month";
        }
    }

    if (lastday <= minlastday)// 最近有直播的主播，不收集信息
    {
        return false;
    }

    // 需要获取数据的主播，再继续分析信息
    // 主播信息页面，就是当前分析的页面
    singerinfo->userurl = singerurl;
    singerinfo->lastday = lastday;
    base::StringToUint(singerId, &singerinfo->userid);

    // 用户昵称
    // class="name" title="
    std::string nicknameflag = R"(class="name" title=")";
    beginpos = responsedata.find(nicknameflag);
    if (beginpos != std::string::npos)
    {
        beginpos += nicknameflag.size();
        endpos = responsedata.find(R"(")", beginpos);
        singerinfo->nickname = responsedata.substr(beginpos, endpos-beginpos);
    }

    // 直播间地址和roomid
    std::string roomidflag = base::WideToUTF8(L"直播间地址");
    beginpos = responsedata.find(roomidflag);
    if (beginpos != std::string::npos)
    {
        beginpos += roomidflag.size();
        roomidflag = R"(href="/)";
        beginpos = responsedata.find(roomidflag, beginpos);
        beginpos += roomidflag.size();
        endpos = responsedata.find(R"(")", beginpos);
        std::string strroomid = responsedata.substr(beginpos, endpos - beginpos);
        base::StringToUint(strroomid, &singerinfo->roomid);
        singerinfo->roomurl = "http://fanxing.kugou.com/" + strroomid;
    }

    // 粉丝数量
    std::string fansflag = base::WideToUTF8(L"TA的粉丝");
    beginpos = responsedata.find(fansflag);
    if (beginpos!=std::string::npos)
    {
        beginpos += fansflag.size();
        fansflag = R"(<em style="margin:0 5px 0 10px;">)";
        beginpos = responsedata.find(fansflag, beginpos);
        beginpos += fansflag.size();
        endpos = responsedata.find(R"(</em>)", beginpos);
        std::string fanstr = responsedata.substr(beginpos, endpos - beginpos);
        base::StringToUint(fanstr, &singerinfo->fans);
    }

    // 个人照片数量
    std::string picflag = base::WideToUTF8(L"相册（");
    beginpos = responsedata.find(picflag);
    if (beginpos != std::string::npos)
    {
        beginpos += picflag.size();
        endpos = responsedata.find(base::WideToUTF8(L"）"), beginpos);
        std::string strpic = responsedata.substr(beginpos, endpos - beginpos);
        base::StringToUint(strpic, &singerinfo->pictures);
    }

    // title='财富级别:
    std::string richflag = base::WideToUTF8(L"title='财富级别:");
    beginpos = responsedata.find(richflag);
    if (beginpos != std::string::npos)
    {
        beginpos += richflag.size();
        endpos = responsedata.find(R"('/>)", beginpos);
        std::string strrich = responsedata.substr(beginpos, endpos - beginpos);
        std::wstring wrichlevel = base::UTF8ToWide(strrich);
        singerinfo->richlevel = strrich;
    }

    // title='明星级别:
    std::string starflag = base::WideToUTF8(L"title='明星级别:");
    beginpos = responsedata.find(starflag);
    if (beginpos != std::string::npos)
    {
        beginpos += starflag.size();
        endpos = responsedata.find(R"('/>)", beginpos);
        std::string strstar = responsedata.substr(beginpos, endpos - beginpos);
        std::wstring wstarlevel = base::UTF8ToWide(strstar);
        singerinfo->starlevel = strstar;
    }

    return true;
}

void SearchHelper::Run()
{
    std::map<uint32, FamilyInfo> familyMap;
    std::vector<SingerInfo> singerVec;
    if (!GetExpiredFamilySingers(&familyMap, &singerVec))
    {
        return;
    }
 
    return;
}

bool SearchHelper::WriteToFile(const std::map<uint32, FamilyInfo>& familyMap,
    const std::vector<SingerInfo>& singerVec)
{
    if (!singerVec.size())
    {
        return true; // 没有数据，不需要写
    }

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::string timestr = base::Uint64ToString(base::Time::Now().ToInternalValue());
    std::wstring prestr = L"上次上播时间跟踪";
    std::wstring filename = prestr + base::UTF8ToWide(timestr + ".txt");
    base::FilePath logpath = dirPath.Append(filename);
    base::FilePath path(logpath);
    base::File fileobject;
    fileobject.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_APPEND);

    std::vector<std::string> headers;
    headers.push_back(base::WideToUTF8(L"家族id"));
    headers.push_back(base::WideToUTF8(L"繁星号"));
    headers.push_back(base::WideToUTF8(L"用户名"));
    headers.push_back(base::WideToUTF8(L"财富等级"));
    headers.push_back(base::WideToUTF8(L"明星等级"));
    headers.push_back(base::WideToUTF8(L"房间号"));
    headers.push_back(base::WideToUTF8(L"上次直播1"));
    headers.push_back(base::WideToUTF8(L"上次直播2"));
    headers.push_back(base::WideToUTF8(L"粉丝数"));
    headers.push_back(base::WideToUTF8(L"个人图片数"));
    headers.push_back(base::WideToUTF8(L"用户页面"));
    headers.push_back(base::WideToUTF8(L"房间链接"));
    std::string headerstr;
    for (const auto& header:headers)
    {
        headerstr += header + " \t";
    }
    headerstr += "\n";
    fileobject.WriteAtCurrentPos(headerstr.c_str(), headerstr.size());

    // 按数据库格式，打印到文件
    for (auto singer : singerVec)
    {
        std::string outstring;
        outstring += base::UintToString(singer.clanid) + " \t";
        outstring += base::UintToString(singer.userid) + " \t";
        outstring += singer.nickname + " \t";
        outstring += singer.richlevel + " \t";
        outstring += singer.starlevel + " \t";
        outstring += base::UintToString(singer.roomid) + " \t";
        outstring += base::UintToString(singer.lastday) + " \t";
        outstring += singer.lastactiionString + " \t";
        outstring += base::UintToString(singer.fans) + " \t";
        outstring += base::UintToString(singer.pictures) + " \t";
        outstring += singer.userurl + " \t";
        outstring += singer.roomurl + " \t";
        outstring += "\n";
        fileobject.WriteAtCurrentPos(outstring.c_str(), outstring.size());
    }
    fileobject.Close();
    return true;
}

bool SearchHelper::GetExpiredFamilySingers(
    std::map<uint32, FamilyInfo>* familyInfoMap,
    std::vector<SingerInfo>* singerinfo)
{
    std::cout << "GetAllFamilyInfo begin!" << std::endl;
    if (!GetAllFamilyInfo(familyInfoMap))
    {
        assert(false);
        return false;
    }
    std::cout << "GetAllFamilyInfo success!" << std::endl;
    std::cout << "GetAllFamilyInfo count = " << 
        base::IntToString(static_cast<int>(familyInfoMap->size()))<< std::endl;
    std::vector<SingerInfo> singerInfoVec;
    //uint32 testcount = 0;
    for (auto familyInfoIt : *familyInfoMap)
    {
        auto& familyInfo = familyInfoIt.second;
        std::cout << "Family id = " <<
            base::IntToString(static_cast<int>(familyInfo.clanid))<< std::endl;
        std::vector<std::string> singerIdList;
        if (!GetAllSingers(familyInfo.clanid, &singerIdList))
        {
            assert(false);
            continue;
        }
        std::cout << "Singer count = " <<
            base::IntToString(singerIdList.size()) << std::endl;
        for (auto singerid : singerIdList)
        {
            SingerInfo singerInfo;
            if (!GetSingsInfos(singerid, &singerInfo))
            {
                continue;
            }
            singerInfo.clanid = familyInfo.clanid;
            singerInfoVec.push_back(singerInfo);
        }
        std::cout << "Expired Singer count = " <<
            base::IntToString(singerInfoVec.size()) << std::endl;
        WriteToFile(*familyInfoMap, singerInfoVec);
        singerInfoVec.clear();
    }

    *singerinfo = singerInfoVec;
    return true;
}
bool SearchHelper::GetExpiredNormalSingers(std::vector<SingerInfo>* singerinfo)
{
    return false;
}

bool SearchHelper::GetClanAllSingerInfo(uint32 clanid)
{
    std::vector<SingerInfo> singerInfoVec;
    std::vector<std::string> singerIdList;
    if (!GetAllSingers(clanid, &singerIdList))
    {
        assert(false);
        return false;
    }
    std::cout << "Singer count = " <<
        base::IntToString(singerIdList.size()) << std::endl;
    for (auto singerid : singerIdList)
    {
        SingerInfo singerInfo;
        if (!GetSingsInfos(singerid, &singerInfo))
        {
            continue;
        }
        singerInfo.clanid = clanid;
        singerInfoVec.push_back(singerInfo);
    }
    std::cout << "Expired Singer count = " <<
        base::IntToString(singerInfoVec.size()) << std::endl;

    std::map<uint32, FamilyInfo> familyInfoMap;
    FamilyInfo familyInfo;
    familyInfoMap.insert(std::make_pair(clanid, familyInfo));
    WriteToFile(familyInfoMap, singerInfoVec);
    return false;
}

bool SearchHelper::GetUrlData(const std::string& url, std::string* response) const
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();

    if (!curl)
        return false;

    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 0L);// 大量爬数据的时候，不要使用长连接
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    curl_easy_setopt(curl, CURLOPT_HEADER, 0L);

    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Connection:Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN");
    headers = curl_slist_append(headers, "Accept:text/html, application/xhtml+xml, */*");

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_AUTOREFERER, 1L);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, this);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);// 设置整个请求超时时间为20秒
    curl_easy_setopt(curl, CURLOPT_HTTPGET, 1L);

    responsedata.clear();
    /* Perform the request, res will get the return code */
    res = curl_easy_perform(curl);
    /* Check for errors */
    if (res != CURLE_OK)
    {
        //fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        curl_easy_cleanup(curl);
        return false;
    }
    // 获取请求业务结果
    long responsecode = 0;
    res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responsecode);

    *response = responsedata;
    /* always cleanup */
    curl_easy_cleanup(curl);

    if (responsecode == 200)
    {
        return true;
    }

    return false;
}
