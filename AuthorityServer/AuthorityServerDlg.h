
// AuthorityServerDlg.h : 头文件
//

#pragma once
#include <memory>
#include "AuthorityNetwork.h"

#include "afxwin.h"

class AuthorityNetwork;

// CAuthorityServerDlg 对话框
class CAuthorityServerDlg : public CDialogEx
{
// 构造
public:
	CAuthorityServerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_AUTHORITYSERVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnRunService();
    afx_msg void OnBnClickedBtnStopService();
    afx_msg void OnBnClickedBtnQuery();

// 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();

    DECLARE_MESSAGE_MAP()

public:
    CListBox m_list_msg;
    CEdit m_edit_query;

    std::unique_ptr<AuthorityNetwork> authority_network_;
};
