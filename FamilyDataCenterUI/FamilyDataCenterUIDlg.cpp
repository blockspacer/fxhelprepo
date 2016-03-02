
// FamilyDataCenterUIDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "afxdialogex.h"
#include "FamilyDataCenterUI/FamilyDataCenterUI.h"
#include "FamilyDataCenterUI/FamilyDataCenterUIDlg.h"
#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"
#include "FamilyDataCenterUI/Config.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
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
    DDX_Control(pDX, IDC_LIST_MESSAGE, m_list_message);
    DDX_Check(pDX, IDC_CHECK_REMEMBER, m_remember);
}

BEGIN_MESSAGE_MAP(CFamilyDataCenterUIDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_GET_FAMILY_DATA, &CFamilyDataCenterUIDlg::OnBnClickedGetFamilyData)
    ON_BN_CLICKED(IDC_BTN_EXPORT_TO_EXCEL, &CFamilyDataCenterUIDlg::OnBnClickedBtnExportToExcel)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CFamilyDataCenterUIDlg::OnBnClickedBtnLogin)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_SUMMARY_DATA, &CFamilyDataCenterUIDlg::OnLvnGetdispinfoListSummaryData)
END_MESSAGE_MAP()


// CFamilyDataCenterUIDlg ��Ϣ��������

BOOL CFamilyDataCenterUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵������ӵ�ϵͳ�˵��С�

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

	// TODO:  �ڴ����Ӷ���ĳ�ʼ������
    DWORD dwStyle = m_ListCtrl_SummaryData.GetExtendedStyle();
    dwStyle |= LVS_REPORT;
    dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
    dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��
    dwStyle |= LVS_OWNERDATA;
    dwStyle |= LVS_AUTOARRANGE;
    m_ListCtrl_SummaryData.SetExtendedStyle(dwStyle); //������չ���

    std::vector<std::wstring> columnlist = {
        L"����",
        L"�����ȼ�",
        L"��������",
        L"�ۼ�ֱ��",
        L"��Чֱ��",
        L"ֱ�����������",
        L"�Ƕ�����"
    };

    uint32 i = 0;
    for (const auto& it : columnlist)
    {
        m_ListCtrl_SummaryData.InsertColumn(i++, it.c_str(), LVCFMT_LEFT, 100);//������
    }
    familyDataController_.reset(new FamilyDataController);
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

// �����Ի���������С����ť������Ҫ����Ĵ���
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
    bool result = familyDataController_->GetSingerFamilyData(beginTime, endTime, &griddata);
    if (!result)
    {
        DisplayMessage(L" GetFamilyData failed!");
        return;
    }

    DisplayMessage(L" GetFamilyData success!");
    DisplayDataToGrid(griddata);
}

void CFamilyDataCenterUIDlg::DisplayDataToGrid(const GridData& griddata)
{
    if (griddata.empty())
        return;
    m_griddata = griddata;

    //ɾ��֮ǰ������
    m_ListCtrl_SummaryData.SetItemCountEx(0);
    m_ListCtrl_SummaryData.Invalidate();
    m_ListCtrl_SummaryData.UpdateWindow();

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
        //ʹ��������������������Ӧ
        pItem->pszText = (LPWSTR)m_griddata[pItem->iItem][pItem->iSubItem].c_str();
    }

    *pResult = 0;
}