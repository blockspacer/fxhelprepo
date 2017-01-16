
// LibEventClientDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <functional>
#include "LibEventClient.h"
#include "LibEventClientDlg.h"
#include "afxdialogex.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLibEventClientDlg 对话框

CLibEventClientDlg::CLibEventClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLibEventClientDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CLibEventClientDlg::~CLibEventClientDlg()
{
    client_controller_.Finalize();
}

void CLibEventClientDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_TARGET_IP, m_edit_ip);
    DDX_Control(pDX, IDC_EDIT_TARGET_PORT, m_edit_port);
    DDX_Control(pDX, IDC_EDIT_CLIENT_COUNT, m_edit_target_count);
    DDX_Control(pDX, IDC_LIST_MESSAGE, m_list_message);
    DDX_Control(pDX, IDC_LIST_STATUS, m_listctrl_ip_target_status);
}

BEGIN_MESSAGE_MAP(CLibEventClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_BEGIN, &CLibEventClientDlg::OnBnClickedBtnBegin)
END_MESSAGE_MAP()


// CLibEventClientDlg 消息处理程序

BOOL CLibEventClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    client_controller_.Initialize();
    m_edit_ip.SetWindowTextW(L"58.63.236.248");
    m_edit_port.SetWindowTextW(L"80");
    m_edit_target_count.SetWindowTextW(L"3");

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLibEventClientDlg::OnPaint()
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
HCURSOR CLibEventClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CLibEventClientDlg::OnBnClickedBtnBegin()
{
    CString cs_ip;
    CString cs_port;
    CString cs_target_count;

    m_edit_ip.GetWindowText(cs_ip);
    m_edit_port.GetWindowText(cs_port);
    m_edit_target_count.GetWindowText(cs_target_count);

    std::string utf8_target_count = base::WideToUTF8(cs_target_count.GetBuffer());
    int target_count = 1;
    base::StringToInt(utf8_target_count, &target_count);

    std::string ip = base::WideToUTF8(cs_ip.GetBuffer());

    int port = 0;
    std::string utf8_port = base::WideToUTF8(cs_port.GetBuffer());
    base::StringToInt(utf8_port, &port);

    for (int index = 0; index < target_count; index++)
    {
        client_controller_.AddClient(ip, port,
                                     std::bind(&CLibEventClientDlg::ConnectNotify,
                                     this, std::placeholders::_1),
                                     std::bind(&CLibEventClientDlg::DataReceiveNotify, 
                                     this, std::placeholders::_1, std::placeholders::_2)
                                     );
    }
}

void CLibEventClientDlg::ConnectNotify(bool result)
{

}

void CLibEventClientDlg::DataReceiveNotify(bool result, std::vector<uint8>& data)
{

}


