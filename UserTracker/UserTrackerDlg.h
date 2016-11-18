
// UserTrackerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CUserTrackerDlg 对话框
class CUserTrackerDlg : public CDialogEx
{
// 构造
public:
	CUserTrackerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_USERTRACKER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

    DECLARE_MESSAGE_MAP()

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnGetallroomdata();
    afx_msg void OnBnClickedBtnFindInCache();
    afx_msg void OnBnClickedBtnUpdataFind();

private:
    CEdit m_edit_target_fanxing_id;
    CListBox m_list_message;
};
