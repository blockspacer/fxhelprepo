#pragma once
#include <memory>
#include <vector>
#include <mutex>
#include "resource.h"
#include "afxwin.h"
#include "afxcmn.h"
#include "Network/IpProxy.h"

class NetworkHelper;
class RegisterHelper;
// DlgRegister 对话框

class CDlgRegister : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgRegister)

public:
	CDlgRegister(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CDlgRegister();

// 对话框数据
	enum { IDD = IDD_DLG_REGISTER ,
        WM_USER_REGISTER_INFO = WM_USER + 1,
    };

    afx_msg void OnPaint();
    afx_msg LRESULT OnHotKey(WPARAM wp, LPARAM lp);
    afx_msg void OnBnClickedBtnCheckExist();
    afx_msg void OnBnClickedBtnRegister();
    afx_msg void OnBnClickedBtnVerifyCode();
    afx_msg void OnNMCustomdrawListIpProxy(NMHDR *pNMHDR, LRESULT *pResult);

    // 代理列表操作
    afx_msg void OnBnClickedBtnImportProxy();
    afx_msg void OnNMClickListIpProxy(NMHDR *pNMHDR, LRESULT *pResult);
    afx_msg void OnBnClickedBtnAddProxy();
    afx_msg void OnBnClickedBtnSelectAll();
    afx_msg void OnBnClickedBtnSelectReverse();
    afx_msg void OnBnClickedBtnRemoveSelect();
    afx_msg void OnBnClickedBtnSaveProxy();
    afx_msg void OnBnClickedBtnChkProxy();

protected:
    virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
    virtual void OnOK();

	DECLARE_MESSAGE_MAP()

    LRESULT OnNotifyMessage(WPARAM wParam, LPARAM lParam);
    void Notify(const std::wstring& message);
private:

    bool GetIpProxy(IpProxy* ipproxy);
    CStatic m_static_verifycode;
    CEdit m_register_username;
    CEdit m_register_password;
    CEdit m_register_verifycode;
    CImage image;
    CListBox m_register_info_list;
    CListCtrl m_listctrl_ip_proxy;

    std::unique_ptr<RegisterHelper> registerHelper_;
    std::vector<IpProxy> ipProxys_;

    int infoListCount_;
    std::mutex messageMutex_;
    std::vector<std::wstring> messageQueen_;
    std::unique_ptr<CFont> font18_;
    CButton m_use_proxy;
};
