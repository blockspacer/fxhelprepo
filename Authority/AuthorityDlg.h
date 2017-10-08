
// AuthorityDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "ATLComTime.h"


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
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedBtnAntiFloodAuthority();
    afx_msg void OnBnClickedBtnView();
    afx_msg void OnBnClickedBtnAdd1Mon();
    afx_msg void OnBnClickedBtn3Mon();
    afx_msg void OnBnClickedBtn6Mon();
    afx_msg void OnBnClickedBtnPackage();

private:
    CEdit m_edit_userid;
    CEdit m_edit_roomid;
    CEdit m_edit_clanid;
    CButton m_chk_kickout;
    CButton m_banchat;
    CButton m_chk_anti_advance;
    COleDateTime m_oleDateTime_End;
    CEdit m_edit_serverip;
public:
    afx_msg void OnBnClickedBtnTrackAuthority();
    afx_msg void OnBnClickedBtnBackgroupAuthority();
};
