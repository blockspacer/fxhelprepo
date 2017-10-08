
// AuthorityDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "ATLComTime.h"


// CAuthorityDlg �Ի���
class CAuthorityDlg : public CDialogEx
{
// ����
public:
	CAuthorityDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_AUTHORITY_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
    virtual void OnOK();
    virtual void OnCancel();

    DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg void OnBnClickedCancel();
    afx_msg void OnBnClickedBtnAntiFloodAuthority();
    afx_msg void OnBnClickedBtnView();
    afx_msg void OnBnClickedBtnAdd1Mon();
    afx_msg void OnBnClickedBtn3Mon();
    afx_msg void OnBnClickedBtn6Mon();
    afx_msg void OnBnClickedBtnPackage();

private:
    CEdit m_edit_userid;
    CEdit m_edit_roomid;
    CEdit m_edit_clanid;
    CButton m_chk_kickout;
    CButton m_banchat;
    CButton m_chk_anti_advance;
    COleDateTime m_oleDateTime_End;
    CEdit m_edit_serverip;
public:
    afx_msg void OnBnClickedBtnTrackAuthority();
    afx_msg void OnBnClickedBtnBackgroupAuthority();
};
