#pragma once

#include <string>
#include <vector>

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"

struct SingerDailyData
{
    uint32 singerid;
};

struct SingerSummaryData
{
    uint32 singerid;
    uint32 roomid;
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

    bool WriteResponseDataCallback(const std::string& data);
    bool WriteResponseHeaderCallback(const std::string& data);

private:
    bool GetSummaryDataByPage(const base::Time& begintime,
        const base::Time& endtime, uint32 pagenumber,
        std::string* pagedata);

    // 从摘取的数据中获取主播数据信息以及所有分页数量
    bool ParseSummaryData(const std::string& pagedata, 
                          std::vector<SingerSummaryData>* summerydata);

    bool ParsePageCount(const std::string& pagedata, uint32* pagenumber);

    std::string cookies_;
    std::string cookiespath_;

    std::string currentResponseData_;
    std::string currentResponseHeader_;
};
