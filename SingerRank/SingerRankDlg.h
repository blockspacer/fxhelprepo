
// SingerRankDlg.h : ͷ�ļ�
//

#pragma once
#include <vector>
#include "PhoneRank.h"
#include "SingerRetriver.h"
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

    enum
    {
        WM_USER_MSG = WM_USER + 1,
        WM_USER_PROGRESS = WM_USER + 2,
        WM_USER_FOUND_RESULT = WM_USER + 3,
    };

    enum ProgressType
    {
        PROGRESSTYPE_ROOM = 1,
        PROGRESSTYPE_USER = 2,
    };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
    virtual void OnClose();
    afx_msg void OnOK();
    afx_msg void OnPaint();

	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    afx_msg void OnBnClickedBtnSearchRank();
    afx_msg void OnBnClickedBtnGetRoomRank();
    afx_msg void OnBnClickedBtnCityRank();
    afx_msg void OnBnClickedBtnClanRetrive();

protected:
    LRESULT OnMessage(WPARAM wParam, LPARAM lParam);
    LRESULT OnProgress(WPARAM wParam, LPARAM lParam);
    LRESULT OnFoundResult(WPARAM wParam, LPARAM lParam);

private:
    void SingerInfoCallback(uint32 roomid, bool result, const RowData& singer_infos);
    void MessageCallback(const std::wstring& message);
    void ClanSingerCallback(const std::vector<uint32>& roomids);
    void OneSingerInfoCallback(const RowData& singer_info);

    PhoneRank phone_rank_;
    SingerRetriver singer_retriver_;
    base::Thread worker_thread_;

    CEdit m_edit_roomid;
    int message_index;
    CListCtrl m_singer_list;
    CListBox m_list_info;
    CButton m_chk_beautiful;
    CButton m_chk_new_singer;
    CEdit m_edit_clan;
};
