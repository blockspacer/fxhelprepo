
// LibEventClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ClientController.h"


// CLibEventClientDlg 对话框
class CLibEventClientDlg : public CDialogEx
{
// 构造
public:
	CLibEventClientDlg(CWnd* pParent = NULL);	// 标准构造函数

    ~CLibEventClientDlg();

// 对话框数据
	enum { IDD = IDD_LIBEVENTCLIENT_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedBtnBegin();
	DECLARE_MESSAGE_MAP()

private:
    CEdit m_edit_ip;
    CEdit m_edit_port;
    CEdit m_edit_target_count;
    CListBox m_list_message;
    CListCtrl m_listctrl_ip_target_status;

    ClientController client_controller_;

    void ConnectNotify(bool result);
    void DataReceiveNotify(bool result, std::vector<uint8>& data);
    
};
