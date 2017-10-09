
// FamilyDataCenterUIDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "FamilyDataCenterUI/FamilyDataCenterUI.h"
#include "FamilyDataCenterUI/FamilyDataCenterUIDlg.h"
#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"
#include "FamilyDataCenterUI/Config.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/callback.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
#include "third_party/chromium/base/time/time.h"

namespace
{
    std::vector<std::wstring> family_columnlist = {
        L"����id",
        L"�����ǳ�",
        L"�����ȼ�",
        L"��������",
        L"�ۼ�ֱ��",
        L"ֱ��ʱ��",
        L"PCʱ��",
        L"�ֻ�ʱ��",
        L"ֱ������",
        L"��Ч��",
        L"�������",
        L"�Ƕ�����"
    };

    std::vector<std::wstring> singer_columnlist = {
        L"����",
        L"��������",
        L"����ʱ��",
        L"��Чֱ������",
        L"�������",
        L"�Ƕ�����",
        L"�����ۼƿ۷�"
    };

    bool OleDateTimeToBaseTime(const COleDateTime& oletime, base::Time* basetime)
    {
        if (!basetime)
            return false;

        SYSTEMTIME systemtime;
        if (!oletime.GetAsSystemTime(systemtime))
            return false;
        
        base::Time::Exploded exploded;
        exploded.year = systemtime.wYear;
        exploded.month = systemtime.wMonth;
        exploded.day_of_month = systemtime.wDay;
        exploded.day_of_week = systemtime.wDayOfWeek;
        exploded.hour = systemtime.wHour;
        exploded.minute = systemtime.wMinute;
        exploded.second = systemtime.wSecond;
        exploded.millisecond = systemtime.wMilliseconds;
        *basetime = base::Time::FromLocalExploded(exploded);
        return true;
    }

    bool SaveUserInfo(const std::wstring& username, const std::wstring& password,
        bool remember)
    {
        Config config;
        return config.Save(username, password, remember);
    }

    bool LoadUserInfo(std::wstring* username, std::wstring* password,bool* remeber)
    {
        Config config;
        bool result = config.GetUserName(username);
        config.GetPassword(password);
        *remeber = config.GetRemember();
        return result;
    }
}

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CFamilyDataCenterUIDlg �Ի���



CFamilyDataCenterUIDlg::CFamilyDataCenterUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFamilyDataCenterUIDlg::IDD, pParent)
    , familyDataController_(nullptr)
    , familyDataModle_(nullptr)
    , m_oleDateTime_Begin(COleDateTime::GetCurrentTime())
    , m_oleDateTime_End(COleDateTime::GetCurrentTime())
    , m_username(_T(""))
    , m_password(_T(""))
    , index_(0)
    , m_remember(FALSE)
    , m_singer_id(0)
    , m_total_income(0)
    , m_total_hours(0)
    , m_new_count(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    int day_of_month = 1;
    int month = m_oleDateTime_Begin.GetMonth();
    int year = m_oleDateTime_Begin.GetYear();
    m_oleDateTime_Begin.SetDate(year, month, day_of_month);
}

CFamilyDataCenterUIDlg::~CFamilyDataCenterUIDlg()
{
    if (familyDataController_)
    {
        familyDataController_->Finalize();
    }
}

void CFamilyDataCenterUIDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_SUMMARY_DATA, m_ListCtrl_SummaryData);
    DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_BEGIN, m_oleDateTime_Begin);
    DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER_END, m_oleDateTime_End);
    DDX_Text(pDX, IDC_EDIT_USERNAME, m_username);
    DDX_Text(pDX, IDC_EDIT_PASSWORD, m_password);
    DDX_Control(pDX, IDC_LIST_MESSAGE, m_list_message);
    DDX_Check(pDX, IDC_CHECK_REMEMBER, m_remember);
    DDX_Text(pDX, IDC_EDIT_SINGER_ID, m_singer_id);
    DDX_Text(pDX, IDC_EDIT_TOTAL_INCOME, m_total_income);
    DDX_Text(pDX, IDC_EDIT_TOTAL_HOURS, m_total_hours);
    DDX_Text(pDX, IDC_EDIT_NEW_COUNT, m_new_count);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
    DDX_Control(pDX, IDC_STATIC_PROGRESS, m_static_room_progress);
}

BEGIN_MESSAGE_MAP(CFamilyDataCenterUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_GET_FAMILY_DATA, &CFamilyDataCenterUIDlg::OnBnClickedGetFamilyData)
    ON_BN_CLICKED(IDC_BTN_EXPORT_TO_EXCEL, &CFamilyDataCenterUIDlg::OnBnClickedBtnExportToExcel)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CFamilyDataCenterUIDlg::OnBnClickedBtnLogin)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_SUMMARY_DATA, &CFamilyDataCenterUIDlg::OnLvnGetdispinfoListSummaryData)
    ON_BN_CLICKED(IDC_BTN_GET_SINGER_DATA, &CFamilyDataCenterUIDlg::OnBnClickedBtnGetSingerData)
    ON_BN_CLICKED(IDC_BTN_GET_NEW_SINGER, &CFamilyDataCenterUIDlg::OnBnClickedBtnGetSingerEffectiveDays)
    ON_MESSAGE(WM_USER_MSG, &CFamilyDataCenterUIDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_PROGRESS, &CFamilyDataCenterUIDlg::OnUpdateProgress)
    ON_MESSAGE(WM_USER_UPDATE_RESULT, &CFamilyDataCenterUIDlg::OnUpdateResult)
    ON_MESSAGE(WM_USER_UPDATE_EFFECT_SINGER, &CFamilyDataCenterUIDlg::OnUpdateEffectSingers)
