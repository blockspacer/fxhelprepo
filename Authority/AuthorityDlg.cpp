
// AuthorityDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Authority.h"
#include "AuthorityDlg.h"
#include "afxdialogex.h"
#include "AuthorityHelper.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversion_utils.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAuthorityDlg 对话框



CAuthorityDlg::CAuthorityDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CAuthorityDlg::IDD, pParent)
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
}

BEGIN_MESSAGE_MAP(CAuthorityDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CAuthorityDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &CAuthorityDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_GENERATE, &CAuthorityDlg::OnBnClickedBtnGenerate)
    ON_BN_CLICKED(IDC_BTN_VIEW, &CAuthorityDlg::OnBnClickedBtnView)
END_MESSAGE_MAP()


// CAuthorityDlg 消息处理程序

BOOL CAuthorityDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MINIMIZE);

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CAuthorityDlg::OnPaint()
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
HCURSOR CAuthorityDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CAuthorityDlg::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnOK();
}


void CAuthorityDlg::OnBnClickedCancel()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}


void CAuthorityDlg::OnBnClickedBtnGenerate()
{
    CString csUserid = L"0";
    m_edit_userid.GetWindowTextW(csUserid);
    CString csRoomid = L"0";
    m_edit_roomid.GetWindowTextW(csRoomid);
    CString csClanid = L"0";
    m_edit_clanid.GetWindowTextW(csClanid);

    int kickout = m_chk_kickout.GetCheck();
    int banchat = m_banchat.GetCheck();
    int antiadvance = m_chk_anti_advance.GetCheck();

    AuthorityHelper authorityHelper;
    Authority authority;
    base::StringToUint(base::WideToUTF8(csUserid.GetBuffer()), &authority.userid);
    base::StringToUint(base::WideToUTF8(csRoomid.GetBuffer()), &authority.roomid);
    base::StringToUint(base::WideToUTF8(csClanid.GetBuffer()), &authority.clanid);
    authority.kickout = kickout;
    authority.banchat = banchat;
    authority.antiadvance = antiadvance;

    if (!authorityHelper.Save(authority))
        return;
}


void CAuthorityDlg::OnBnClickedBtnView()
{
    AuthorityHelper authorityHelper;
    Authority authority;
    if (!authorityHelper.Load(&authority))
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
}
