// GiftThanksSetting.cpp : 实现文件
//

#include "stdafx.h"
#include "AntiFlood.h"
#include "GiftThanksSettingDlg.h"
#include "afxdialogex.h"


// GiftThanksSetting 对话框

IMPLEMENT_DYNAMIC(GiftThanksSettingDlg, CDialogEx)

GiftThanksSettingDlg::GiftThanksSettingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(GiftThanksSettingDlg::IDD, pParent)
{

}

GiftThanksSettingDlg::~GiftThanksSettingDlg()
{
}

void GiftThanksSettingDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(GiftThanksSettingDlg, CDialogEx)
END_MESSAGE_MAP()


// GiftThanksSetting 消息处理程序
