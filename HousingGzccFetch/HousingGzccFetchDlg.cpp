
// HousingGzccFetchDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HousingGzccFetch.h"
#include "HousingGzccFetchDlg.h"
#include "afxdialogex.h"

#include "Network/EncodeHelper.h"
#include "ExcelHelper.h"
#include "HousingRequest.h"

#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/files/file.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHousingGzccFetchDlg 对话框



CHousingGzccFetchDlg::CHousingGzccFetchDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHousingGzccFetchDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHousingGzccFetchDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_PAGE_NUMBERS, m_edit_max_pages);
    DDX_Control(pDX, IDC_LIST_DISPLAY, m_list_display);
}

BEGIN_MESSAGE_MAP(CHousingGzccFetchDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_FETCH_DATA, &CHousingGzccFetchDlg::OnBnClickedBtnFetchData)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_DISPLAY, &CHousingGzccFetchDlg::OnLvnGetdispinfoListSummaryData)
    ON_BN_CLICKED(IDC_BTN_EXPORT, &CHousingGzccFetchDlg::OnBnClickedBtnExport)
END_MESSAGE_MAP()


// CHousingGzccFetchDlg 消息处理程序

BOOL CHousingGzccFetchDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    exePath_ = path;

    LOG(INFO) << __FUNCTION__;

    DWORD dwStyle = m_list_display.GetExtendedStyle();
    dwStyle |= LVS_REPORT;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
    dwStyle |= LVS_OWNERDATA;
    dwStyle |= LVS_AUTOARRANGE;
    m_list_display.SetExtendedStyle(dwStyle); //设置扩展风格
    m_edit_max_pages.SetWindowTextW(L"1");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHousingGzccFetchDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHousingGzccFetchDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CHousingGzccFetchDlg::OnBnClickedBtnFetchData()
{
    // TODO:  在此添加控件通知处理程序代码

    CString cs_max_pages;
    m_edit_max_pages.GetWindowTextW(cs_max_pages);
    std::string s_max_page = base::WideToUTF8(cs_max_pages.GetBuffer());
    uint32 max_pages = 1;
    if (!s_max_page.empty())
    {
        base::StringToUint(s_max_page, &max_pages);
    }

    HousingRequest house_request;
    std::vector<std::string> headers;
    std::list<std::vector<std::string>> record_list;
    if (!house_request.GetYszResult(&headers, &record_list, max_pages))
        return;
    
    std::vector<std::wstring> columnlist;
    for (const auto& header : headers)
    {
        columnlist.push_back(GBKToWide(header));
    }

    GridData grid_data;
    for (const auto& record : record_list)
    {
        RowData row_data;
        for (const auto& item : record)
            row_data.push_back(GBKToWide(item));

        grid_data.push_back(row_data);
    }
    DisplayDataToGrid(columnlist, grid_data);
    m_griddata = grid_data;
}

void CHousingGzccFetchDlg::OnBnClickedBtnExport()
{
    ExportToExcel();
}


void CHousingGzccFetchDlg::OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM *pItem = &(pDispInfo)->item;
    if (pItem->mask & LVIF_TEXT)
    {
        //使缓冲区数据与表格子项对应
        pItem->pszText = (LPWSTR)m_griddata[pItem->iItem][pItem->iSubItem].c_str();
    }

    *pResult = 0;
}

void CHousingGzccFetchDlg::DisplayDataToGrid(
    const std::vector<std::wstring> columnlist,
    const GridData& griddata)
{
    if (griddata.empty())
        return;
    m_griddata = griddata;

    //删除之前的数据
    m_list_display.SetItemCountEx(0);
    m_list_display.Invalidate();
    m_list_display.UpdateWindow();

    int nColumnCount = m_list_display.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_list_display.DeleteColumn(i);

    uint32 i = 0;
    for (const auto& it : columnlist)
    {
        m_list_display.InsertColumn(i++, it.c_str(), LVCFMT_LEFT, 100);//插入列
    }

    //生成新的数据缓冲区
    int nItemCount = griddata.size();
    m_list_display.SetItemCountEx(nItemCount);
    m_list_display.Invalidate();
}

bool CHousingGzccFetchDlg::ExportToExcel()
{
    base::FilePath templatepath = exePath_.Append(L"template.xlsx");
    uint64 time64 = base::Time::Now().ToInternalValue();
    std::wstring timestr = base::Uint64ToString16(time64);
    std::wstring filename = L"数据_" + timestr + L".xlsx";
    base::FilePath excelpath = exePath_.Append(filename);

    ExcelHelper excelHelper;
    if (!excelHelper.Init(templatepath.value()))
        return false;

    if (!excelHelper.Create(excelpath.value()))
        return false;
    
    if (!excelHelper.Open(excelpath.value()))
        return false;

    if (!excelHelper.Export(m_griddata))
        return false;

    if (!excelHelper.Close())
        return false;

    return true;
}
