// AdvertisementDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AntiFlood/resource.h"        // 主符号
#include "AdvertisementDlg.h"
#include "afxdialogex.h"


// AdvertisementDlg 对话框

IMPLEMENT_DYNAMIC(AdvertisementDlg, CDialogEx)

AdvertisementDlg::AdvertisementDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(AdvertisementDlg::IDD, pParent)
{

}

AdvertisementDlg::~AdvertisementDlg()
{
}

void AdvertisementDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EXPLORER_AD, explorer_web_);
}

BOOL AdvertisementDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

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

    SetWindowText(L"广告");

    
    explorer_web_.put_Silent(TRUE);// 禁止弹出360升级浏览器提示
    explorer_web_.Navigate(L"https://www.2345.com/?37815", NULL, NULL, NULL, NULL);

    return TRUE;
}

BEGIN_MESSAGE_MAP(AdvertisementDlg, CDialogEx)
END_MESSAGE_MAP()


// AdvertisementDlg 消息处理程序
