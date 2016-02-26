#include "stdafx.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"


FamilyDataModle::FamilyDataModle()
{
}


FamilyDataModle::~FamilyDataModle()
{
}


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
}

bool ExcelHelper::Init()
{
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
    book_.AttachDispatch(books_.Add(covoptional));
    book_.SaveCopyAs(_variant_t(filepath.c_str()));
    book_.Close(covtrue, _variant_t(filepath.c_str()), covtrue);
    books_.Close();

    book_.ReleaseDispatch();
    books_.ReleaseDispatch();
    return false;
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

bool ExcelHelper::ExportToExcel(std::vector<std::vector<std::wstring>> data)
{
    return false;
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