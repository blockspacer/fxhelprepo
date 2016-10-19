
// BatchLoginDlg.h : 头文件
//

#pragma once

#include <memory>
#include <string>
#include <mutex>
#include "afxcmn.h"

#include "Network/EncodeHelper.h"
#include "afxwin.h"

class UserRoomManager;
class TcpManager;
// CBatchLoginDlg 对话框
class CBatchLoginDlg : public CDialogEx
{
// 构造
public:
	CBatchLoginDlg(CWnd* pParent = NULL);	// 标准构造函数
    virtual ~CBatchLoginDlg();
// 对话框数据
	enum { IDD = IDD_BATCHLOGIN_DIALOG };

    enum
    {
        WM_USER_NOTIFY_MESSAGE = WM_USER + 1,
        WM_USER_USER_LIST_INFO = WM_USER + 2,
        WM_USER_ROOM_LIST_INFO = WM_USER + 3,
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
    afx_msg void OnBnClickedBtnImportUser();
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnBnClickedBtnGetProxy();
    afx_msg void OnBnClickedBtnBatchEnterRoom();
    afx_msg void OnBnClickedBtnImportRoom();
    afx_msg void OnBnClickedBtnUpMvBillboard();
    afx_msg void OnBnClickedBtnSaveUserPwdCookie();
    afx_msg void OnLvnItemchangedListRoom(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnNMClickListRoom(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnSendAward();
    afx_msg void OnBnClickedBtnLottery();
	DECLARE_MESSAGE_MAP()

private:
    void Notify(const std::wstring& message);
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayDataToUserList(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayDataToRoomList(WPARAM wParam, LPARAM lParam);

    std::unique_ptr<UserRoomManager> userRoomManager_;
    std::unique_ptr<TcpManager> tcpManager_;
    CListCtrl m_ListCtrl_Users;
    CListCtrl m_ListCtrl_Rooms;
    CListCtrl m_list_proxy;
    CEdit m_mv_collection_id;
    CEdit m_mv_id;
    CListBox InfoList_;
    int infoListCount_ = 0;

    std::mutex messageMutex_;
    std::vector<std::wstring> messageQueen_;    
    CEdit m_roomid;
};
