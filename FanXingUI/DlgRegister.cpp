// DlgRegister.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgRegister.h"
#include "atlimage.h"
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "RegisterHelper.h"
#include "../Network/EncodeHelper.h"
#include "third_party/chromium/base/basictypes.h"

// DlgRegister 对话框

IMPLEMENT_DYNAMIC(CDlgRegister, CDialogEx)

CDlgRegister::CDlgRegister(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRegister::IDD, pParent)
{
    registerNetworkHelper_.reset(new NetworkHelper);
    registerNetworkHelper_->Initialize();
    registerHelper_.reset(new RegisterHelper);
}

CDlgRegister::~CDlgRegister()
{
    registerNetworkHelper_->Finalize();
}

void CDlgRegister::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_REGISTER_NAME, m_register_username);
    DDX_Control(pDX, IDC_EDIT_REGISTER_PASSWORD, m_register_password);
    DDX_Control(pDX, IDC_EDIT_VERIFY_CODE, m_register_verifycode);
    DDX_Control(pDX, IDC_STATIC_VERIFYCODE, m_static_verifycode);
}


BEGIN_MESSAGE_MAP(CDlgRegister, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CHECK_EXIST, &CDlgRegister::OnBnClickedBtnCheckExist)
    ON_BN_CLICKED(IDC_BTN_REGISTER, &CDlgRegister::OnBnClickedBtnRegister)
    ON_BN_CLICKED(IDC_BTN_VERIFY_CODE, &CDlgRegister::OnBnClickedBtnVerifyCode)
    ON_WM_PAINT()
END_MESSAGE_MAP()


// DlgRegister 消息处理程序

void CDlgRegister::OnPaint()
{
    if (!image.IsNull())
    {
        int hight = image.GetHeight();
        int width = image.GetWidth();
        CRect rc;
        m_static_verifycode.GetWindowRect(&rc);
        ScreenToClient(rc);
        image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 30, rc.left + width + 20,
            rc.top + hight + 30));
    }
    CDialogEx::OnPaint();
}

void CDlgRegister::OnBnClickedBtnCheckExist()
{
    CString username;
    m_register_username.GetWindowTextW(username);
    registerNetworkHelper_->RegisterCheckUserExist(username.GetBuffer());
}


void CDlgRegister::OnBnClickedBtnRegister()
{
    CString username;
    CString password;
    CString verifycode;

    m_register_username.GetWindowTextW(username);
    m_register_password.GetWindowTextW(password);
    m_register_verifycode.GetWindowTextW(verifycode);

    if (username.IsEmpty() || password.IsEmpty() || verifycode.IsEmpty())
    {
        AfxMessageBox(L"请输入完整注册信息");
        return;
    }

    if (!registerNetworkHelper_->RegisterCheckUserInfo(username.GetString(),
        password.GetString()))
    {
        AfxMessageBox(L"检测用户信息失败");
        return;
    }

    if (!registerNetworkHelper_->RegisterCheckVerifyCode(verifycode.GetString()))
    {
        AfxMessageBox(L"验证码检测失败");
        return;
    }

    if (!registerNetworkHelper_->RegisterUser(username.GetString(),
        password.GetString(), verifycode.GetString()))
    {
        AfxMessageBox(L"注册失败");
        return;
    }
}


void CDlgRegister::OnBnClickedBtnVerifyCode()
{
    std::vector<uint8> picture;
    registerNetworkHelper_->RegisterGetVerifyCode(&picture);
    registerHelper_->SaveVerifyCodeImage(picture);
    std::wstring pathname;
    registerHelper_->GetVerifyCodeImagePath(&pathname);
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
    image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 30, rc.left + width +20, 
        rc.top + hight + 30));
}
