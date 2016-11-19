
// UserTrackerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"

#include <memory>
#include <mutex>
#include "UserTrackerHelper.h"
#include "afxcmn.h"

class UserTrackerHelper;

// CUserTrackerDlg 对话框
class CUserTrackerDlg : public CDialogEx
{
// 构造
public:
	CUserTrackerDlg(CWnd* pParent,
        UserTrackerHelper* tracker_helper);	// 标准构造函数
    ~CUserTrackerDlg();

// 对话框数据
	enum { IDD = IDD_USERTRACKER_DIALOG };

    enum
    {
        WM_USER_MSG = WM_USER + 1,
        WM_USER_PROGRESS = WM_USER + 2,
        WM_USER_FOUND_RESULT = WM_USER +3,
    };

    enum ProgressType
    {
        PROGRESSTYPE_ROOM = 1,
        PROGRESSTYPE_USER = 2,
    };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

    DECLARE_MESSAGE_MAP()

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
    virtual void OnClose();
    afx_msg void OnOK();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnBnClickedBtnGetAllRoomData();
    afx_msg void OnBnClickedBtnFindInCache();
    afx_msg void OnBnClickedBtnUpdataFind();
    afx_msg void OnBnClickedBtnCancel();

protected:
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnRoomProgress(WPARAM wParam, LPARAM lParam);
    LRESULT OnFoundResult(WPARAM wParam, LPARAM lParam);

private:
    void SetHScroll();
    void NotifyMessage(const std::wstring& message);
    void RoomProgress(uint32 current, uint32 all);
    void FoundResult(uint32 user_id, uint32 room_id);

    CEdit m_edit_target_fanxing_id;
    CListBox m_list_message;
    int list_info_count;

    UserTrackerHelper* tracker_helper_;

    std::mutex messageMutex_;
    std::vector<std::wstring> messageQueen_;
    CEdit m_edit_account;
    CEdit m_edit_password;
    CStatic m_static_room_progress;
    CProgressCtrl m_progress1;
    CListCtrl m_list_result;
    int result_index = 0;
};
