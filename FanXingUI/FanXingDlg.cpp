
// FanXingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FanXing.h"
#include "FanXingDlg.h"
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "WebHandler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

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


// CFanXingDlg 对话框



CFanXingDlg::CFanXingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFanXingDlg::IDD, pParent)
    , network_(nullptr)
    , count(0)
    , rowcount_(0)
{
    //network_->Initialize();
    //network_->SetNotify(
    //    std::bind(&CFanXingDlg::Notify, this, std::placeholders::_1));

	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); 
}

CFanXingDlg::~CFanXingDlg()
{
    if (network_)
    {
        network_->Finalize();
    }  
}

void CFanXingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, InfoList_);
    DDX_Control(pDX, IDC_LIST_USER_STATUS, m_ListCtrl_UserStatus);
}

BEGIN_MESSAGE_MAP(CFanXingDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON1, &CFanXingDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON_CLICK, &CFanXingDlg::OnBnClickedButtonClick)
    ON_BN_CLICKED(IDC_BUTTON_REWARSTAR, &CFanXingDlg::OnBnClickedButtonRewarstar)
    ON_BN_CLICKED(IDC_BUTTON_REWARDGIFT, &CFanXingDlg::OnBnClickedButtonRewardgift)
    ON_BN_CLICKED(IDC_BUTTON_NAV, &CFanXingDlg::OnBnClickedButtonNav)
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BTN_GETMSG, &CFanXingDlg::OnBnClickedBtnGetmsg)
    ON_BN_CLICKED(IDC_BTN_TEST, &CFanXingDlg::OnBnClickedBtnTest)
    ON_MESSAGE(WM_USER_01, &CFanXingDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_ADD_ENTER_ROOM_INFO, &CFanXingDlg::OnDisplayDataToGrid)
    ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_USER_STATUS, &CFanXingDlg::OnLvnGetdispinfoListUserStatus)
END_MESSAGE_MAP()


// CFanXingDlg 消息处理程序

BOOL CFanXingDlg::OnInitDialog()
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

    DWORD dwStyle = m_ListCtrl_UserStatus.GetExtendedStyle();
    dwStyle |= LVS_REPORT;
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
    dwStyle |= LVS_OWNERDATA;
    dwStyle |= LVS_AUTOARRANGE;
    m_ListCtrl_UserStatus.SetExtendedStyle(dwStyle); //设置扩展风格

    SetDlgItemText(IDC_EDIT_NAV, L"1014619");
    SetDlgItemInt(IDC_EDIT_X, 0);
    SetDlgItemInt(IDC_EDIT_Y, 0);

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFanXingDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFanXingDlg::OnPaint()
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

void CFanXingDlg::OnClose()
{
    if (network_)
    {
        network_->RemoveNotify();
    }  
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFanXingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 登录功能
void CFanXingDlg::OnBnClickedButton1()
{
    CString username;
    CString password;
    GetDlgItemText(IDC_EDIT_Username, username);
    GetDlgItemText(IDC_EDIT_Password, password);

    // 测试通过的curl登录方式
    bool result = LoginByRequest(username.GetBuffer(), password.GetBuffer());
    std::wstring message = std::wstring(L"login ") + (result ? L"success" : L"failed");
    Notify(message);
}

//跳转页面功能
void CFanXingDlg::OnBnClickedButtonNav()
{
    std::vector<std::wstring> columnlist = {
        L"昵称",
        L"财富等级",
        L"用户id",
        L"进房时间",
        L"房间号"
    };

    int nColumnCount = m_ListCtrl_UserStatus.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_UserStatus.DeleteColumn(i);

    uint32 i = 0;
    for (const auto& it : columnlist)
    {
        m_ListCtrl_UserStatus.InsertColumn(i++, it.c_str(), LVCFMT_LEFT, 100);//插入列
    }

    CString strRoomid;
    GetDlgItemText(IDC_EDIT_NAV, strRoomid);
    network_->SetNotify(
        std::bind(&CFanXingDlg::Notify, this, std::placeholders::_1));

    network_->SetNotify201(
        std::bind(&CFanXingDlg::Notify201, this, std::placeholders::_1));

    network_->EnterRoom(strRoomid.GetBuffer());
}

//指定位置点击功能
void CFanXingDlg::OnBnClickedButtonClick()
{

}

// 送星星功能
void CFanXingDlg::OnBnClickedButtonRewarstar()
{
}

// 送礼物功能
void CFanXingDlg::OnBnClickedButtonRewardgift()
{
}

void CFanXingDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialogEx::OnLButtonDown(nFlags, point);
}

// 获取公屏信息
void CFanXingDlg::OnBnClickedBtnGetmsg()
{
}

// 用来做测试的函数
void CFanXingDlg::OnBnClickedBtnTest()
{
    EnterRoomUserInfo enterRoomUserInfo;
    enterRoomUserInfo.roomid = 1053637;
    enterRoomUserInfo.richlevel = 1;
    enterRoomUserInfo.nickname = "fanxingtest111";
    enterRoomUserInfo.userid = 120831944;
    network_->KickoutUsers(110468466, enterRoomUserInfo);
}

void CFanXingDlg::Notify(const std::wstring& message)
{
    // 发送数据给窗口
    messageMutex_.lock();
    messageQueen_.push_back(message);
    messageMutex_.unlock();
    this->PostMessage(WM_USER_01, 0, 0);
}

void CFanXingDlg::Notify201(const RowData& rowdata)
{
    // 发送数据给窗口
    rowdataMutex_.lock();
    rowdataQueue_.push_back(rowdata);
    rowdataMutex_.unlock();

    rowcount_++;
    m_ListCtrl_UserStatus.SetItemCountEx(rowcount_);
    m_ListCtrl_UserStatus.Invalidate();
}

bool CFanXingDlg::LoginByRequest(const std::wstring& username, const std::wstring& password)
{
    if (network_)
    {
        network_->Finalize();
    }
    network_.reset(new NetworkHelper);
    network_->Initialize();
    network_->SetNotify(
        std::bind(&CFanXingDlg::Notify, this, std::placeholders::_1));

    return network_->Login(username, password);
}

LRESULT CFanXingDlg::OnDisplayDataToGrid(WPARAM wParam, LPARAM lParam)
{
    if (rowdataQueue_.empty())
        return 0;
    
    std::vector<RowData> rowdatas;
    rowdataMutex_.lock();
    rowdatas.swap(rowdataQueue_);
    rowdataMutex_.unlock();

    // 获取之前的数据

    // 如果不存在,插入新的通知数据

    // 如果存在相同的userid项,则更新数据


    // 先统一全部插入数据

    return 0;
}

LRESULT CFanXingDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::vector<std::wstring> messages;
    messageMutex_.lock();
    messages.swap(messageQueen_);
    messageMutex_.unlock();

    for (auto str : messages)
    {
        InfoList_.InsertString(count++, str.c_str());
    }
    
    InfoList_.SetCurSel(count-1);
    return 0;
}


void CFanXingDlg::OnLvnGetdispinfoListUserStatus(NMHDR *pNMHDR, LRESULT *pResult)
{
    LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
    LV_ITEM *pItem = &(pDispInfo)->item;

    rowdataMutex_.lock();
    if (pItem->mask & LVIF_TEXT)
    {
        //使缓冲区数据与表格子项对应
        pItem->pszText = (LPWSTR)rowdataQueue_[pItem->iItem][pItem->iSubItem].c_str();
    }
    rowdataMutex_.unlock();

    *pResult = 0;
}
