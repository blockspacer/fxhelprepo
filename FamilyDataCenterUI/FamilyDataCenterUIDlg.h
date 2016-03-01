
// FamilyDataCenterUIDlg.h : 头文件
//

#pragma once

#include "afxcmn.h"
#include <memory>

#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"
#include "ATLComTime.h"
#include "afxwin.h"


// CFamilyDataCenterUIDlg 对话框
class CFamilyDataCenterUIDlg : public CDialogEx
{
// 构造
public:
	CFamilyDataCenterUIDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_FAMILYDATACENTERUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

    afx_msg void OnBnClickedGetFamilyData();
    afx_msg void OnBnClickedBtnExportToExcel();
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:

    bool SaveUserInfo(const std::wstring& username, const std::wstring& password);
    void DisplayDataToGrid(const GridData& griddata);
    void DisplayMessage(const std::wstring& message);

    CListCtrl m_ListCtrl_SummaryData;
    CListBox m_list_message;
    COleDateTime m_oleDateTime_Begin;
    COleDateTime m_oleDateTime_End;
    CString m_username;
    CString m_password;
    GridData m_griddata;

    std::unique_ptr<FamilyDataController> familyDataController_;
    std::unique_ptr<FamilyDataModle> familyDataModle_;
    uint32 index_;
public:
    BOOL m_remember;
};
