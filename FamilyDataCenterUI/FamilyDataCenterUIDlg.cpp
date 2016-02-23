
// FamilyDataCenterUIDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "FamilyDataCenterUI/FamilyDataCenterUI.h"
#include "FamilyDataCenterUI/FamilyDataCenterUIDlg.h"
#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框
#include "third_party/chromium/base/time/time.h"

namespace
{
    bool OleDateTimeToBaseTime(const COleDateTime& oletime, base::Time* basetime)
    {
        if (!basetime)
            return false;

        SYSTEMTIME systemtime;
        if (!oletime.GetAsSystemTime(systemtime))
            return false;
        
        FILETIME filetime;
        if (!SystemTimeToFileTime(&systemtime, &filetime))
            return false;
        
        *basetime = base::Time::FromFileTime(filetime);
        return true;
    }
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CFamilyDataCenterUIDlg 对话框



CFamilyDataCenterUIDlg::CFamilyDataCenterUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFamilyDataCenterUIDlg::IDD, pParent)
    , familyDataController_(nullptr)
    , familyDataModle_(nullptr)
    , m_oleDateTime_Begin(COleDateTime::GetCurrentTime())
    , m_oleDateTime_End(COleDateTime::GetCurrentTime())
    , m_username(_T(""))
    , m_password(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFamilyDataCenterUIDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SUMMARY_DATA, m_ListCtrl_SummaryData);
    DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_BEGIN, m_oleDateTime_Begin);
    DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_END, m_oleDateTime_End);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_username);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
}

BEGIN_MESSAGE_MAP(CFamilyDataCenterUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_GET_FAMILY_DATA, &CFamilyDataCenterUIDlg::OnBnClickedGetFamilyData)
    ON_BN_CLICKED(IDC_BTN_EXPORT_TO_EXCEL, &CFamilyDataCenterUIDlg::OnBnClickedBtnExportToExcel)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CFamilyDataCenterUIDlg::OnBnClickedBtnLogin)
END_MESSAGE_MAP()


// CFamilyDataCenterUIDlg 消息处理程序

BOOL CFamilyDataCenterUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    DWORD dwStyle = m_ListCtrl_SummaryData.GetExtendedStyle();
    dwStyle |= LVS_REPORT;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
    m_ListCtrl_SummaryData.SetExtendedStyle(dwStyle); //设置扩展风格

    std::vector<std::wstring> columnlist = {
        L"主播",
        L"主播等级",
        L"开播次数",
        L"累计直播",
        L"有效直播",
        L"直播间最高人气",
        L"星豆收入"
    };

    uint32 i = 0;
    for (const auto& it : columnlist)
    {
        m_ListCtrl_SummaryData.InsertColumn(i++, it.c_str(), LVCFMT_LEFT, 140);//插入列
    }

    familyDataController_.reset(new FamilyDataController);
    familyDataModle_.reset(new FamilyDataModle);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFamilyDataCenterUIDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFamilyDataCenterUIDlg::OnPaint()
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
HCURSOR CFamilyDataCenterUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFamilyDataCenterUIDlg::OnBnClickedGetFamilyData()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);
    base::Time beginTime;
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_Begin, &beginTime);
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    GridData griddata;
    if (!familyDataController_->GetSingerFamilyData(beginTime, endTime, &griddata))
    {
        // 输入日志信息
        return;
    }
    DisplayDataToGrid(griddata);
}

void CFamilyDataCenterUIDlg::DisplayDataToGrid(const GridData& griddata)
{  
    // 这里要补充显示在界面表格里的数据功能

    UpdateData(FALSE);
    return;
}

void CFamilyDataCenterUIDlg::OnBnClickedBtnExportToExcel()
{
    // TODO:  在此添加控件通知处理程序代码
    familyDataController_->ExportToExcel();
}


void CFamilyDataCenterUIDlg::OnBnClickedBtnLogin()
{
    // TODO:  在此添加控件通知处理程序代码
    UpdateData(TRUE);
    familyDataController_->Login(m_username.GetString(), m_password.GetString());
}
