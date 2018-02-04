
// SingerRankDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SingerRank.h"
#include "SingerRankDlg.h"
#include "afxdialogex.h"
#include "PhoneRank.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "Network/CurlWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSingerRankDlg 对话框



CSingerRankDlg::CSingerRankDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSingerRankDlg::IDD, pParent)
    , message_index(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSingerRankDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SINGER_ID, m_edit_roomid);
    DDX_Control(pDX, IDC_LIST_SINGERS, m_singer_list);
    DDX_Control(pDX, IDC_LIST_INFO, m_list_info);
}

BEGIN_MESSAGE_MAP(CSingerRankDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_SEARCH_RANK, &CSingerRankDlg::OnBnClickedBtnSearchRank)
    ON_BN_CLICKED(IDC_BTN_GET_ROOM_RANK, &CSingerRankDlg::OnBnClickedBtnGetRoomRank)
END_MESSAGE_MAP()


// CSingerRankDlg 消息处理程序

BOOL CSingerRankDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    CurlWrapper::CurlInit();


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSingerRankDlg::OnOK()
{

}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSingerRankDlg::OnPaint()
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
HCURSOR CSingerRankDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSingerRankDlg::OnBnClickedBtnSearchRank()
{
    // TODO:  在此添加控件通知处理程序代码
    phone_rank_.InitNewSingerRankInfos();

}


void CSingerRankDlg::OnBnClickedBtnGetRoomRank()
{
    // TODO:  在此添加控件通知处理程序代码
    CString cs_roomid;
    m_edit_roomid.GetWindowTextW(cs_roomid);
    uint32 roomid = 0;
    base::StringToUint(base::WideToUTF8(cs_roomid.GetBuffer()), &roomid);
    uint32 rank = 0;
    uint32 all = 0;
    if (!phone_rank_.GetSingerRankByRoomid(roomid, &rank, &all))
    {
        std::wstring w_message = L"无法获取排名";
        m_list_info.InsertString(message_index++, w_message.c_str());
        return;
    }


    std::string message;
    message += base::UintToString(roomid);
    message += " rank ( ";
    message += base::UintToString(rank);
    message += "/";
    message += base::UintToString(all);
    message += " )";

    m_list_info.InsertString(message_index++, base::UTF8ToWide(message).c_str());

}
