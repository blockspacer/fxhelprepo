
// IpProxyAchieverDlg.h : 头文件
//

#pragma once
#include <memory>

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#include <afxcontrolbars.h>     // 功能区和控件条的 MFC 支持

// CIpProxyAchieverDlg 对话框
class CIpProxyAchieverDlg : public CDialogEx
{
// 构造
public:
	CIpProxyAchieverDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_IPPROXYACHIEVER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    afx_msg void OnBnClickedBtnGetProxy();
};
