
// AuthorityDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Authority.h"
#include "AuthorityDlg.h"
#include "afxdialogex.h"
#include "AuthorityHelper.h"
#undef max
#undef min
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversion_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


namespace
{
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

    bool BaseTimeToOleDateTime(const  base::Time& basetime, COleDateTime* oletime)
    {
        if (!oletime)
            return false;

        base::Time::Exploded exploded;
        basetime.LocalExplode(&exploded);

        try
        {
            // �����ʽ����,���׳��쳣
            oletime->SetDateTime(exploded.year,
                exploded.month,
                exploded.day_of_month,
                exploded.hour,
                exploded.minute,
                exploded.second);
        }
        catch (CException* e)
        {
            return false;
        }
        return true;
    }
}
// CAuthorityDlg �Ի���

CAuthorityDlg::CAuthorityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAuthorityDlg::IDD, pParent)
    , m_oleDateTime_End(COleDateTime::GetCurrentTime())
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CAuthorityDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_USER_ID, m_edit_userid);
    DDX_Control(pDX, IDC_EDIT_ROOM_ID, m_edit_roomid);
    DDX_Control(pDX, IDC_EDIT_CLAN_ID, m_edit_clanid);
    DDX_Control(pDX, IDC_CHK_KICK_HOUR, m_chk_kickout);
    DDX_Control(pDX, IDC_CHK_SLIENT, m_banchat);
    DDX_Control(pDX, IDC_CHK_ANTI_ADVANCE, m_chk_anti_advance);
    DDX_DateTimeCtrl(pDX, IDC_DATETIMEPICKER1, m_oleDateTime_End);
    DDX_Control(pDX, IDC_EDIT_SERVER_IP, m_edit_serverip);
}

BEGIN_MESSAGE_MAP(CAuthorityDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDCANCEL, &CAuthorityDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_ANTIFLOOD_AUTHORITY, &CAuthorityDlg::OnBnClickedBtnAntiFloodAuthority)
    ON_BN_CLICKED(IDC_BTN_VIEW, &CAuthorityDlg::OnBnClickedBtnView)
    ON_BN_CLICKED(IDC_BTN_ADD_1_MON, &CAuthorityDlg::OnBnClickedBtnAdd1Mon)
    ON_BN_CLICKED(IDC_BTN_3_MON, &CAuthorityDlg::OnBnClickedBtn3Mon)
    ON_BN_CLICKED(IDC_BTN_6_MON, &CAuthorityDlg::OnBnClickedBtn6Mon)
    ON_BN_CLICKED(IDC_BTN_PACKAGE, &CAuthorityDlg::OnBnClickedBtnPackage)
    ON_BN_CLICKED(IDC_BTN_TRACK_AUTHORITY, &CAuthorityDlg::OnBnClickedBtnTrackAuthority)
    ON_BN_CLICKED(IDC_BTN_BACKGROUP_AUTHORITY, &CAuthorityDlg::OnBnClickedBtnBackgroupAuthority)
END_MESSAGE_MAP()


// CAuthorityDlg ��Ϣ�������

BOOL CAuthorityDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_NORMAL);

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    m_edit_serverip.SetWindowTextW(L"114.54.2.205");
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAuthorityDlg::OnOK()
{
}

void CAuthorityDlg::OnCancel()
{
}
// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAuthorityDlg::OnPaint()
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
HCURSOR CAuthorityDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAuthorityDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}

void CAuthorityDlg::OnBnClickedBtnAntiFloodAuthority()
{
    UpdateData(TRUE);
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    uint64 expiretime = endTime.ToInternalValue();
    
    CString csUserid = L"0";
    m_edit_userid.GetWindowTextW(csUserid);
    CString csRoomid = L"0";
    m_edit_roomid.GetWindowTextW(csRoomid);
    CString csClanid = L"0";
    m_edit_clanid.GetWindowTextW(csClanid);
    CString csServerIp = L"0";
    m_edit_serverip.GetWindowTextW(csServerIp);

    int kickout = m_chk_kickout.GetCheck();
    int banchat = m_banchat.GetCheck();
    int antiadvance = m_chk_anti_advance.GetCheck();

    AuthorityHelper authorityHelper;
    AntiFloodAuthority authority;
    base::StringToUint(base::WideToUTF8(csUserid.GetBuffer()), &authority.userid);
    base::StringToUint(base::WideToUTF8(csRoomid.GetBuffer()), &authority.roomid);
    base::StringToUint(base::WideToUTF8(csClanid.GetBuffer()), &authority.clanid);
    authority.kickout = kickout;
    authority.banchat = banchat;
    authority.antiadvance = antiadvance;
    authority.expiretime = expiretime;
    authority.serverip = base::WideToUTF8(csServerIp.GetBuffer());

    if (!authorityHelper.SaveAntiFloodAuthority(authority))
        return;
}


