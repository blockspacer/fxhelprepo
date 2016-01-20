#pragma once

#include <string>
#include <vector>
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"

struct SingerDailyData
{
    uint32 singerid;
};

struct SingerSummaryData
{
    uint32 singerid;
    std::string nickname;
    std::string singerlevel;    //主播等级
    uint32 onlinecount;         //开播次数
    uint32 onlineminute;        //累计直播时长（分钟）
    uint32 effectivecount;      //有效直播次数（大于1个小时）
    uint32 maxusers;            //直播间最高人气
    double revenue;             // 星豆收入
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
                                std::vector<SingerDailyData> dailydata);

    bool GetSummaryData(const base::Time& begintime, const base::Time& endtime,
                        std::vector<SingerSummaryData>* summerydata);

    bool WriteCallback(const std::string& data);

private:
    std::string cookies_;
    std::string cookiespath_;

    std::string currentWriteData_;
};
