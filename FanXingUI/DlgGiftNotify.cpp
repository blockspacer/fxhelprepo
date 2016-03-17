// DlgGiftNotify.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgGiftNotify.h"
#include "afxdialogex.h"
#include "GiftInfoHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

// CDlgGiftNotify 对话框

IMPLEMENT_DYNAMIC(CDlgGiftNotify, CDialogEx)

CDlgGiftNotify::CDlgGiftNotify(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGiftNotify::IDD, pParent)
    , m_room_left(0)
    , m_room_right(0)
    , m_time_all(0)
    , m_time_left(0)
    , m_coin_left(0)
    , m_coin_right(0)
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
    DDX_Text(pDX, IDC_EDIT_LEFT_GIFT, m_coin_left);
    DDX_Text(pDX, IDC_EDIT_RIGHT_GIFT, m_coin_right);
}


BEGIN_MESSAGE_MAP(CDlgGiftNotify, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_BEGIN, &CDlgGiftNotify::OnBnClickedBtnBegin)
    ON_MESSAGE(WM_USER_ADD_GIFT_INFO, &CDlgGiftNotify::OnAddGiftInfo)
    ON_MESSAGE(WM_USER_UPDATE_TOTAL_COUNT, &CDlgGiftNotify::OnUpdateTotalCount)
END_MESSAGE_MAP()


// CDlgGiftNotify 消息处理程序
void CDlgGiftNotify::OnBnClickedBtnBegin()
{
    UpdateData(TRUE);

    // 获取房间礼物列表
    std::string giftliststr;
    bool result = networkLeft_->GetGiftList(m_room_left);
    if (!result)
    {
        return;
    }

    // 分别进入房间
    networkLeft_->SetNotify(
        std::bind(&CDlgGiftNotify::Notify, this, std::placeholders::_1));
    networkLeft_->SetNotify601(
        std::bind(&CDlgGiftNotify::Notify601, this, ROOM_TYPE::ROOM_LEFT, 
        std::placeholders::_1, std::placeholders::_2));

    networkLeft_->EnterRoom(m_room_left);

    //networkRight_->SetNotify(
    //    std::bind(&CDlgGiftNotify::Notify, this, std::placeholders::_1));
    //networkRight_->SetNotify601(
    //    std::bind(&CDlgGiftNotify::Notify601, this, ROOM_TYPE::ROOM_RIGHT,  std::placeholders::_1));
    //networkRight_->EnterRoom(m_room_right);
}

LRESULT CDlgGiftNotify::OnAddGiftInfo(WPARAM wParam, LPARAM lParam)
{
    std::vector<UserGiftAccumulative> newmessage;
    messageLock_.lock();
    newmessage.swap(messageQueue_);
    messageLock_.unlock();

    for (const auto&it : newmessage)
    {
        std::wstring wstr = base::UTF8ToUTF16(it.tips);
        switch (it.roomtype)
        {
        case ROOM_TYPE::ROOM_LEFT:
            m_coin_left += it.giftcoin;
            m_list_left.AddString(wstr.c_str());
            break;;
        case ROOM_TYPE::ROOM_RIGHT:
            m_coin_right += it.giftcoin;
            m_list_right.AddString(wstr.c_str());
            break;
        default:
            break;
        }
    }
    UpdateData(FALSE);
    return 0;
}

LRESULT CDlgGiftNotify::OnUpdateTotalCount(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void CDlgGiftNotify::Notify(const std::wstring& message)
{
    return;
}

void CDlgGiftNotify::Notify601(ROOM_TYPE roomtype, 
                               const RoomGiftInfo601& roomgiftinfo,
                               const GiftInfo& giftinfo)
{
    uint32 income = roomgiftinfo.gitfnumber * giftinfo.price;

    UserGiftAccumulative giftAccumulative;
    giftAccumulative.roomtype = roomtype;
    giftAccumulative.userid = roomgiftinfo.senderid;
    giftAccumulative.nickname = roomgiftinfo.sendername;
    giftAccumulative.giftcoin = income;
    giftAccumulative.accumulative = 0;//暂时不使用

    switch (roomtype)
    {
    case CDlgGiftNotify::ROOM_TYPE::ROOM_LEFT:
        m_coin_left += income;
        break;
    case CDlgGiftNotify::ROOM_TYPE::ROOM_RIGHT:
        m_coin_right += income;
        break;
    default:
        break;
    }
    
    messageLock_.lock();
    messageQueue_.push_back(giftAccumulative);
    messageLock_.unlock();
    SendMessage(WM_USER_ADD_GIFT_INFO, 0, 0);
}

void CDlgGiftNotify::ClearList()
{
    messageLock_.lock();
    messageQueue_.clear();
    messageLock_.unlock();

    m_coin_left = 0;
    m_coin_right = 0;
    int leftcount = m_list_left.GetCount();
    for (int i = leftcount; i > 0; --leftcount)
    {
        m_list_left.DeleteString(i);
    }
    int rightcount = m_list_left.GetCount();
    for (int i = rightcount; i > 0; --rightcount)
    {
        m_list_right.DeleteString(i);
    }
    UpdateData(FALSE);
}
