#include "stdafx.h"
#include "FanXingUI/family_background/FamilyBackground.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#include "Network/FamilyDaily.h"
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
    bool result = Init();
    result = Login("globalstar001", "123!");

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

    result = GetSummaryData(begintime, endtime);
}

bool FamilyBackground::Init()
{
    FamilyDaily::CurlInit();
    familyDaily_.reset(new FamilyDaily);
    bool result = familyDaily_->Init();
    return result;
}

bool FamilyBackground::Login(const std::string& username, 
                             const std::string& password)
{
    bool result = familyDaily_->Login(username, password);
    return result;
}

bool FamilyBackground::GetSummaryData(const base::Time& begintime,
    const base::Time& endtime)
{
    std::vector<SingerSummaryData> summary;
    bool result = familyDaily_->GetSummaryData(begintime, endtime, &summary);
    return result;
}

