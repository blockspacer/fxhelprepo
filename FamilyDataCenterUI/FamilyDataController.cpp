#include "stdafx.h"

#include <shellapi.h>

#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"

#include "FamilyDataCenterUI/family_background/FamilyBackground.h"

#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/files/file.h"

//
//struct SingerSummaryData
//{
//    uint32 singerid;
//    uint32 roomid;
//    std::string nickname;
//    std::string singerlevel;    //主播等级
//    uint32 onlinecount;         //开播次数
//    uint32 onlineminute;        //累计直播时长（分钟）
//    uint32 effectivecount;      //有效直播次数（大于1个小时）
//    uint32 maxusers;            //直播间最高人气
//    double revenue;             // 星豆收入
//};
//

namespace{
    const wchar_t* patternName = L"pattern.xlsx";
    

    bool SingerSummaryDataToGridData(
        const std::vector<SingerSummaryData>& singerSummaryData,
        GridData* griddata)
    {
        if (!griddata)
            return false;

        for (const auto& it:singerSummaryData)
        {
            RowData rowdata;
            rowdata.push_back(base::UTF8ToWide(it.nickname));
            rowdata.push_back(base::UTF8ToWide(it.singerlevel));
            rowdata.push_back(base::UintToString16(it.onlinecount));
            rowdata.push_back(base::UintToString16(it.onlineminute));
            rowdata.push_back(base::UintToString16(it.effectivecount));
            rowdata.push_back(base::UintToString16(it.maxusers));
            std::wstring revenue = base::UTF8ToWide(base::DoubleToString(it.revenue));
            rowdata.push_back(revenue);
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
{
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    exePath_ = path;
    familyBackground_.reset(new FamilyBackground);
}

FamilyDataController::~FamilyDataController()
{
}

bool FamilyDataController::Login(const std::string& username, 
    const std::string& password)
{
    if (!familyBackground_)
        return false;
    if (!familyBackground_->Init())
        return false;

    if (!familyBackground_->Login(username, password))
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
    if (!familyBackground_)
        return false;
    if (!familyBackground_->Init())
    {
        return false;
    }
    if (!familyBackground_->Login(username, password))
    {
        return false;
    }
    return true;
}

bool FamilyDataController::GetSingerFamilyData(
    const base::Time& begintime,
    const base::Time& endtime,
    GridData* griddata)
{
    if (!familyBackground_)
        return false;

    singerSummaryData_.reset(new std::vector<SingerSummaryData>);
    std::vector<SingerSummaryData> singerSummaryData;
    if (!familyBackground_->GetSummaryData(begintime, endtime, &singerSummaryData))
    {
        return false;
    }
    
    if (!SingerSummaryDataToGridData(singerSummaryData, griddata))
    {
        return false;
    }
    singerSummaryData_->assign(singerSummaryData.begin(), singerSummaryData.end());
    return true;
}

bool FamilyDataController::ExportToExcel()
{
    base::FilePath templatepath = exePath_.Append(L"exportdata.xlsx");
    uint64 time64 = base::Time::Now().ToInternalValue();
    std::wstring timestr = base::Uint64ToString16(time64);
    std::wstring filename = timestr + L".xlsx";
    base::FilePath excelpath = exePath_.Append(filename);

    GridData griddata;
    if (!SingerSummaryDataToGridData(*singerSummaryData_, &griddata))
    {
        return false;
    }

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
    GridData griddata;
    if (!SingerSummaryDataToGridData(*singerSummaryData_, &griddata))
    {
        return false;
    }
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
