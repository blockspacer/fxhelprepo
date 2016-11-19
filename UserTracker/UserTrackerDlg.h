
// UserTrackerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include <memory>
#include <mutex>
#include "UserTrackerHelper.h"

class UserTrackerHelper;

// CUserTrackerDlg 对话框
class CUserTrackerDlg : public CDialogEx
{
// 构造
public:
	CUserTrackerDlg(CWnd* pParent = NULL);	// 标准构造函数
    ~CUserTrackerDlg();

// 对话框数据
	enum { IDD = IDD_USERTRACKER_DIALOG };

    enum
    {
        WM_USER_MSG = WM_USER + 1,
    };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

    DECLARE_MESSAGE_MAP()

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
    afx_msg void OnOK();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnBnClickedBtnGetAllRoomData();
    afx_msg void OnBnClickedBtnFindInCache();
    afx_msg void OnBnClickedBtnUpdataFind();
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);

private:
    void SetHScroll();
    void Notify(const std::wstring& message);
    CEdit m_edit_target_fanxing_id;
    CListBox m_list_message;
    int list_info_count;

    std::unique_ptr<UserTrackerHelper> tracker_helper_;

    std::mutex messageMutex_;
    std::vector<std::wstring> messageQueen_;
    CEdit m_edit_account;
    CEdit m_edit_password;
public:
    afx_msg void OnBnClickedBtnCancel();
};
