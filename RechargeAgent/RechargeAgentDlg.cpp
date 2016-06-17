
// RechargeAgentDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "RechargeAgent.h"
#include "RechargeAgentDlg.h"
#include "RechargeAgentHelper.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRechargeAgentDlg 对话框


CRechargeAgentDlg::CRechargeAgentDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CRechargeAgentDlg::IDD, pParent)
    , rechargeAgentHelper_(new RechargeAgentHelper)
{
    rechargeAgentHelper_->Initialize();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRechargeAgentDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_TAB_MAIN_PAGE, m_tabctrl);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_edit_account);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_password);
    DDX_Control(pDX, IDC_CHK_REMEMBER_USER, m_chk_remember);
    DDX_Control(pDX, IDC_STATIC_VERIFYCODE, m_static_verifycode);
    DDX_Control(pDX, IDC_EDIT_VERIFYCODE, m_register_verifycode);
}

BEGIN_MESSAGE_MAP(CRechargeAgentDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN_PAGE, &CRechargeAgentDlg::OnTcnSelchangeTabMainPage)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CRechargeAgentDlg::OnBnClickedBtnLogin)
    ON_BN_CLICKED(IDC_BTN_REFLASH_VERIFYCODE, &CRechargeAgentDlg::OnBnClickedBtnReflashVerifycode)
END_MESSAGE_MAP()


// CRechargeAgentDlg 消息处理程序

BOOL CRechargeAgentDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

    //为Tab Control增加两个页面
    m_tabctrl.InsertItem(0, _T("充值链接销售记录"));
    m_tabctrl.InsertItem(1, _T("星币转帐记录"));

    //创建两个对话框
    m_dlgsalerecord.Create(IDD_DLG_SALE_RECORD, &m_tabctrl);
    m_dlgtransferrecord.Create(IDD_DLG_TRANSFER_RECORD, &m_tabctrl);
    pDialog.push_back(&m_dlgsalerecord);
    pDialog.push_back(&m_dlgtransferrecord);

    //设定在Tab内显示的范围
    CRect rc;
    m_tabctrl.GetClientRect(rc);
    rc.top += 20;
    rc.bottom -= 0;
    rc.left += 0;
    rc.right -= 0;
    m_dlgsalerecord.MoveWindow(&rc);
    m_dlgtransferrecord.MoveWindow(&rc);

    m_dlgsalerecord.ShowWindow(SW_SHOW);
    m_dlgtransferrecord.ShowWindow(SW_HIDE);

    //保存当前选择
    m_curtabid = 0;

    return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRechargeAgentDlg::OnPaint()
{
    if (!image.IsNull())
    {
        int hight = image.GetHeight();
        int width = image.GetWidth();
        CRect rc;
        m_static_verifycode.GetWindowRect(&rc);
        ScreenToClient(rc);
        image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 18, rc.left + width + 20,
            rc.top + hight + 18));
        m_register_verifycode.SetFocus();
    }

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
HCURSOR CRechargeAgentDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CRechargeAgentDlg::OnTcnSelchangeTabMainPage(NMHDR *pNMHDR, LRESULT *pResult)
{
    //把当前的页面隐藏起来
    pDialog[m_curtabid]->ShowWindow(SW_HIDE);
    //得到新的页面索引
    m_curtabid = m_tabctrl.GetCurSel();
    //把新的页面显示出来
    pDialog[m_curtabid]->ShowWindow(SW_SHOW);
    *pResult = 0;
}


void CRechargeAgentDlg::OnBnClickedBtnLogin()
{
    CString username;
    CString password;
    CString verifycode;
    m_edit_account.GetWindowTextW(username);
    m_edit_password.GetWindowTextW(password);
    m_register_verifycode.GetWindowTextW(verifycode);

    bool remember = !!m_chk_remember.GetCheck();

    std::wstring errorcode;
    bool result = LoginByRequest(username.GetBuffer(), password.GetBuffer(),
                                 verifycode.GetBuffer(), &errorcode);
    std::wstring message = std::wstring(L"login ") + (result ? L"success" : L"failed");
}


void CRechargeAgentDlg::OnBnClickedBtnReflashVerifycode()
{
    std::wstring errormsg;
    std::vector<uint8> picture;
    if (!rechargeAgentHelper_->GetVerifyCode(&picture))
    {
        errormsg = L"获取验证码失败";
        return;
    }
    
    std::wstring pathname;
    if (!rechargeAgentHelper_->SaveVerifyCodeImage(picture, &pathname))
    {
        assert(false);
        return;
    }

    if (!image.IsNull())
    {
        image.Destroy();
    }

    image.Load(pathname.c_str());
    int hight = image.GetHeight();
    int width = image.GetWidth();
    CRect rc;
    m_static_verifycode.GetWindowRect(&rc);
    ScreenToClient(rc);
    image.Draw(GetDC()->m_hDC, CRect(rc.left, rc.top, rc.left + width + 20,
        rc.top + hight + 30));
    m_register_verifycode.SetWindowTextW(L"");
}

bool CRechargeAgentDlg::LoginByRequest(
    const std::wstring& account, const std::wstring& password,
    const std::wstring& verifycode, std::wstring* errorcode)
{
    // 测试通过的curl登录方式
    bool result = rechargeAgentHelper_->Login(
        base::WideToUTF8(account), base::WideToUTF8(password), 
        base::WideToUTF8(verifycode), errorcode);

    return result;
}

bool CRechargeAgentDlg::GetVerifyCode(std::vector<uint8>* picture)
{

    return false;
}