END_MESSAGE_MAP()


// CFamilyDataCenterUIDlg ��Ϣ�������

BOOL CFamilyDataCenterUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    DWORD dwStyle = m_ListCtrl_SummaryData.GetExtendedStyle();
    dwStyle |= LVS_REPORT;
    dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
    dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
    dwStyle |= LVS_OWNERDATA;
    dwStyle |= LVS_AUTOARRANGE;
    m_ListCtrl_SummaryData.SetExtendedStyle(dwStyle); //������չ���

    familyDataController_.reset(new FamilyDataController);
    familyDataController_->Initialize();
    std::wstring display_msg;
    familyDataController_->LoadAuthority(&display_msg);
    if (!display_msg.empty())
    {
        SetWindowText(display_msg.c_str());
    }

    familyDataModle_.reset(new FamilyDataModle);

    std::wstring username;
    std::wstring password;
    bool remember = false;
    if (LoadUserInfo(&username, &password,&remember))
    {
        m_remember = static_cast<int>(remember);
        m_username = username.c_str();
        m_password = password.c_str();
        UpdateData(FALSE);
    }

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CFamilyDataCenterUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CFamilyDataCenterUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFamilyDataCenterUIDlg::OnBnClickedGetFamilyData()
{
    UpdateData(TRUE);
    base::Time beginTime;
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_Begin, &beginTime);
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    GridData griddata;
    DisplayMessage(L" GetFamilyData Begin!");
    bool result = familyDataController_->GetSingerFamilyData(beginTime, endTime, 
        base::Bind(&CFamilyDataCenterUIDlg::NotifyUpdateProgress, base::Unretained(this)),
        base::Bind(&CFamilyDataCenterUIDlg::NotifyUpdateResult, base::Unretained(this)));

    if (!result)
    {
        DisplayMessage(L" GetFamilyData failed!");
        return;
    }

    DisplayMessage(L" GetFamilyData success!");

    DisplayDataToGrid(family_columnlist, griddata);
}

void CFamilyDataCenterUIDlg::DisplayDataToGrid(
    const std::vector<std::wstring> columnlist, 
    const GridData& griddata)
{
    if (griddata.empty())
        return;
    m_griddata = griddata;

    //ɾ��֮ǰ������
    m_ListCtrl_SummaryData.SetItemCountEx(0);
    m_ListCtrl_SummaryData.Invalidate();
    m_ListCtrl_SummaryData.UpdateWindow();

    int nColumnCount = m_ListCtrl_SummaryData.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_SummaryData.DeleteColumn(i);

    uint32 i = 0;
    for (const auto& it : columnlist)
    {
        m_ListCtrl_SummaryData.InsertColumn(i++, it.c_str(), LVCFMT_LEFT, 60);//������
    }

    //�����µ����ݻ�����
    int nItemCount = griddata.size();
    m_ListCtrl_SummaryData.SetItemCountEx(nItemCount);
    m_ListCtrl_SummaryData.Invalidate();
}

void CFamilyDataCenterUIDlg::DisplayMessage(const std::wstring& message)
{
    m_list_message.InsertString(index_++, message.c_str());
    UpdateData(FALSE);
}

void CFamilyDataCenterUIDlg::NotifyMessageCallback(const std::wstring& message)
{
    if (message.empty())
        return;

    std::wstring* p_msg(new std::wstring(message));
    this->PostMessage(WM_USER_MSG, 0, (LPARAM)p_msg);
}

void CFamilyDataCenterUIDlg::NotifyUpdateProgress(uint32 current, uint32 all)
{
    this->PostMessage(WM_USER_PROGRESS, current, all);
}

void CFamilyDataCenterUIDlg::NotifyUpdateResult(const GridData& grid_data)
{
    GridData* p_grid_data = new GridData(grid_data);
    this->PostMessage(WM_USER_UPDATE_RESULT, 0, (LPARAM)(p_grid_data));
}

void CFamilyDataCenterUIDlg::NotifyUpdateEffectSingers(uint32 count)
{
    this->PostMessageW(WM_USER_UPDATE_EFFECT_SINGER, 0, (LPARAM)(count));
}

void CFamilyDataCenterUIDlg::OnBnClickedBtnExportToExcel()
{
    familyDataController_->ExportToTxt();

    DisplayMessage(L" ExportToExcel Begin!");
    bool result = familyDataController_->ExportToExcel();
    
    if (result)
    {
        DisplayMessage(L" Export success!");
    }
    else
    {
        DisplayMessage(L" Export failed!");
    }
}


