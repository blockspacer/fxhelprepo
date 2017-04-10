
// HousingGzccFetchDlg.h : 头文件
//

#pragma once

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/files/file_path.h"

#include "afxwin.h"
#include "afxcmn.h"

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;

// CHousingGzccFetchDlg 对话框
class CHousingGzccFetchDlg : public CDialogEx
{
// 构造
public:
	CHousingGzccFetchDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_HOUSINGGZCCFETCH_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

    afx_msg void OnBnClickedBtnFetchData();
    afx_msg void OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnExport();

	DECLARE_MESSAGE_MAP()

private:

    void DisplayDataToGrid(const std::vector<std::wstring> columnlist,
        const GridData& griddata);

    bool ExportToExcel();

    CEdit m_edit_max_pages;
    CListCtrl m_list_display;

    GridData m_griddata;
    base::FilePath exePath_;
};
