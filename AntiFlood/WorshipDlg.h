#pragma once
#include <vector>
#include <utility>
#include "afxwin.h"
#include "afxdtctl.h"
#include "NetworkHelper.h"
#include "afxcmn.h"


// CWorshipDlg 对话框

class WorshipDlg : public CDialogEx
{
	DECLARE_DYNAMIC(WorshipDlg)

public:
    WorshipDlg(NetworkHelper* network_helper, CWnd* pParent = NULL);   // 标准构造函数
	virtual ~WorshipDlg();

    virtual BOOL OnInitDialog();

// 对话框数据
	enum { IDD = IDD_DLG_WORSHIP };

    afx_msg void OnBnClickedBtnWorship();
    afx_msg void OnBnClickedBtnAddToList();
    afx_msg void OnBnClickedBtnWorshipSelect();
    afx_msg void OnBnClickedBtnDeleteSelect();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()

private:
    CEdit m_edit_fanxing_id;
    CEdit m_edit_roomid;
    CDateTimeCtrl m_time_worship;
    CButton m_check_auto_worship;
    CListCtrl m_list_worship;

    typedef std::pair<std::wstring, std::wstring> worship_pair;
    bool GetSelectItems(std::vector<worship_pair>* select_items);
    NetworkHelper* network_helper_;
};
