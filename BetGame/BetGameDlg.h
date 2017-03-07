
// BetGameDlg.h : 头文件
//

#pragma once

#include <memory>
#include "afxcmn.h"
#include "afxwin.h"

#include "BetNetworkHelper.h"
#include "ButtonBasic.h"

class BetNetworkHelper;
struct BetResult;
// CBetGameDlg 对话框
class CBetGameDlg : public CDialogEx
{
// 构造
public:
	CBetGameDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_BETGAME_DIALOG };

    enum
    {
        WM_USER_TIPS = WM_USER + 1,
        WM_USER_BET_RESULT = WM_USER + 2,
        WM_USER_BET_TIME = WM_USER + 3
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
	DECLARE_MESSAGE_MAP()

private:
    afx_msg void OnBnClickedButtonEnterRoom();
    afx_msg void OnBnClickedButtonLogin();
    afx_msg void OnHdnBegintrackListBetdata(NMHDR *pNMHDR, LRESULT *pResult);

    void TipMessageCallback(const std::wstring& message);
    void DisplayBetResultCallback(const CaculationData& bet_result);
    void DisplayBetTimeCallback(uint32 time);

    LRESULT OnTipsMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayBetResult(WPARAM wParam, LPARAM lParam);
    LRESULT OnDisplayBetTime(WPARAM wParam, LPARAM lParam);

    void CreateFont();

    int message_count_;
    CListBox m_list_msg;
    CListCtrl m_list_bet_data;
    CEdit m_edit_username;
    CEdit m_edit_password;
    CEdit m_edit_room_id;
    CButton m_check_remember;

    std::unique_ptr<BetNetworkHelper> bet_network_;
    CEdit m_edit_display_result;

    CFont font_bet_basic_;
    CFont font_bet_count_;
    CFont font_bet_space_;
    CFont font_bet_result_red_;
    CFont font_bet_result_black_;
    CFont font_bet_result_blue_;

    CButtonBasic m_btn_basic[8];
    CStatic m_static_space[8];
    CStatic m_static_count[8];
};
