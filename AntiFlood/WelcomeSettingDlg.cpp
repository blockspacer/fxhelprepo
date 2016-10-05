// WelcomeSettingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AntiFlood.h"
#include "WelcomeSettingDlg.h"
#include "afxdialogex.h"


// WelcomeSettingDlg 对话框

IMPLEMENT_DYNAMIC(WelcomeSettingDlg, CDialogEx)

WelcomeSettingDlg::WelcomeSettingDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(WelcomeSettingDlg::IDD, pParent)
{

}

WelcomeSettingDlg::~WelcomeSettingDlg()
{
}

void WelcomeSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_WELCOME_FANXINGID, m_edit_welcome_fanxingid);
    DDX_Control(pDX, IDC_EDIT_WELCOME_SETTING_NAME, m_edit_welcome_setting_name);
    DDX_Control(pDX, IDC_EDIT_WELCOME_CONTENT, m_edit_welcome_content);
}


BEGIN_MESSAGE_MAP(WelcomeSettingDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &WelcomeSettingDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &WelcomeSettingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// WelcomeSettingDlg 消息处理程序


void WelcomeSettingDlg::OnBnClickedOk()
{
    CString fanxingid;
    m_edit_welcome_fanxingid.GetWindowTextW(fanxingid);
    fanxingid_ = fanxingid.GetBuffer();
    CString name;
    m_edit_welcome_setting_name.GetWindowTextW(name);
    name_ = name.GetBuffer();

    CString content;
    m_edit_welcome_content.GetWindowTextW(content);
    content_ = content.GetBuffer();

    CDialogEx::OnOK();
}


void WelcomeSettingDlg::OnBnClickedCancel()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}

bool WelcomeSettingDlg::GetSettingInfo(std::wstring* fanxingid, std::wstring* name,
    std::wstring* content) const
{
    if (!fanxingid || !name || !content)
        return false;

    fanxingid->assign(fanxingid_);
    name->assign(name_);
    content->assign(content_);
    return true;
}
