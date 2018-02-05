
// SingerRankDlg.h : ͷ�ļ�
//

#pragma once
#include "PhoneRank.h"
#include "afxwin.h"
#include "afxcmn.h"

// CSingerRankDlg �Ի���
class CSingerRankDlg : public CDialogEx
{
// ����
public:
	CSingerRankDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SINGERRANK_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedBtnSearchRank();
    afx_msg void OnBnClickedBtnGetRoomRank();
    virtual void OnOK();

private:
    void MessageCallback(const std::wstring& message);

    PhoneRank phone_rank_;
    CEdit m_edit_roomid;

    int message_index;
    CListCtrl m_singer_list;
    CListBox m_list_info;
public:
    CButton m_chk_beautiful;
private:
    CButton m_chk_new_singer;
public:
    afx_msg void OnBnClickedBtnCityRank();
};
