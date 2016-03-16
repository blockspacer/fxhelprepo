// DlgGiftNotify.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgGiftNotify.h"
#include "afxdialogex.h"
#include "third_party/chromium/base/basictypes.h"

// CDlgGiftNotify 对话框

IMPLEMENT_DYNAMIC(CDlgGiftNotify, CDialogEx)

CDlgGiftNotify::CDlgGiftNotify(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGiftNotify::IDD, pParent)
    , m_room_left(0)
    , m_room_right(0)
    , m_time_all(0)
    , m_time_left(0)
{
    networkLeft_.reset(new NetworkHelper);
    networkLeft_->Initialize();
    networkRight_.reset(new NetworkHelper);
    networkRight_->Initialize();
}

CDlgGiftNotify::~CDlgGiftNotify()
{
    networkLeft_->Finalize();
    networkRight_->Finalize();
}

void CDlgGiftNotify::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_ROOM_LEFT, m_room_left);
    DDX_Text(pDX, IDC_EDIT_ROOM_RIGHT, m_room_right);
    DDX_Text(pDX, IDC_EDIT_TIME_ALL, m_time_all);
    DDX_Text(pDX, IDC_EDIT_TIME_LEFT, m_time_left);
    DDX_Control(pDX, IDC_LIST_LEFT, m_list_left);
    DDX_Control(pDX, IDC_LIST_RIGHT, m_list_right);
}


BEGIN_MESSAGE_MAP(CDlgGiftNotify, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_BEGIN, &CDlgGiftNotify::OnBnClickedBtnBegin)
END_MESSAGE_MAP()


// CDlgGiftNotify 消息处理程序


void CDlgGiftNotify::OnBnClickedBtnBegin()
{
    UpdateData(TRUE);
    // 检查变量合理性

    // 分别进入房间
    networkLeft_->SetNotify(
        std::bind(&CDlgGiftNotify::Notify, this, std::placeholders::_1));
    networkLeft_->SetNotify601(
        std::bind(&CDlgGiftNotify::Notify601Left, this, std::placeholders::_1));

    networkLeft_->EnterRoom(m_room_left);

    //networkRight_->SetNotify(
    //    std::bind(&CDlgGiftNotify::Notify, this, std::placeholders::_1));
    //networkRight_->SetNotify601(
    //    std::bind(&CDlgGiftNotify::Notify601Right, this, std::placeholders::_1));
    //networkRight_->EnterRoom(m_room_right);
}

void CDlgGiftNotify::Notify(const std::wstring& message)
{
    return;
}

void CDlgGiftNotify::Notify601Left(const RoomGiftInfo601& roomgiftinfo)
{
    return;
}

void CDlgGiftNotify::Notify601Right(const RoomGiftInfo601& roomgiftinfo)
{
    return;
}
