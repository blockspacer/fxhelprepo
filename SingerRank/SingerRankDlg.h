
// SingerRankDlg.h : 头文件
//

#pragma once
#include <vector>
#include "PhoneRank.h"
#include "afxwin.h"
#include "afxcmn.h"

// CSingerRankDlg 对话框
class CSingerRankDlg : public CDialogEx
{
// 构造
public:
	CSingerRankDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SINGERRANK_DIALOG };

    enum
    {
        WM_USER_MSG = WM_USER + 1,
        WM_USER_PROGRESS = WM_USER + 2,
        WM_USER_FOUND_RESULT = WM_USER + 3,
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

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
    virtual void OnClose();
    afx_msg void OnOK();
    afx_msg void OnPaint();

	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedBtnSearchRank();
    afx_msg void OnBnClickedBtnGetRoomRank();
    afx_msg void OnBnClickedBtnCityRank();

protected:
    LRESULT OnMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnProgress(WPARAM wParam, LPARAM lParam);
    LRESULT OnFoundResult(WPARAM wParam, LPARAM lParam);

private:
    void SingerInfoCallback(const GridData& singer_infos);
    void MessageCallback(const std::wstring& message);

    PhoneRank phone_rank_;
    CEdit m_edit_roomid;
    int message_index;
    CListCtrl m_singer_list;
    CListBox m_list_info;
    CButton m_chk_beautiful;
    CButton m_chk_new_singer;

};
