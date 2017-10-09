
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
    ~CFamilyDataCenterUIDlg();

// 对话框数据
	enum { IDD = IDD_FAMILYDATACENTERUI_DIALOG };

    enum
    {
        WM_USER_MSG = WM_USER + 1,
        WM_USER_PROGRESS = WM_USER + 2,
        WM_USER_UPDATE_RESULT = WM_USER + 3,
        WM_USER_UPDATE_EFFECT_SINGER = WM_USER + 4,
    };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

    afx_msg void OnBnClickedGetFamilyData();
    afx_msg void OnBnClickedBtnExportToExcel();
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnGetSingerData();
    afx_msg void OnBnClickedBtnGetSingerEffectiveDays();

// 实现
protected:
    HICON m_hIcon;

    // 生成的消息映射函数
    virtual BOOL OnInitDialog();
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
    afx_msg void OnPaint();
    afx_msg HCURSOR OnQueryDragIcon();

    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnUpdateProgress(WPARAM wParam, LPARAM lParam);
    LRESULT OnUpdateResult(WPARAM wParam, LPARAM lParam);
    LRESULT OnUpdateEffectSingers(WPARAM wParam, LPARAM lParam);
    DECLARE_MESSAGE_MAP()

private:
    void DisplayDataToGrid(const std::vector<std::wstring> columnlist, 
        const GridData& griddata);
    void DisplayMessage(const std::wstring& message);

    // 异步回调函数，要通过消息机制切换线程后显示在界面上
    void NotifyMessageCallback(const std::wstring& message);
    void NotifyUpdateProgress(uint32 current, uint32 all);
    void NotifyUpdateResult(const GridData& grid_data);
    void NotifyUpdateEffectSingers(uint32 count);

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

    BOOL m_remember;
    int m_singer_id;
    double m_total_income;
    double m_total_hours;
    int m_new_count;
    CProgressCtrl m_progress1;
    CStatic m_static_room_progress;
};
