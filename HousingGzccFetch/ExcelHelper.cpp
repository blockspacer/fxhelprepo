#include "stdafx.h"

#include "ExcelHelper.h"
#include <iterator>
#include <assert.h>
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

ExcelHelper::ExcelHelper()
{

}

ExcelHelper::~ExcelHelper()
{

}

namespace
{
    COleVariant
        covtrue((short)TRUE),
        covfalse((short)FALSE),
        covoptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

    std::wstring TranslateToLocationString(uint32 rowid, uint32 columnid)
    {
        std::wstring location;
        uint32 times = columnid / 26;
        uint32 offset = columnid % 26;
        if (times>26)
        {
            assert(false && L"参数值过大");
            return L"";
        }

        if (times>0)
        {
            location.push_back(L'A' + times - 1);
        }
        
        location.push_back(L'A' + offset);
        location += base::UintToString16(rowid);
        return location;
    }
}

bool ExcelHelper::Init(const std::wstring& filepath)
{
    templatepath_ = filepath;
    if (!app_.CreateDispatch(L"Excel.Application"))
    {
        return false;
    }
    app_.put_Visible(FALSE);
    return true;
}

bool ExcelHelper::Create(const std::wstring& filepath)
{
    books_.AttachDispatch(app_.get_Workbooks());

    /*打开一个工作簿*/
    LPDISPATCH lpDisp = books_.Open(templatepath_.c_str(),
                        covoptional, covfalse, covoptional, covoptional, covoptional,
                        covoptional, covoptional, covoptional, covoptional, covoptional,
                        covoptional, covoptional, covoptional, covoptional);
        
    if (!lpDisp)
    {
        AfxMessageBox(_T("无法打开模板文件"));
        return false;
    }
    
    book_.AttachDispatch(lpDisp);
    book_.SaveCopyAs(_variant_t(filepath.c_str()));
    book_.Close(covtrue, _variant_t(filepath.c_str()), covtrue);
    books_.Close();

    book_.ReleaseDispatch();
    books_.ReleaseDispatch();
    return true;
}

bool ExcelHelper::Open(const std::wstring& filepath)
{
    books_.AttachDispatch(app_.get_Workbooks());

    auto lpdisp = books_.Open(
        filepath.c_str(), 
        covoptional, covfalse, covoptional, covoptional, covoptional,
        covoptional, covoptional, covoptional, covoptional, covoptional,
        covoptional, covoptional, covoptional, covoptional);

    if (!lpdisp)
    {
        return false;
    }
    //get workbook  
    book_.AttachDispatch(lpdisp);

    //get worksheets  
    sheets_.AttachDispatch(book_.get_Worksheets());

    OpenSheet(L"FamilyData", &sheet_);
    return true;

}

bool ExcelHelper::Close()
{
    book_.Save();
    book_.Close(covtrue, covoptional, covoptional);

    book_.ReleaseDispatch();
    books_.ReleaseDispatch();

    return true;
}

bool ExcelHelper::Export(std::vector<std::vector<std::wstring>> data)
{
    // 填写数据
    for (auto rowdata = data.begin(); rowdata != data.end(); rowdata++)
    {
        // excel里面的行下标从1开始,还要跳过列头，所以是+2
        uint32 rowid = std::distance(data.begin(), rowdata)+2;
        std::wstring wlinedata;
        for (auto columndata = rowdata->begin(); columndata!=rowdata->end();
            columndata++)
        {
            uint32 columnid = std::distance(rowdata->begin(), columndata);
            SetString(rowid, columnid, *columndata);
        }
    }

    return true;
}

bool ExcelHelper::OpenSheet(const std::wstring& sheetname, CWorksheet* sheet)
{
    LPDISPATCH lpdisp = nullptr;
    for (short i = 1; i <= sheets_.get_Count(); i++)
    {
        COleVariant v((short)i);
        lpdisp = sheets_.get_Item(v);
        sheet->AttachDispatch(lpdisp);
        std::wstring sheet_name = sheet->get_Name();
        if (sheet_name.compare(sheetname) == 0)
        {
            return true;
        }
    }

    /*创建一个新的Sheet*/
    lpdisp = sheets_.Add(covoptional, covoptional, _variant_t((long)1), covoptional);
    sheet->AttachDispatch(lpdisp);
    sheet->put_Name(sheetname.c_str());

    return true;
}

bool ExcelHelper::SetString(uint32 rowid, uint32 columnid, 
                            const std::wstring& data)
{
    std::wstring location = TranslateToLocationString(rowid, columnid);
    CRange range;
    COleVariant start((LPCTSTR)(location.c_str()));
    COleVariant end((LPCTSTR)(location.c_str()));
    range.AttachDispatch(sheet_.get_Range(start,end));
    COleVariant value((LPCTSTR)(data.c_str()));
    range.put_Value2(value);
    range.put_VerticalAlignment(COleVariant((long)-4108));
    range.put_HorizontalAlignment(COleVariant((long)-4108));
    range.ReleaseDispatch();

    return true;
}