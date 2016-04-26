
// FanXingDlg.h : 头文件
//

#pragma once
#include <memory>
#include <mutex>

#include "NetworkHelper.h"
#include "BlacklistHelper.h"
#include "afxwin.h"
#include "afxcmn.h"


// CFanXingDlg 对话框
class CFanXingDlg : public CDialogEx
{
// 构造
public:
    CFanXingDlg(CWnd* pParent = NULL);
    virtual ~CFanXingDlg();

// 对话框数据
	enum { IDD = IDD_FANXING_DIALOG };

    enum
    {
        WM_USER_01 = WM_USER + 1,
        WM_USER_ADD_ENTER_ROOM_INFO = WM_USER + 2,
        WM_USER_ADD_TO_BLACK_LIST = WM_USER + 3,
    };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    afx_msg void OnBnClickedButtonRewarstar();
    afx_msg void OnBnClickedButtonRewardgift();
    afx_msg void OnBnClickedButtonNav();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedBtnGetmsg();
    afx_msg void OnBnClickedBtnAdd();
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayDataToViewerList(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayDtatToBlackList(WPARAM wParam, LPARAM lParam);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
    afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedButtonLogin();

private:
    void Notify(const std::wstring& message);
    void Notify201(const RowData& rowdata);
    bool LoginByRequest(const std::wstring& username, const std::wstring& password);
	bool GetSelectViewers(std::vector<EnterRoomUserInfo>* enterRoomUserInfos);
    bool GetSelectBlacks(std::vector<EnterRoomUserInfo>* enterRoomUserInfos);
    bool KickOut(const std::vector<EnterRoomUserInfo>& enterRoomUserInfos,
        KICK_TYPE kicktype);

    std::unique_ptr<NetworkHelper> network_;
    std::mutex messageMutex_;
    std::vector<std::wstring> messageQueen_;

    std::mutex viewerRowdataMutex_;
    std::vector<RowData> viewerRowdataQueue_;

    std::mutex blackRowdataMutex_;
    std::vector<RowData> blackRowdataQueue_;

    uint32 listCtrlRowIndex_;
    CButton m_check_remember;
    uint32 roomid_ = 0;
    uint32 singerid_ = 0;

    CString m_query_key;
    CListBox InfoList_;
    int infoListCount_;
    CListCtrl m_ListCtrl_Viewers;
    CListCtrl m_ListCtrl_Blacks;
    std::unique_ptr<BlacklistHelper> blacklistHelper_;

    afx_msg void OnBnClickedButton2();
    afx_msg void OnHdnItemclickListUserStatus(NMHDR *pNMHDR, LRESULT *pResult);

    static int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
    afx_msg void OnBnClickedButtonRemove();
    afx_msg void OnBnClickedBtnQuery();
    afx_msg void OnBnClickedBtnSelectAll();
    afx_msg void OnBnClickedBtnSelectReverse();
    afx_msg void OnBnClickedBtnKickoutMonth();
    afx_msg void OnBnClickedBtnKickoutHour();
    afx_msg void OnBnClickedBtnSilent();
    afx_msg void OnBnClickedBtnUnsilent();  
    afx_msg void OnBnClickedBtnClear();

    afx_msg void OnBnClickedBtnGetViewerList();

    afx_msg void OnBnClickedBtnKickoutMonthBlack();
    afx_msg void OnBnClickedBtnKickoutHourBlack();
    afx_msg void OnBnClickedBtnSilentBlack();
    afx_msg void OnBnClickedBtnUnsilentBlack();
    afx_msg void OnBnClickedBtnSelectAllBlack();
    afx_msg void OnBnClickedBtnSelectReverseBlack();
    afx_msg void OnBnClickedBtnRemoveBlack();
    afx_msg void OnBnClickedBtnLoadBlack();
    afx_msg void OnBnClickedBtnAddToBlack();
public:
    afx_msg void OnBnClickedBtnSaveBlack();
};
