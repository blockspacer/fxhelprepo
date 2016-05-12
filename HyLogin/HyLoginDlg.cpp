
// HyLoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "HyLogin.h"
#include "HyLoginDlg.h"
#include "afxdialogex.h"
#include "HuyaLogic.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHyLoginDlg 对话框



CHyLoginDlg::CHyLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CHyLoginDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHyLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_edit_account);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_password);
    DDX_Control(pDX, IDC_LIST_INFO, m_list_info);
}

BEGIN_MESSAGE_MAP(CHyLoginDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CHyLoginDlg::OnBnClickedBtnLogin)
END_MESSAGE_MAP()


// CHyLoginDlg 消息处理程序

BOOL CHyLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHyLoginDlg::OnPaint()
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
HCURSOR CHyLoginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CHyLoginDlg::OnBnClickedBtnLogin()
{
    CString csAccount;
    m_edit_account.GetWindowTextW(csAccount);
    CString csPassword;
    m_edit_password.GetWindowTextW(csPassword);

    HuyaLogic huyaLogic;
    std::wstring errormsg = L"Login success";
    if (huyaLogic.HuyaLogin(csAccount.GetBuffer(), csPassword.GetBuffer(), &errormsg))
    {
        m_list_info.AddString(L"Login success");
    }
    else
    {
        m_list_info.AddString(L"Login failed");
        m_list_info.AddString(errormsg.c_str());
    }
}
