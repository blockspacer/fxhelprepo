#pragma once


// GiftThanksSetting 对话框

class GiftThanksSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(GiftThanksSettingDlg)

public:
	GiftThanksSettingDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~GiftThanksSettingDlg();

// 对话框数据
	enum { IDD = IDD_DLG_THANKS_SETTING };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
