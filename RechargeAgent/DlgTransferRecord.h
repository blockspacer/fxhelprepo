#pragma once
#include "afxdialogex.h"
// CDlgTransferRecord 对话框

class CDlgTransferRecord : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgTransferRecord)

public:
	CDlgTransferRecord(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgTransferRecord();

// 对话框数据
	enum { IDD = IDD_DLG_TRANSFER_RECORD };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
};
