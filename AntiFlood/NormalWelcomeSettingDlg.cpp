// NormalWelcomeSetting.cpp : 实现文件
//

#include "stdafx.h"
#include <string>
#include "AntiFlood.h"
#include "NormalWelcomeSettingDlg.h"
#include "afxdialogex.h"

// CNormalWelcomeSetting 对话框

IMPLEMENT_DYNAMIC(NormalWelcomeSettingDlg, CDialogEx)

NormalWelcomeSettingDlg::NormalWelcomeSettingDlg(
    const std::wstring& welcome, CWnd* pParent /*=NULL*/)
	: CDialogEx(NormalWelcomeSettingDlg::IDD, pParent)
{
    normal_welcome_setting_ = welcome;
}

NormalWelcomeSettingDlg::~NormalWelcomeSettingDlg()
{
}

BOOL NormalWelcomeSettingDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();
    std::wstring tips = L"设置欢迎格式说明：[玩家]是特殊关键字，请在设置里保留。"
        L"\r\n比如:"
        L"\r\n你设置了：欢迎[玩家]进入直播间"
        L"\r\n等小明进入直播间的时候，会提示:欢迎小明进入直播间";
    m_edit_welcome_tips.SetWindowTextW(tips.c_str());
    m_edit_normal_welcome_setting.SetWindowTextW(normal_welcome_setting_.c_str());
    m_edit_welcome_tips.EnableWindow(FALSE);
    return TRUE;
}

std::wstring NormalWelcomeSettingDlg::GetNormalWelcome() const
{
    return normal_welcome_setting_;
}

void NormalWelcomeSettingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_WELCOME_TIPS, m_edit_welcome_tips);
    DDX_Control(pDX, IDC_EDIT_WELCOME_PRE, m_edit_normal_welcome_setting);
}


BEGIN_MESSAGE_MAP(NormalWelcomeSettingDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &NormalWelcomeSettingDlg::OnBnClickedOk)
    ON_BN_CLICKED(IDCANCEL, &NormalWelcomeSettingDlg::OnBnClickedCancel)
END_MESSAGE_MAP()


// CNormalWelcomeSetting 消息处理程序
void NormalWelcomeSettingDlg::OnBnClickedOk()
{
    // TODO:  在此添加控件通知处理程序代码
    CString welcome;
    m_edit_normal_welcome_setting.GetWindowTextW(welcome);
    normal_welcome_setting_ = welcome.GetBuffer();
    if (normal_welcome_setting_.find(L"[玩家]")==std::wstring::npos)
    {
        ::MessageBoxW(NULL, L"设置里必须包含\"[玩家]\"", L"设置错误", 0);
        return;
    }
    CDialogEx::OnOK();
}


void NormalWelcomeSettingDlg::OnBnClickedCancel()
{
    // TODO:  在此添加控件通知处理程序代码
    CDialogEx::OnCancel();
}