void CFamilyDataCenterUIDlg::OnBnClickedBtnLogin()
{
    UpdateData(TRUE);
    DisplayMessage(std::wstring(m_username.GetString()) + L" Login Begin!");
    bool result = familyDataController_->Login(m_username.GetString(), m_password.GetString());
    if (!result)
    {
        DisplayMessage(std::wstring(m_username.GetString()) + L" Login failed!");
        return;
    }

    DisplayMessage(std::wstring(m_username.GetString()) + L" Login success!");
    if (m_remember)
    {
        SaveUserInfo(m_username.GetString(), m_password.GetString(), !!m_remember);
    }
    else
    {
        SaveUserInfo(L"", L"", !!m_remember);
    }
}

void CFamilyDataCenterUIDlg::OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM *pItem = &(pDispInfo)->item;
    if (pItem->mask & LVIF_TEXT)
    {
        //ʹ�������������������Ӧ
        pItem->pszText = (LPWSTR)m_griddata[pItem->iItem][pItem->iSubItem].c_str();
    }

    *pResult = 0;
}

//struct SingerDailyData
//{
//    uint32 singerid;
//    std::string date;
//    uint32 onlinecount;         //��������
//    uint32 onlineminute;        //�ۼ�ֱ��ʱ�������ӣ�
//    uint32 effectivecount;      //��Чֱ������������1��Сʱ��
//    uint32 maxusers;            //ֱ�����������
//    double revenue;             // �Ƕ�����
//    uint32 blame;               // �����ۼƿ۷�
//};
void CFamilyDataCenterUIDlg::OnBnClickedBtnGetSingerData()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    UpdateData(TRUE);
    if (!m_singer_id)
    {
        AfxMessageBox(L"��������ȷ���������Ǻ�");
        return;
    }
    base::Time beginTime;
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_Begin, &beginTime);
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    GridData griddata;
    DisplayMessage(L" GetDailyDataBySingerId Begin!");

    uint32 effect_day = 0;
    uint32 onlineminute = 0;
    double revenue = 0.0;
    bool result = familyDataController_->GetDailyDataBySingerId(
        m_singer_id, beginTime, 
        endTime, &griddata, &onlineminute, &effect_day, & revenue);

    if (!result)
    {
        DisplayMessage(L" GetDailyDataBySingerId failed!");
        return;
    }
    DisplayMessage(L" GetDailyDataBySingerId success!");

    std::wstring weffect_day = base::UintToString16(effect_day);
    SetDlgItemTextW(IDC_EDIT_EFFECT_DAYS, weffect_day.c_str());
    SetDlgItemTextW(IDC_EDIT_TOTAL_INCOME, base::UTF8ToUTF16(base::DoubleToString(revenue)).c_str());
    SetDlgItemInt(IDC_EDIT_TOTAL_HOURS, onlineminute);
    DisplayDataToGrid(singer_columnlist, griddata);
}


void CFamilyDataCenterUIDlg::OnBnClickedBtnGetSingerEffectiveDays()
{
    UpdateData(TRUE);
    base::Time beginTime;
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_Begin, &beginTime);
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);

    uint32 effect_count = 0;
    familyDataController_->GetFamilyEffectiveDayCountSummary(
        beginTime, endTime, 
        base::Bind(&CFamilyDataCenterUIDlg::NotifyUpdateProgress, base::Unretained(this)),
        base::Bind(&CFamilyDataCenterUIDlg::NotifyUpdateResult, base::Unretained(this)),
        base::Bind(&CFamilyDataCenterUIDlg::NotifyUpdateEffectSingers, base::Unretained(this)));
}

LRESULT CFamilyDataCenterUIDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::wstring* message = reinterpret_cast<std::wstring*>(lParam);
    DisplayMessage(*message);
    delete message;
    return 0;
}

LRESULT CFamilyDataCenterUIDlg::OnUpdateProgress(WPARAM wParam, LPARAM lParam)
{
    uint32 current = (uint32)(wParam);
    uint32 all = (uint32)(lParam);

    int pos = static_cast<int>(current*100.0 / all);
    std::wstring show_msg = base::UintToString16(current) +
        L" / " + base::UintToString16(all);

    m_progress1.SetPos(pos);
    m_static_room_progress.SetWindowTextW(show_msg.c_str());
    return 0;
}

LRESULT CFamilyDataCenterUIDlg::OnUpdateResult(WPARAM wParam, LPARAM lParam)
{
    GridData* griddata = (GridData*)(lParam);
    DisplayDataToGrid(family_columnlist, *griddata);
    delete griddata;
    return 0;
}

LRESULT CFamilyDataCenterUIDlg::OnUpdateEffectSingers(WPARAM wParam, LPARAM lParam)
{
    std::wstring effect_count = base::UintToString16(lParam);
    SetDlgItemTextW(IDC_EDIT_EFFECT_SINGER_COUNT, effect_count.c_str());
    return 0;
}
