
// FanXingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "FanXing.h"
#include "FanXingDlg.h"
#include "afxdialogex.h"

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
    DDX_Control(pDX, IDC_EXPLORER1, web_);
    DDX_Control(pDX, IDC_LIST1, InfoList_);
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

    web_.put_Silent(TRUE);// 禁止弹出360升级浏览器提示

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

    web_.Navigate(L"http://fanxing.kugou.com", NULL, NULL, NULL, NULL);

    SetDlgItemText(IDC_EDIT_NAV, L"1014619");
    SetDlgItemInt(IDC_EDIT_X, 0);
    SetDlgItemInt(IDC_EDIT_Y, 0);
    SetDlgItemText(IDC_EDIT_GIFT, L"普通,红心");

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
    CComQIPtr<IDispatch> iDisp(web_.get_Document());
    if (iDisp)
    {
        CComQIPtr<IHTMLDocument2> iDocu;
        HRESULT hr = iDisp->QueryInterface(IID_IHTMLDocument2, (void**)&iDocu);
        if (!FAILED(hr))
        {
            WebHandler handler(iDocu);
            //handler.Execute();
            handler.Login(username, password);
        }
    }
}

//跳转页面功能
void CFanXingDlg::OnBnClickedButtonNav()
{
    CString strRoomid;
    GetDlgItemText(IDC_EDIT_NAV, strRoomid);
    CString strUrl = L"http://fanxing.kugou.com/" + strRoomid;
    VARIANT vtNull = {};
    web_.Navigate(strUrl, &vtNull, &vtNull, &vtNull, &vtNull);
    LOG(INFO) << L"Navigate To " << strUrl;
    // 获取房间信息，启动功能

    if (network_)
    {
        network_->Finalize();
    }    
    network_.reset(new NetworkHelper);
    network_->Initialize();
    network_->SetNotify(
        std::bind(&CFanXingDlg::Notify, this, std::placeholders::_1));

    network_->EnterRoom(strRoomid.GetBuffer());
}

//指定位置点击功能
void CFanXingDlg::OnBnClickedButtonClick()
{
    CComQIPtr<IDispatch> iDisp(web_.get_Document());
    if (iDisp)
    {
        CComQIPtr<IHTMLDocument2> iDocu;
        HRESULT hr = iDisp->QueryInterface(IID_IHTMLDocument2, (void**)&iDocu);
        if (!FAILED(hr))
        {
            int x = GetDlgItemInt(IDC_EDIT_X);
            int y = GetDlgItemInt(IDC_EDIT_Y);           

            HWND explorerHWND = nullptr;

            HWND hwnd = ::FindWindowEx(web_.m_hWnd, 0, L"Shell DocObject View", NULL);
            if (hwnd)
                explorerHWND = ::FindWindowEx(hwnd, 0, L"Internet Explorer_Server", NULL);
            
            if (explorerHWND)
            {
                WebHandler handler(iDocu);
                handler.ClickXY(explorerHWND, x, y);
            }
        }
    }
}

// 送星星功能
void CFanXingDlg::OnBnClickedButtonRewarstar()
{
    CComQIPtr<IDispatch> iDisp(web_.get_Document());
    if (iDisp)
    {
        CComQIPtr<IHTMLDocument2> iDocu;
        HRESULT hr = iDisp->QueryInterface(IID_IHTMLDocument2, (void**)&iDocu);
        if (!FAILED(hr))
        {
            WebHandler handler(iDocu);
            handler.RewardStar();
        }
    }
}

// 送礼物功能
void CFanXingDlg::OnBnClickedButtonRewardgift()
{
    CComQIPtr<IDispatch> iDisp(web_.get_Document());
    if (iDisp)
    {
        CComQIPtr<IHTMLDocument2> iDocu;
        HRESULT hr = iDisp->QueryInterface(IID_IHTMLDocument2, (void**)&iDocu);
        if (!FAILED(hr))
        {
            CString strGift;
            GetDlgItemText(IDC_EDIT_GIFT, strGift);

            WebHandler handler(iDocu);
            handler.RewardGift((LPCTSTR)strGift);
        }
    }
}



void CFanXingDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    // TODO:  在此添加消息处理程序代码和/或调用默认值

    CDialogEx::OnLButtonDown(nFlags, point);
}

// 获取公屏信息
void CFanXingDlg::OnBnClickedBtnGetmsg()
{
    // TODO:  在此添加控件通知处理程序代码
}

// 用来做测试的函数
void CFanXingDlg::OnBnClickedBtnTest()
{
    // TODO:  在此添加控件通知处理程序代码
    CComQIPtr<IDispatch> iDisp(web_.get_Document());
    if (iDisp)
    {
        CComQIPtr<IHTMLDocument2> iDocu;
        HRESULT hr = iDisp->QueryInterface(IID_IHTMLDocument2, (void**)&iDocu);
        if (!FAILED(hr))
        {
            //处理数据
            WebHandler handler(iDocu);
            handler.GetChatMessage();
        }
    }
}

void CFanXingDlg::Notify(const std::wstring& message)
{
    // 发送数据给窗口
    messageMutex_.lock();
    messageQueen_.push_back(message);
    messageMutex_.unlock();
    this->PostMessage(WM_USER_01, 0, 0);
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
