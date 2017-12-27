#pragma once
#include "afxwin.h"


// CVMPVerifyDlg 对话框

class CVMPVerifyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVMPVerifyDlg)

public:
	CVMPVerifyDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CVMPVerifyDlg();

    afx_msg void OnBnClickedOk();

// 对话框数据
	enum { IDD = IDD_DLG_KEY_CHECK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
    CEdit m_edit_hardware_code;
    CEdit m_edit_authority_key;
    std::string serial_;
};
