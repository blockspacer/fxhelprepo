#include "stdafx.h"
#include "FamilyDataCenterUI/family_background/FamilyBackground.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

//FamilyBackground familyBackground;
//familyBackground.Init();
//familyBackground.Login("username", "password");

FamilyBackground::FamilyBackground()
{

}

FamilyBackground::~FamilyBackground()
{

}

void FamilyBackground::Test()
{
    bool result = Init("family.fanxing.kugou.com");
    result = Login("111", "123!");

    base::Time nowtime = base::Time::Now();
    base::Time::Exploded exploded;
    nowtime.LocalExplode(&exploded);
    int dayofmonth = exploded.day_of_month;
    exploded.day_of_month = 1;
    exploded.hour = 0;
    exploded.minute = 0;
    exploded.second = 1;
    exploded.day_of_week = (exploded.day_of_week - dayofmonth%7 + 8) % 7;
    base::Time begintime = base::Time::FromLocalExploded(exploded);
    base::Time endtime = base::Time::Now();

    std::vector<SingerSummaryData> summary;

    //result = GetSummaryData(begintime, endtime, &summary);
}

bool FamilyBackground::Init(const std::string& family_host)
{
    FamilyDaily::CurlInit();
    familyDaily_.reset(new FamilyDaily);
    bool result = familyDaily_->Init(family_host);
    return result;
}

bool FamilyBackground::Login(const std::string& username, 
                             const std::string& password)
{
    bool result = familyDaily_->Login(username, password);
    return result;
}

bool FamilyBackground::GetServerTime(base::Time* server_time) const
{
    return familyDaily_->GetServerTime(server_time);
}

bool FamilyBackground::GetSummaryData(const base::Time& begintime, 
    const base::Time& endtime,
    std::vector<SingerSummaryData>* summary)
{
    if (!summary)
        return false;

    bool result = familyDaily_->GetSummaryData(begintime, endtime, summary);
    return result;
}

bool FamilyBackground::GetDailyDataBySingerId(uint32 singerid, 
    const base::Time& begintime, const base::Time& endtime, 
    std::vector<SingerDailyData>* singerdata)
{
    if (!singerdata)
        return false;

    bool result = familyDaily_->GetDailyDataBySingerId(singerid, begintime,
        endtime, singerdata);

    return result;
}

bool FamilyBackground::GetNormalSingerIds(std::vector<uint32>* singerids,
    const base::Callback<bool(const std::string&)>& assign_check_func)
{
    bool result = familyDaily_->GetNormalSingerList(singerids,assign_check_func);
    return true;
}

