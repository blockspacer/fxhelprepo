// DlgLogin.cpp : 实现文件
//

#include "stdafx.h"
#include "UserTracker.h"
#include "LoginDlg.h"
#include "afxdialogex.h"
#include "UserTrackerHelper.h"
#include "AuthorityHelper.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

// CDlgLogin 对话框
namespace
{
    void MessageCallback(const std::wstring& msg){};// 忽略消息回调
}
IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent,
    UserTrackerHelper* tracker_helper)
	: CDialogEx(CLoginDlg::IDD, pParent)
    , tracker_helper_(tracker_helper)
{
    tracker_helper_->SetNotifyMessageCallback(base::Bind(&MessageCallback));
}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_edit_account);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_password);
    DDX_Control(pDX, IDC_EDIT_VERIFYCODE, m_edit_verifycode);
    DDX_Control(pDX, IDC_STATIC_VERIFYCODE, m_static_verifycode);
}

BOOL CLoginDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // 在窗口标题增加授权信息
    std::wstring title_post = tracker_helper_->GetAuthorityMessage();
    CString title;
    GetWindowTextW(title);
    title += title_post.c_str();
    SetWindowTextW(title);

    return TRUE;
}
BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_WM_PAINT()
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CLoginDlg::OnBnClickedBtnLogin)
    ON_BN_CLICKED(IDCANCEL, &CLoginDlg::OnBnClickedCancel)
    ON_MESSAGE(WM_USER_LOGIN_RESULT, &CLoginDlg::OnLoginResult)
END_MESSAGE_MAP()


// CDlgLogin 消息处理程序
void CLoginDlg::OnBnClickedBtnLogin()
{
    CString cs_account;
    CString cs_password;
    CString cs_verifycode;
    m_edit_account.GetWindowText(cs_account);
    m_edit_password.GetWindowText(cs_password);
    m_edit_verifycode.GetWindowText(cs_verifycode);

    std::string account = base::WideToUTF8(cs_account.GetBuffer());
    std::string password = base::WideToUTF8(cs_password.GetBuffer());
    std::string verifycode = base::WideToUTF8(cs_verifycode.GetBuffer());
    std::string errormsg;
    auto callback = base::Bind(&CLoginDlg::LoginResult,
        base::Unretained(this));
    if (!tracker_helper_->LoginUser(account, password, verifycode, callback))
    {
        AfxMessageBox(L"登录失败");
        return;
    }
}

void CLoginDlg::OnBnClickedCancel()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}

void CLoginDlg::OnPaint()
{
    if (!image.IsNull())
    {
        int hight = image.GetHeight();
        int width = image.GetWidth();
        CRect rc;
        m_static_verifycode.GetWindowRect(&rc);
        ScreenToClient(rc);
        image.Draw(GetDC()->m_hDC, CRect(rc.left, rc.top, rc.left + width,
            rc.top + hight));
        m_edit_verifycode.SetFocus();
    }

    CDialogEx::OnPaint();
}

LRESULT CLoginDlg::OnLoginResult(WPARAM wParam, LPARAM lParam)
{
    bool result = !!wParam;
    std::string errmsg = std::string(*(std::string*)(lParam));
    delete (std::string*)(lParam);

    if (result)
    {
        CDialogEx::OnOK();
        return 0;
    }

    if (base::UTF8ToWide(errmsg).find(L"验证码") != std::string::npos)
    {
        m_edit_verifycode.EnableWindow(TRUE);
        RefreshVerifyCode();
    }
    else
    {
        AfxMessageBox(base::UTF8ToWide(errmsg).c_str());
    }
    return 0;
}

void CLoginDlg::LoginResult(
    bool result, uint32 servertime, const std::string& errormsg)
{
    std::string* msg = new std::string(errormsg);
    this->PostMessage(WM_USER_LOGIN_RESULT, (int)(result), (LPARAM)(msg));
}

bool CLoginDlg::RefreshVerifyCode()
{
    std::vector<uint8> picture;
    if (!tracker_helper_->LoginGetVerifyCode(&picture))
    {
        AfxMessageBox(L"获取验证码失败");
        return false;
    }

    // 显示验证码
    if (!image.IsNull())
    {
        image.Destroy();
    }
    COleStreamFile osf;
    osf.CreateMemoryStream(NULL);
    osf.Write(picture.data(), picture.size());
    osf.SeekToBegin();
    image.Load(osf.GetStream());
    int hight = image.GetHeight();
    int width = image.GetWidth();
    CRect rc;
    m_static_verifycode.GetWindowRect(&rc);
    ScreenToClient(rc);
    image.Draw(GetDC()->m_hDC, CRect(rc.left, rc.top, rc.left + width,
        rc.top + hight));
    m_edit_verifycode.SetWindowTextW(L"");

    return true;
}
