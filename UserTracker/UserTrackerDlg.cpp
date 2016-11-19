
// UserTrackerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UserTracker.h"
#include "UserTrackerDlg.h"
#include "afxdialogex.h"
#include "UserTrackerHelper.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUserTrackerDlg 对话框
namespace
{
    struct ProgressStruct 
    {
        uint32 current;
        uint32 all;
    };
}


CUserTrackerDlg::CUserTrackerDlg(CWnd* pParent,
    UserTrackerHelper* tracker_helper)
	: CDialogEx(CUserTrackerDlg::IDD, pParent)
    , tracker_helper_(tracker_helper)
    , list_info_count(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CUserTrackerDlg::~CUserTrackerDlg()
{
}

void CUserTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_USER_ID, m_edit_target_fanxing_id);
    DDX_Control(pDX, IDC_LIST_MESSAGE, m_list_message);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_edit_account);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_password);
    DDX_Control(pDX, IDC_STATIC_ROOM_PROGRESS, m_static_room_progress);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
}

BEGIN_MESSAGE_MAP(CUserTrackerDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_GETALLROOMDATA, &CUserTrackerDlg::OnBnClickedBtnGetAllRoomData)
    ON_BN_CLICKED(IDC_BTN_FIND_IN_CACHE, &CUserTrackerDlg::OnBnClickedBtnFindInCache)
    ON_BN_CLICKED(IDC_BTN_UPDATA_FIND, &CUserTrackerDlg::OnBnClickedBtnUpdataFind)
    ON_MESSAGE(WM_USER_MSG, &CUserTrackerDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_PROGRESS, &CUserTrackerDlg::OnRoomProgress)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CUserTrackerDlg::OnBnClickedBtnLogin)
    ON_BN_CLICKED(IDC_BTN_CANCEL, &CUserTrackerDlg::OnBnClickedBtnCancel)
END_MESSAGE_MAP()


// CUserTrackerDlg 消息处理程序

BOOL CUserTrackerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    //tracker_helper_->Initialize();
    tracker_helper_->SetNotifyMessageCallback(
        base::Bind(&CUserTrackerDlg::Notify, base::Unretained(this)));

    m_progress1.SetRange(0, 100);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUserTrackerDlg::OnClose()
{
    tracker_helper_->CancelCurrentOperation();
    //tracker_helper_->Finalize();
    CDialogEx::OnClose();
}

void CUserTrackerDlg::OnOK()
{
    // 避免按回车直接退出
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUserTrackerDlg::OnPaint()
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
HCURSOR CUserTrackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUserTrackerDlg::SetHScroll()
{
    CDC* dc = GetDC();

    CString str;
    int index = m_list_message.GetCount() - 1;
    if (index >= 0)
    {
        m_list_message.GetText(index, str);
        SIZE s = dc->GetTextExtent(str);
        long temp = (long)SendDlgItemMessage(IDC_LIST_MESSAGE, LB_GETHORIZONTALEXTENT, 0, 0); //temp得到滚动条的宽度
        if (s.cx > temp)
        {
            SendDlgItemMessage(IDC_LIST_MESSAGE, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
        }
    }

    ReleaseDC(dc);
}

void CUserTrackerDlg::OnBnClickedBtnGetAllRoomData()
{
    if (!tracker_helper_->UpdataAllStarRoomUserMap(
        base::Bind(&CUserTrackerDlg::RoomProgress,
        base::Unretained(this))))
    {
        Notify(L"操作失败, 请先登录");
    } 
}

void CUserTrackerDlg::OnBnClickedBtnFindInCache()
{
    CString cs_user_id;
    m_edit_target_fanxing_id.GetWindowTextW(cs_user_id);
    uint32 user_id = 0;
    base::StringToUint(base::WideToUTF8(cs_user_id.GetBuffer()), &user_id);
    std::vector<uint32> users;
    users.push_back(user_id);
    if (!tracker_helper_->GetUserLocationByUserId(users,
        base::Bind(&CUserTrackerDlg::RoomProgress,
        base::Unretained(this))))
    {
        Notify(L"操作失败, 请先登录");
    }

}

void CUserTrackerDlg::OnBnClickedBtnUpdataFind()
{
    CString cs_user_id;
    m_edit_target_fanxing_id.GetWindowTextW(cs_user_id);
    uint32 user_id = 0;
    base::StringToUint(base::WideToUTF8(cs_user_id.GetBuffer()), &user_id);
    std::vector<uint32> users;
    users.push_back(user_id);
    if (!tracker_helper_->UpdateForFindUser(users,
        base::Bind(&CUserTrackerDlg::RoomProgress,
        base::Unretained(this))))
    {
        Notify(L"操作失败, 请先登录");
    }

}

void CUserTrackerDlg::Notify(const std::wstring& message)
{
    // 发送数据给窗口
    messageMutex_.lock();
    messageQueen_.push_back(message);
    messageMutex_.unlock();
    this->PostMessage(WM_USER_MSG, 0, 0);
}

void CUserTrackerDlg::RoomProgress(uint32 current, uint32 all)
{
    ProgressStruct* progress = new ProgressStruct;
    progress->all = all;
    progress->current = current;
    this->PostMessage(WM_USER_PROGRESS, (WPARAM)(PROGRESSTYPE_ROOM), (LPARAM)progress);
}

LRESULT CUserTrackerDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::vector<std::wstring> messages;
    messageMutex_.lock();
    messages.swap(messageQueen_);
    messageMutex_.unlock();

    for (auto str : messages)
    {
        m_list_message.InsertString(list_info_count++, str.c_str());
    }

    m_list_message.SetCurSel(list_info_count - 1);
    SetHScroll();
    return 0;
}

LRESULT CUserTrackerDlg::OnRoomProgress(WPARAM wParam, LPARAM lParam)
{
    ProgressType pt = (ProgressType)(wParam);
    ProgressStruct* ps = (ProgressStruct*)(lParam);

    int pos = static_cast<int>(ps->current*100.0 / ps->all);
    std::wstring show_msg = base::UintToString16(ps->current) +
        L" / " + base::UintToString16(ps->all);
    switch (pt)
    {
    case CUserTrackerDlg::PROGRESSTYPE_ROOM:   
        m_progress1.SetPos(pos);
        m_static_room_progress.SetWindowTextW(show_msg.c_str());
        break;
    case CUserTrackerDlg::PROGRESSTYPE_USER:
        break;
    default:
        break;
    }

    delete ps;
    return 0;
}

void CUserTrackerDlg::OnBnClickedBtnLogin()
{
    //CString cs_account;
    //CString cs_password;
    //m_edit_account.GetWindowText(cs_account);
    //m_edit_password.GetWindowText(cs_password);

    //std::string account = base::WideToUTF8(cs_account.GetBuffer());
    //std::string password = base::WideToUTF8(cs_password.GetBuffer());
    //tracker_helper_->LoginUser(account, password);
}


void CUserTrackerDlg::OnBnClickedBtnCancel()
{
    tracker_helper_->CancelCurrentOperation();
}
