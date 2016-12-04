#pragma once
#include "resource.h"
#include <vector>
#include <utility>
#include "afxwin.h"
#include "afxdtctl.h"
#include "WorshipHelper.h"
#include "afxcmn.h"


// CWorshipDlg 对话框

class WorshipDlg : public CDialogEx
{
	DECLARE_DYNAMIC(WorshipDlg)

public:
    WorshipDlg(WorshipHelper* network_helper, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~WorshipDlg();

    virtual BOOL OnInitDialog();

// 对话框数据
	enum { IDD = IDD_DLG_WORSHIP };

    enum
    {
        WM_USER_MSG = WM_USER + 1,
    };

    afx_msg void OnBnClickedBtnWorship();
    afx_msg void OnBnClickedBtnAddToList();
    afx_msg void OnBnClickedBtnWorshipSelect();
    afx_msg void OnBnClickedBtnDeleteSelect();
    afx_msg void OnBnClickedChkAutoWorship();
    afx_msg void OnBnClickedBtnEnter();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    void SetHScroll();
    void TipMessageCallback(const std::wstring& message);
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
    CEdit m_edit_fanxing_id;
    CEdit m_edit_roomid;
    CDateTimeCtrl m_time_worship;
    CButton m_check_auto_worship;
    CListCtrl m_list_worship;

    typedef std::pair<std::wstring, std::wstring> worship_pair;
    bool GetSelectItems(std::vector<worship_pair>* select_items);
    WorshipHelper* worship_helper_;
    int list_info_count_;
    CListBox m_list_message;


};
