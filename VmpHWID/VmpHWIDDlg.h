
// VmpHWIDDlg.h : header file
//

#pragma once
#include "afxwin.h"


// CVmpHWIDDlg dialog
class CVmpHWIDDlg : public CDialogEx
{
// Construction
public:
	CVmpHWIDDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VMPHWID_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
    CEdit m_edit_hwid;
public:
    afx_msg void OnBnClickedOk();
};
