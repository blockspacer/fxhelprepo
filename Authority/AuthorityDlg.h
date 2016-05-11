
// AuthorityDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CAuthorityDlg 对话框
class CAuthorityDlg : public CDialogEx
{
// 构造
public:
	CAuthorityDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_AUTHORITY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedOk();
    afx_msg void OnBnClickedCancel();
    CEdit m_edit_userid;
    CEdit m_edit_roomid;
    CEdit m_edit_clanid;
    afx_msg void OnBnClickedBtnGenerate();
    afx_msg void OnBnClickedBtnView();
    CButton m_chk_kickout;
    CButton m_banchat;
    CButton m_chk_anti_advance;
};
