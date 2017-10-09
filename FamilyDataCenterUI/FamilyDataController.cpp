#include "stdafx.h"

#include <shellapi.h>

#include "Network/EncodeHelper.h"
#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"

#include "FamilyDataCenterUI/family_background/FamilyBackground.h"
#include "FamilyDataCenterUI/AuthorityHelper.h"

#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/callback.h"
#include "third_party/chromium/base/bind.h"

//class SingerSummaryData
//{
//public:
//    uint32 singerid;
//    uint32 roomid;
//    std::string nickname;
//    std::string singerlevel;    //主播等级
//    uint32 onlinecount;         //开播次数
//    uint32 onlineminute;        //累计直播时长（分钟）
//    std::string total_hours;    // 累计直播时长(小时)
//    std::string pc_hours;
//    std::string phone_hours;
//    uint32 effectivecount;      //有效直播次数（大于1个小时）
//    uint32 maxusers;            //直播间最高人气
//    double revenue;             // 星豆收入
//};

namespace{
    const wchar_t* patternName = L"pattern.xlsx";
    
    bool SignedTimeCheck(const base::Time& begintime, const std::string& assing_time)
    {
        base::Time time = base::Time::Now();
        if (!DateTimeStringToBaseTime(assing_time, &time))
            return false;

        // 因为上月转入艺人，会显示在下月1号的00:00:01秒签约，这里要当有效艺人处理。
        return time < (begintime + base::TimeDelta::FromSeconds(10));
    }


    bool SingerSummaryDataToGridData(
        const std::map<uint32, SingerSummaryData>& singerSummaryData,
        GridData* griddata)
    {
        if (!griddata)
            return false;

        for (const auto& map_it:singerSummaryData)
        {
            const auto& it = map_it.second;
            RowData rowdata;
            rowdata.push_back(base::UintToString16(it.singerid));
            rowdata.push_back(base::UTF8ToWide(it.nickname));
            rowdata.push_back(base::UTF8ToWide(it.singerlevel));
            rowdata.push_back(base::UintToString16(it.onlinecount));
            rowdata.push_back(base::UintToString16(it.onlineminute));
            rowdata.push_back(base::UTF8ToWide(it.total_hours));
            rowdata.push_back(base::UTF8ToWide(it.pc_hours));
            rowdata.push_back(base::UTF8ToWide(it.phone_hours));
            rowdata.push_back(base::UintToString16(it.effectivecount));
            rowdata.push_back(base::UintToString16(it.effectivedays));
            rowdata.push_back(base::UintToString16(it.maxusers));
            std::wstring revenue = base::UTF8ToWide(base::DoubleToString(it.revenue));
            rowdata.push_back(revenue);
            griddata->push_back(rowdata);
        }

        return true;
    }

    bool SingerDailyDataToGridData(
        const std::vector<SingerDailyData>& singerDailyData,
        GridData* griddata)
    {
        if (!griddata)
            return false;

        for (const auto& it : singerDailyData)
        {
            RowData rowdata;
            rowdata.push_back(base::UTF8ToWide(it.date));
            rowdata.push_back(base::UintToString16(it.onlinecount));
            rowdata.push_back(base::UintToString16(it.onlineminute));
            rowdata.push_back(base::UintToString16(it.effectivecount));
            rowdata.push_back(base::UintToString16(it.maxusers));
            std::wstring revenue = base::UTF8ToWide(base::DoubleToString(it.revenue));            
            rowdata.push_back(revenue);
            rowdata.push_back(base::UintToString16(it.blame));
            griddata->push_back(rowdata);
        }

        return true;
    }

    COleVariant
        covtrue((short)TRUE),
        covfalse((short)FALSE),
        covoptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);
}

FamilyDataController::FamilyDataController()
    :authority_(new FamilyDataAuthority)
{
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    exePath_ = path;
    familyBackground_.reset(new FamilyBackground);


}

FamilyDataController::~FamilyDataController()
{
}

bool FamilyDataController::LoadAuthority(std::wstring* diaplay_message)
{
    AuthorityHelper authority_helper;
    if (!authority_helper.LoadFailyaDataAuthority(authority_.get()))
    {
        MessageBoxW(NULL, L"软件未授权", L"错误", 0);
    }

    return authority_helper.GetFailyaDataAuthorityDisplayInfo(*authority_.get(), diaplay_message);
}

bool FamilyDataController::InnerLogin(const std::string& username,
    const std::string& password)
{
    if (!familyBackground_)
        return false;

    // 用户没授权
    if (username != authority_->username)
        return false;

    if (!familyBackground_->Init(authority_->family_data_host))
        return false;

    if (!familyBackground_->Login(username, password))
        return false;

    base::Time expired_time = base::Time::FromInternalValue(authority_->expiretime);
    base::Time server_time = base::Time::Now();
    if (!familyBackground_->GetServerTime(&server_time))
        return false;

    std::string test_expired = MakeFormatDateString(expired_time);

    std::string test_server = MakeFormatDateString(server_time);

    // 用户授权已过期
    if (server_time > expired_time)
        return false;

    return true;
}

