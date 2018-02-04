
// SingerRankDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SingerRank.h"
#include "SingerRankDlg.h"
#include "afxdialogex.h"
#include "PhoneRank.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "Network/CurlWrapper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSingerRankDlg �Ի���



CSingerRankDlg::CSingerRankDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSingerRankDlg::IDD, pParent)
    , message_index(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSingerRankDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SINGER_ID, m_edit_roomid);
    DDX_Control(pDX, IDC_LIST_SINGERS, m_singer_list);
    DDX_Control(pDX, IDC_LIST_INFO, m_list_info);
}

BEGIN_MESSAGE_MAP(CSingerRankDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_SEARCH_RANK, &CSingerRankDlg::OnBnClickedBtnSearchRank)
    ON_BN_CLICKED(IDC_BTN_GET_ROOM_RANK, &CSingerRankDlg::OnBnClickedBtnGetRoomRank)
END_MESSAGE_MAP()


// CSingerRankDlg ��Ϣ�������

BOOL CSingerRankDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������
    CurlWrapper::CurlInit();


	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSingerRankDlg::OnOK()
{

}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSingerRankDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSingerRankDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSingerRankDlg::OnBnClickedBtnSearchRank()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    phone_rank_.InitNewSingerRankInfos();

}


void CSingerRankDlg::OnBnClickedBtnGetRoomRank()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
    CString cs_roomid;
    m_edit_roomid.GetWindowTextW(cs_roomid);
    uint32 roomid = 0;
    base::StringToUint(base::WideToUTF8(cs_roomid.GetBuffer()), &roomid);
    uint32 rank = 0;
    uint32 all = 0;
    if (!phone_rank_.GetSingerRankByRoomid(roomid, &rank, &all))
    {
        std::wstring w_message = L"�޷���ȡ����";
        m_list_info.InsertString(message_index++, w_message.c_str());
        return;
    }


    std::string message;
    message += base::UintToString(roomid);
    message += " rank ( ";
    message += base::UintToString(rank);
    message += "/";
    message += base::UintToString(all);
    message += " )";

    m_list_info.InsertString(message_index++, base::UTF8ToWide(message).c_str());

}
