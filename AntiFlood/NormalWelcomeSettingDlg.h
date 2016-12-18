#pragma once
#include "afxwin.h"
#include <string>


// CNormalWelcomeSetting 对话框

class NormalWelcomeSettingDlg : public CDialogEx
{
	DECLARE_DYNAMIC(NormalWelcomeSettingDlg)

public:
	NormalWelcomeSettingDlg(const std::wstring& welcome, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~NormalWelcomeSettingDlg();
    virtual BOOL OnInitDialog();
// 对话框数据
	enum { IDD = IDD_DLG_NORMAL_WELCOME };

    std::wstring GetNormalWelcome() const;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();

	DECLARE_MESSAGE_MAP()
private:
    CEdit m_edit_welcome_tips;
    CEdit m_edit_normal_welcome_setting;

    std::wstring normal_welcome_setting_;

};
