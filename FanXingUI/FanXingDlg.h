
// FanXingDlg.h : 头文件
//

#pragma once
#include <memory>
#include <mutex>
#include "explorer1.h"

#include "NetworkHelper.h"
#include "afxwin.h"


// CFanXingDlg 对话框
class CFanXingDlg : public CDialogEx
{
// 构造
public:
    CFanXingDlg(CWnd* pParent = NULL);
    virtual ~CFanXingDlg();

// 对话框数据
	enum { IDD = IDD_FANXING_DIALOG };

    enum
    {
        WM_USER_01 = WM_USER + 1
    };
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
    afx_msg void OnBnClickedButtonClick();
    afx_msg void OnBnClickedButtonRewarstar();
    afx_msg void OnBnClickedButtonRewardgift();
    afx_msg void OnBnClickedButtonNav();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedBtnGetmsg();
    afx_msg void OnBnClickedBtnTest();
    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
    afx_msg void OnClose();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedButton1();

private:
    void Notify(const std::wstring& message);
    bool LoginByWebAction(const CString& username, const CString& password);
    bool LoginByRequest(const std::wstring& username, const std::wstring& password);

    CExplorer1 web_;
    std::unique_ptr<NetworkHelper> network_;
    std::mutex messageMutex_;
    std::vector<std::wstring> messageQueen_;

public:
    CListBox InfoList_;
    int count;
};
