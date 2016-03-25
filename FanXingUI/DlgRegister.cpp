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
END_MESSAGE_MAP()


// DlgRegister 消息处理程序

//void CDlgRegister::OnPaint()
//{
//    CDialogEx::OnPaint();
//}

void CDlgRegister::OnBnClickedBtnCheckExist()
{
    CString username;
    m_register_username.GetDlgItemTextW(IDC_EDIT_REGISTER_NAME, username);
    registerNetworkHelper_->RegisterCheckUserExist(username.GetBuffer());
}


void CDlgRegister::OnBnClickedBtnRegister()
{
    // TODO:  在此添加控件通知处理程序代码
}


void CDlgRegister::OnBnClickedBtnVerifyCode()
{
    std::vector<uint8> picture;
    registerNetworkHelper_->RegisterGetVerifyCode(&picture);
    registerHelper_->SaveVerifyCodeImage(picture);
    std::wstring pathname;
    registerHelper_->GetVerifyCodeImagePath(&pathname);
    CImage image;
    image.Load(pathname.c_str());
    int hight = image.GetHeight();
    int width = image.GetWidth();
    CRect rc;
    m_static_verifycode.GetWindowRect(&rc);
    ScreenToClient(rc);
    image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 30, rc.left + width +20, 
        rc.top + hight + 30));
}
