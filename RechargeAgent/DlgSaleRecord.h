#pragma once
#include "afxdialogex.h"
// CDlgSaleRecord 对话框

class CDlgSaleRecord : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSaleRecord)

public:
	CDlgSaleRecord(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgSaleRecord();

// 对话框数据
	enum { IDD = IDD_DLG_SALE_RECORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
