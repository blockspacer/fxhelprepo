
// LibEventClientDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "ClientController.h"
#include <set>


// CLibEventClientDlg 对话框
class CLibEventClientDlg : public CDialogEx
{
// 构造
public:
	CLibEventClientDlg(CWnd* pParent = NULL);	// 标准构造函数

    ~CLibEventClientDlg();

// 对话框数据
	enum { IDD = IDD_LIBEVENTCLIENT_DIALOG };

    enum
    {
        WM_USER_DISPLAY_MESSAGE = WM_USER + 1,
    };

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
    afx_msg void OnBnClickedBtnSendMsg();

	DECLARE_MESSAGE_MAP()

    void SetHScroll();
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);

private:
    CEdit m_edit_ip;
    CEdit m_edit_port;
    CEdit m_edit_target_count;
    int message_index_ = 0;
    CListBox m_list_message;
    CListCtrl m_listctrl_ip_target_status;
    CEdit m_edit_message_send;

    ClientController client_controller_;

    void ConnectNotify(const std::string& ip, const std::string& port, 
                       bool result, TCPHANDLE handle);
    void DataReceiveNotify(
        const std::string& ip, const std::string& port, 
        bool result, std::vector<uint8>& data);

    void SendCallback(const std::string& ip, const std::string& port,
                      bool result);

    std::map<TCPHANDLE, std::pair<std::string, std::string>> handles_;
};