bool FamilyDataController::Login(const std::wstring& wusername,
    const std::wstring& wpassword)
{
    std::string username;
    std::string password;
    base::WideToUTF8(wusername.c_str(), wusername.length(), &username);
    base::WideToUTF8(wpassword.c_str(), wpassword.length(), &password);

    return InnerLogin(username, password);
}

bool FamilyDataController::GetSingerFamilyData(
    const base::Time& begintime,
    const base::Time& endtime,
    GridData* griddata)
{
    if (!familyBackground_)
        return false;

    std::vector<SingerSummaryData> singer_summary;
    if (!familyBackground_->GetSummaryData(begintime, endtime, &singer_summary))
    {
        return false;
    }

    for (auto singer : singer_summary)
    {
        auto find_result = singer_summary_map_.find(singer.singerid);
        if (find_result != singer_summary_map_.end())
        {
            uint32 effectivedays = find_result->second.effectivedays;
            singer.effectivedays = effectivedays;
            singer_summary_map_[singer.singerid] = singer;
        }
        else
        {
            singer_summary_map_[singer.singerid] = singer;
        }
    }
    
    if (!SingerSummaryDataToGridData(singer_summary_map_, griddata))
    {
        return false;
    }

    display_griddata_ = *griddata;

    return true;
}


bool FamilyDataController::GetDailyDataBySingerId(uint32 singerid,
    const base::Time& begintime,
    const base::Time& endtime, GridData* griddata, uint32* onlineminute,
    uint32* effect_day, double* revenue)
{
    std::vector<SingerDailyData> singerDailyData;
    if (!familyBackground_->GetDailyDataBySingerId(singerid, begintime, endtime,
        &singerDailyData))
    {
        return false;
    }

    for (const auto& daily : singerDailyData)
    {
        if (daily.onlineminute > 60)
            (*effect_day)++;

        (*revenue) += daily.revenue;
        (*onlineminute) += daily.onlineminute;
    }

    if (!SingerDailyDataToGridData(singerDailyData, griddata))
    {
        return false;
    }

    return true;
}

bool FamilyDataController::GetFamilyEffectiveDayCountSummary(
    const base::Time& begintime, const base::Time& endtime, 
    GridData* griddata, uint32* effect_count)
{
    std::vector<uint32> singerids;
    auto callback = base::Bind(&SignedTimeCheck, begintime);
    if (!familyBackground_->GetNormalSingerIds(&singerids, callback))
        return false;

    std::vector<SingerSummaryData> singer_summary;
    uint32 failed_count = 0;
    for (auto singerid : singerids)
    {
        std::vector<SingerDailyData> singerDailyData;
        if (!familyBackground_->GetDailyDataBySingerId(singerid, begintime, endtime,
            &singerDailyData))
        {
            failed_count++;
            continue;
        }

        SingerSummaryData summary_data;
        summary_data.singerid = singerid;
        for (auto daily : singerDailyData)
        {
            if (daily.onlineminute>60)
                summary_data.effectivedays++;
        }
        singer_summary.push_back(summary_data);
    }

    for (auto singer : singer_summary)
    {
        auto find_result = singer_summary_map_.find(singer.singerid);
        if (find_result != singer_summary_map_.end())
        {
            uint32 effectivedays = singer.effectivedays;
            if (effectivedays >= 20)
                (*effect_count)++;

            singer_summary_map_[singer.singerid].effectivedays = effectivedays;
        }
    }

    SingerSummaryDataToGridData(singer_summary_map_, griddata);

    display_griddata_ = *griddata;

    return true;
}


bool FamilyDataController::ExportToExcel()
{
    base::FilePath templatepath = exePath_.Append(L"template.xlsx");
    uint64 time64 = base::Time::Now().ToInternalValue();
    std::wstring timestr = base::Uint64ToString16(time64);
    std::wstring filename = timestr + L".xlsx";
    base::FilePath excelpath = exePath_.Append(filename);

    GridData griddata = display_griddata_;

    ExcelHelper excelHelper;
    excelHelper.Init(templatepath.value());
    excelHelper.Create(excelpath.value());
    excelHelper.Open(excelpath.value());
    excelHelper.Export(griddata);
    excelHelper.Close();
    return true;
}

bool FamilyDataController::ExportToTxt()
{
    GridData griddata = display_griddata_;

    base::FilePath txtpath;
    txtpath = exePath_.Append(L"exportdata.txt");

    std::string newline = "\n";
    base::File file(txtpath, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    for (const auto& rowdata : griddata)
    {
        // 每一行数据
        std::wstring wlinedata;
        for (const auto& item : rowdata)
        {
            wlinedata += item + L"\t";
        }
        std::string linedata = base::WideToUTF8(wlinedata);
        file.WriteAtCurrentPos(linedata.c_str(), linedata.size());
        file.WriteAtCurrentPos(newline.c_str(), newline.size());
    }
    file.Close();
    
    std::wstring openPath = txtpath.value();
    ShellExecuteW(0, L"open", L"NOTEPAD.EXE", openPath.c_str(), L"", SW_SHOWNORMAL);
    return true;
}
