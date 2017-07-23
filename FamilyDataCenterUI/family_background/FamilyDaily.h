#pragma once

#include <string>
#include <vector>

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "Network/CookiesHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"

struct SingerDailyData
{
    std::string date;
    uint32 onlinecount = 0;         //开播次数
    uint32 onlineminute = 0;        //累计直播时长（分钟）
    std::string pc_hours;
    std::string phone_hours;
    uint32 effectivecount = 0;      //有效直播次数（大于1个小时）
    uint32 maxusers = 0;            //直播间最高人气
    double revenue = 0.0;             // 星豆收入
    uint32 blame = 0;               // 周期累计扣分
};

class SingerSummaryData
{
public:
    uint32 singerid = 0;
    uint32 roomid = 0;
    std::string nickname;
    std::string singerlevel;    //主播等级
    uint32 onlinecount = 0;         //开播次数
    uint32 onlineminute = 0;        //累计直播时长（分钟）
    std::string total_hours;    // 累计直播时长(小时)
    std::string pc_hours;
    std::string phone_hours;
    uint32 effectivecount = 0;      //有效直播次数（大于1个小时）
    uint32 effectivedays = 0;       //有效直播天，当天至少一次有效直播次数
    uint32 maxusers = 0;            //直播间最高人气
    double revenue = 0.0;             // 星豆收入
};

class FamilyDaily
{
public:
    FamilyDaily();
    ~FamilyDaily();

    static void CurlInit();
    static void CurlCleanup();

    // 打开主页面,获取对应的cookie;
    bool Init();

    bool Login(const std::string& username, const std::string& password);

    bool GetDailyDataBySingerId(uint32 singerid,
                                const base::Time& begintime, 
                                const base::Time& endtime,
                                std::vector<SingerDailyData>* dailydata);

    bool GetSummaryData(const base::Time& begintime, const base::Time& endtime,
                        std::vector<SingerSummaryData>* summerydata);

    bool GetNormalSingerList(std::vector<uint32>* singerids);

    bool WriteResponseDataCallback(const std::string& data);
    bool WriteResponseHeaderCallback(const std::string& data);

private:
    bool GetSummaryDataByPage(const base::Time& begintime,
        const base::Time& endtime, uint32 pagenumber,
        std::string* pagedata);

    bool GetDailyDataBySingerIdAndPage(uint32 singerid,
        const base::Time& begintime,
        const base::Time& endtime, uint32 pagenumber,
        std::string* pagedata);

    bool GetSingerListByPage(uint32 pagenumber, std::string* pagedata);

    // 从摘取的数据中获取主播数据信息以及所有分页数量
    bool ParseSummaryData(const std::string& pagedata, 
                          std::vector<SingerSummaryData>* summerydata) const;

    bool ParseSingerDailyData(const std::string& pagedata,
        std::vector<SingerDailyData>* singerdailydata) const;

    bool ParseSingerListData(const std::string& pagedata, 
        std::vector<uint32>* singerids) const;

    bool ParsePageCount(const std::string& pagedata, uint32* pagenumber) const;

    CookiesHelper cookies_helper_;

    std::string currentResponseData_;
    std::string currentResponseHeader_;
};
