
// FamilyDataCenterUIDlg.h : ͷ�ļ�
//

#pragma once

#include "afxcmn.h"
#include <memory>

#include "FamilyDataCenterUI/FamilyDataController.h"
#include "FamilyDataCenterUI/FamilyDataModle.h"
#include "ATLComTime.h"
#include "afxwin.h"


// CFamilyDataCenterUIDlg �Ի���
class CFamilyDataCenterUIDlg : public CDialogEx
{
// ����
public:
	CFamilyDataCenterUIDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_FAMILYDATACENTERUI_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

    afx_msg void OnBnClickedGetFamilyData();
    afx_msg void OnBnClickedBtnExportToExcel();
    afx_msg void OnBnClickedBtnLogin();
    afx_msg void OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult);

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:

    void DisplayDataToGrid(const std::vector<std::wstring> columnlist, 
        const GridData& griddata);
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
    int m_singer_id;
    double m_total_income;
    double m_total_hours;
    int m_new_count;
    afx_msg void OnBnClickedBtnGetSingerData();
    afx_msg void OnBnClickedBtnGetNewSinger();
};
