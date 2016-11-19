#pragma once
#include "afxwin.h"


// CDlgLogin 对话框
class UserTrackerHelper;

class CLoginDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CLoginDlg)

public:
    CLoginDlg(CWnd* pParent, UserTrackerHelper* tracker_helper);   // 标准构造函数
	virtual ~CLoginDlg();

// 对话框数据
	enum { IDD = IDD_DLG_LOGIN };

    enum
    {
        WM_USER_LOGIN_RESULT = WM_USER + 1,
    };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnPaint();

private:
    LRESULT OnLoginResult(WPARAM wParam, LPARAM lParam);
    void LoginResult(bool result, const std::string& errormsg);
    bool RefreshVerifyCode();

    CImage image;
    CEdit m_edit_account;
    CEdit m_edit_password;
    CEdit m_edit_verifycode;

    UserTrackerHelper* tracker_helper_;
    CStatic m_static_verifycode;
};
