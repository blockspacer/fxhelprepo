#include "stdafx.h"

#include <shellapi.h>

#include "FamilyDataCenterUI/FamilyDataController.h"

#include "FamilyDataCenterUI/Application.h"
#include "FamilyDataCenterUI/Range.h"
#include "FamilyDataCenterUI/Workbook.h"
#include "FamilyDataCenterUI/Workbooks.h"
#include "FamilyDataCenterUI/Worksheet.h"
#include "FamilyDataCenterUI/Worksheets.h"

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
    CApplication ExcelApp;
    CWorkbooks books;
    CWorkbook book;
    CWorksheets sheets;
    CWorksheet sheet;
    CRange range;
    LPDISPATCH lpDisp = NULL;

    //创建Excel 服务器(启动Excel)
    if (!ExcelApp.CreateDispatch(_T("Excel.Application"), NULL))
    {
        AfxMessageBox(_T("启动Excel服务器失败!"));
        return false;
    }

    /*判断当前Excel的版本*/
    CString strExcelVersion = ExcelApp.get_Version();
    int iStart = 0;
    strExcelVersion = strExcelVersion.Tokenize(_T("."), iStart);
    if (_T("11") == strExcelVersion)
    {
        AfxMessageBox(_T("当前Excel的版本是2003。"));
    }
    else if (_T("12") == strExcelVersion)
    {
        AfxMessageBox(_T("当前Excel的版本是2007。"));
    }
    else if (_T("14") == strExcelVersion)
    {
        AfxMessageBox(_T("当前Excel的版本是2010。"));
    }
    else
    {
        AfxMessageBox(_T("当前Excel的版本是其他版本。"));
    }

    ExcelApp.put_Visible(TRUE);
    ExcelApp.put_UserControl(FALSE);

    /*得到工作簿容器*/
    books.AttachDispatch(ExcelApp.get_Workbooks());

    /*打开一个工作簿，如不存在，则新增一个工作簿*/
    std::wstring patternPathFile = exePath_.Append(base::FilePath(patternName)).value();
    try
    {
        /*打开一个工作簿*/
        lpDisp = books.Open(patternPathFile.c_str(),
            vtMissing, vtMissing, vtMissing, vtMissing, vtMissing,
            vtMissing, vtMissing, vtMissing, vtMissing, vtMissing,
            vtMissing, vtMissing, vtMissing, vtMissing);
        book.AttachDispatch(lpDisp);
    }
    catch (...)
    {
        AfxMessageBox(_T("无法打开模板文件"));
        return false;
    }


    /*得到工作簿中的Sheet的容器*/
    sheets.AttachDispatch(book.get_Sheets());

    /*打开一个Sheet，如不存在，就新增一个Sheet*/
    CString strSheetName = _T("FamilyData");

    try
    {
        /*打开一个已有的Sheet*/
        lpDisp = sheets.get_Item(_variant_t(strSheetName));
        sheet.AttachDispatch(lpDisp);
    }
    catch (...)
    {
        /*创建一个新的Sheet*/
        lpDisp = sheets.Add(vtMissing, vtMissing, _variant_t((long)1), vtMissing);
        sheet.AttachDispatch(lpDisp);
        sheet.put_Name(strSheetName);
    }

    GridData griddata;
    if (!SingerSummaryDataToGridData(*singerSummaryData_.get(), &griddata))
    {
        return false;
    }

    uint32 column = 0;
    uint32 rowcount = griddata.size();
    if (!griddata.empty())
    {
        column = griddata.begin()->size();
    }

    if ((column <= 0) || (rowcount <= 0))
    {
        return false;
    }

    //向Sheet中写入多个单元格
    char c = 'A' + column - 1;
    std::string rangeend;
    rangeend.push_back(c);
    rangeend += base::UintToString(rowcount+1);

    lpDisp = sheet.get_Range(_variant_t("A2"), _variant_t(rangeend.c_str()));
    range.AttachDispatch(lpDisp);

    // 下面全部都需要修改
    VARTYPE vt = VT_I4; /*数组元素的类型，long*/
    SAFEARRAYBOUND sabWrite[2]; /*用于定义数组的维数和下标的起始值*/
    sabWrite[0].cElements = 10;
    sabWrite[0].lLbound = 0;
    sabWrite[1].cElements = 10;
    sabWrite[1].lLbound = 0;

    COleSafeArray olesaWrite;
    olesaWrite.Create(vt, sizeof(sabWrite) / sizeof(SAFEARRAYBOUND), sabWrite);

    /*通过指向数组的指针来对二维数组的元素进行间接赋值*/
    long(*pArray)[2] = NULL;
    olesaWrite.AccessData((void **)&pArray);
    memset(pArray, 0, sabWrite[0].cElements * sabWrite[1].cElements * sizeof(long));

    /*释放指向数组的指针*/
    olesaWrite.UnaccessData();
    pArray = NULL;

    /*对二维数组的元素进行逐个赋值*/
    long index[2] = { 0, 0 };
    long lFirstLBound = 0;
    long lFirstUBound = 0;
    long lSecondLBound = 0;
    long lSecondUBound = 0;
    olesaWrite.GetLBound(1, &lFirstLBound);
    olesaWrite.GetUBound(1, &lFirstUBound);
    olesaWrite.GetLBound(2, &lSecondLBound);
    olesaWrite.GetUBound(2, &lSecondUBound);
    for (long i = lFirstLBound; i <= lFirstUBound; i++)
    {
        index[0] = i;
        for (long j = lSecondLBound; j <= lSecondUBound; j++)
        {
            index[1] = j;
            long lElement = i * sabWrite[1].cElements + j;
            olesaWrite.PutElement(index, &lElement);
        }
    }

    /*把ColesaWritefeArray变量转换为VARIANT,并写入到Excel表格中*/
    VARIANT varWrite = (VARIANT)olesaWrite;
    range.put_Value2(varWrite);

    /*根据文件的后缀名选择保存文件的格式*/
    CString strSaveAsName = _T("d:\\new.xlsx");
    XlFileFormat NewFileFormat = xlOpenXMLWorkbook;

    book.SaveAs(_variant_t(strSaveAsName), _variant_t((long)NewFileFormat), 
        vtMissing, vtMissing, vtMissing, vtMissing, 0, vtMissing, vtMissing, 
        vtMissing, vtMissing, vtMissing);


    /*释放资源*/
    sheet.ReleaseDispatch();
    sheets.ReleaseDispatch();
    book.ReleaseDispatch();
    books.ReleaseDispatch();
    ExcelApp.Quit();
    ExcelApp.ReleaseDispatch();
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
