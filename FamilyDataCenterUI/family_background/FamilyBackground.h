#pragma once
#include <string>
#include <vector>
#include <memory>

#include "FamilyDaily.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"

// 本类负责提供所有后台操作的接口
// 包括：登录

class FamilyDaily;

class FamilyBackground
{
public:
    FamilyBackground();
    ~FamilyBackground();

    void Test();
    bool Init(const std::string& family_host);
    bool Login(const std::string& username, const std::string& password);
    bool GetServerTime(base::Time* server_time) const;
    bool GetSummaryData(const base::Time& begintime, const base::Time& endtime, 
        std::vector<SingerSummaryData>* summary);
    bool GetDailyDataBySingerId(uint32 singerid, const base::Time& begintime,
        const base::Time& endtime, std::vector<SingerDailyData>* singerdata);

    bool GetNormalSingerIds(std::vector<uint32>* singerids);

private:
    std::unique_ptr<FamilyDaily> familyDaily_;
};

