
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

    enum
    {
        WM_USER_DISPLAY_MESSAGE = WM_USER + 1,
    };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持



// 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();

    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnRunService();
    afx_msg void OnBnClickedBtnStopService();
    afx_msg void OnBnClickedBtnQuery();
    afx_msg void OnClose();

    DECLARE_MESSAGE_MAP()

    void SetHScroll();
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);

private:
    void NotifyMessage(const std::wstring& msg);
    int msg_index_ = 0;
    CListBox m_list_msg;
    CEdit m_edit_query;

    std::unique_ptr<AuthorityNetwork> authority_network_;
};
