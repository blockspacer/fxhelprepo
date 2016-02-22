#pragma once
#include <string>
#include <vector>
#include <memory>

#include "Network/FamilyDaily.h"



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
    bool Init();
    bool Login(const std::string& username, const std::string& password);
    bool GetSummaryData(const base::Time& begintime, const base::Time& endtime,
        std::vector<SingerSummaryData>* summary);

private:
    std::unique_ptr<FamilyDaily> familyDaily_;
};

