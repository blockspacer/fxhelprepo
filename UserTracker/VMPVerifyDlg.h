#pragma once
#include "afxwin.h"


// CVMPVerifyDlg �Ի���

class CVMPVerifyDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVMPVerifyDlg)

public:
	CVMPVerifyDlg(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CVMPVerifyDlg();

    afx_msg void OnBnClickedOk();

// �Ի�������
	enum { IDD = IDD_DLG_KEY_CHECK };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��
    virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
private:
    CEdit m_edit_hardware_code;
    CEdit m_edit_authority_key;
    std::string serial_;
};
