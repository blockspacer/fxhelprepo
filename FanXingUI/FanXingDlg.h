
// FanXingDlg.h : ͷ�ļ�
//

#pragma once
#include "explorer1.h"


// CFanXingDlg �Ի���
class CFanXingDlg : public CDialogEx
{
// ����
public:
	CFanXingDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_FANXING_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedButton1();

private:
    CExplorer1 web_;
public:
    afx_msg void OnBnClickedButtonClick();
    afx_msg void OnBnClickedButtonRewarstar();
    afx_msg void OnBnClickedButtonRewardgift();
    afx_msg void OnBnClickedButtonNav();
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnBnClickedBtnGetmsg();
    afx_msg void OnBnClickedBtnTest();
};