void CAuthorityDlg::OnBnClickedBtnView()
{
    AuthorityHelper authorityHelper;
    AntiFloodAuthority authority;
    if (!authorityHelper.LoadAntiFloodAuthority(&authority))
        return;

    CString csUserid = base::UTF8ToWide(
        base::UintToString(authority.userid)).c_str();
    m_edit_userid.SetWindowTextW(csUserid);

    CString csRoomid = base::UTF8ToWide(
        base::UintToString(authority.roomid)).c_str();
    m_edit_roomid.SetWindowTextW(csRoomid);

    CString csClanid = base::UTF8ToWide(
        base::UintToString(authority.clanid)).c_str();
    m_edit_clanid.SetWindowTextW(csClanid);

    m_chk_kickout.SetCheck(authority.kickout);
    m_banchat.SetCheck(authority.banchat);
    m_chk_anti_advance.SetCheck(authority.antiadvance);
    uint64 expiretime = authority.expiretime;

    base::Time endTime;
    endTime = base::Time::FromInternalValue(expiretime);
    base::Time::Exploded exploded;
    endTime.LocalExplode(&exploded);
    m_oleDateTime_End.SetDate(
        exploded.year, exploded.month, exploded.day_of_month);
    UpdateData(FALSE);
}


void CAuthorityDlg::OnBnClickedBtnAdd1Mon()
{
    UpdateData(TRUE);
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    uint64 expiretime = endTime.ToInternalValue();
    COleDateTimeSpan span;
    span.SetDateTimeSpan(30, 0, 0, 0);
    m_oleDateTime_End += span;
    UpdateData(FALSE);
}

void CAuthorityDlg::OnBnClickedBtn3Mon()
{
    UpdateData(TRUE);
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    uint64 expiretime = endTime.ToInternalValue();
    COleDateTimeSpan span;
    span.SetDateTimeSpan(30*3, 0, 0, 0);
    m_oleDateTime_End += span;
    UpdateData(FALSE);
}

void CAuthorityDlg::OnBnClickedBtn6Mon()
{
    UpdateData(TRUE);
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    uint64 expiretime = endTime.ToInternalValue();
    COleDateTimeSpan span;
    span.SetDateTimeSpan(30*6, 0, 0, 0);
    m_oleDateTime_End += span;
    UpdateData(FALSE);
}

void CAuthorityDlg::OnBnClickedBtnPackage()
{
    ::MessageBox(NULL, L"����δʵ��", L"��ʾ", 0);
}

void CAuthorityDlg::OnBnClickedBtnTrackAuthority()
{
    UpdateData(TRUE);
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    uint64 expiretime = endTime.ToInternalValue();

    CString csUserid = L"0";
    m_edit_userid.GetWindowTextW(csUserid);

    AuthorityHelper authorityHelper;
    UserTrackerAuthority authority;
    base::StringToUint(base::WideToUTF8(csUserid.GetBuffer()), &authority.user_id);
    authority.expiretime = expiretime;
    authority.tracker_host = "visitor.fanxing.kugou.com";

    if (!authorityHelper.SaveUserTrackerAuthority(authority))
        return;
}


void CAuthorityDlg::OnBnClickedBtnBackgroupAuthority()
{
    UpdateData(TRUE);
    base::Time endTime;
    OleDateTimeToBaseTime(m_oleDateTime_End, &endTime);
    uint64 expiretime = endTime.ToInternalValue();

    CString csUserName = L"";
    m_edit_userid.GetWindowTextW(csUserName);

    AuthorityHelper authorityHelper;
    FamilyDataAuthority authority;
    authority.username = base::WideToUTF8(csUserName.GetBuffer());
    authority.expiretime = expiretime;
    authority.family_data_host = "family.fanxing.kugou.com";

    if (!authorityHelper.SaveFamilyDataAuthority(authority))
        return;
}
