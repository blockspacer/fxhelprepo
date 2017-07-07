#pragma once

#include "explorer_web.h"

// AdvertisementDlg 对话框

class AdvertisementDlg : public CDialogEx
{
	DECLARE_DYNAMIC(AdvertisementDlg)

public:
	AdvertisementDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~AdvertisementDlg();

// 对话框数据
	enum { IDD = IDD_ADVERTISEMENT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()

private:
    CExplorer1 explorer_web_;
};
