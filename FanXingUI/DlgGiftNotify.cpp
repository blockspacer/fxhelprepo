// DlgGiftNotify.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgGiftNotify.h"
#include "afxdialogex.h"
#include "GiftInfoHelper.h"
#include "../Network/EncodeHelper.h"
#include "third_party/chromium/base/basictypes.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

// CDlgGiftNotify 对话框

IMPLEMENT_DYNAMIC(CDlgGiftNotify, CDialogEx)

CDlgGiftNotify::CDlgGiftNotify(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgGiftNotify::IDD, pParent)
    , m_room_left(0)
    , m_room_right(0)
    , m_time_all(600)
    , m_time_left(0)
    , m_coin_left(0)
    , m_coin_right(0)
    , m_static_time(_T("00:00:00"))
    , display_(false)
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
    DDX_Text(pDX, IDC_STATIC_TIME, m_static_time);
    DDX_Control(pDX, IDC_BTN_BEGIN, m_btn_begin);
    DDX_Control(pDX, IDC_STATIC_TIME, m_static_now_time);
    DDX_Control(pDX, IDC_EDIT_LEFT_GIFT, m_edit_coin_left);
    DDX_Control(pDX, IDC_EDIT_RIGHT_GIFT, m_edit_coin_right);
    DDX_Control(pDX, IDC_EDIT_TIME_LEFT, m_edit_time_left);
    DDX_Control(pDX, IDC_EDIT_TIME_ALL, m_edit_time_all);
    DDX_Control(pDX, IDC_STATIC_NOTICE, m_static_notice);
}


BEGIN_MESSAGE_MAP(CDlgGiftNotify, CDialogEx)
    ON_WM_TIMER()
    ON_BN_CLICKED(IDC_BTN_BEGIN, &CDlgGiftNotify::OnBnClickedBtnBegin)
    ON_MESSAGE(WM_USER_ADD_GIFT_INFO, &CDlgGiftNotify::OnAddGiftInfo)
    ON_MESSAGE(WM_USER_UPDATE_TOTAL_COUNT, &CDlgGiftNotify::OnUpdateTotalCount)
END_MESSAGE_MAP()

BOOL CDlgGiftNotify::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    scoped_ptr<CFont> font16(new CFont);
    font16->CreateFont(16,                        // nHeight
        30,                         // nWidth
        0,                         // nEscapement
        0,                         // nOrientation
        FW_BOLD,                   // nWeight
        FALSE,                     // bItalic
        FALSE,                     // bUnderline
        0,                         // cStrikeOut
        DEFAULT_CHARSET,           // nCharSet
        OUT_CHARACTER_PRECIS,        // nOutPrecision
        CLIP_DEFAULT_PRECIS,       // nClipPrecision
        DEFAULT_QUALITY,           // nQuality
        DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
        TEXT("黑体"));             // lpszFacename

    scoped_ptr<CFont> font40(new CFont);
    font40->CreateFont(140,                        // nHeight
        400,                         // nWidth
        60,                         // nEscapement
        0,                         // nOrientation
        FW_BOLD,                   // nWeight
        FALSE,                     // bItalic
        FALSE,                     // bUnderline
        0,                         // cStrikeOut
        DEFAULT_CHARSET,           // nCharSet
        OUT_DEFAULT_PRECIS,        // nOutPrecision
        CLIP_DEFAULT_PRECIS,       // nClipPrecision
        DEFAULT_QUALITY,           // nQuality
        DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
        TEXT("黑体"));

    scoped_ptr<CFont> font60(new CFont);
    font60->CreateFont(60,                        // nHeight
        50,                         // nWidth
        0,                         // nEscapement
        0,                         // nOrientation
        FW_BOLD,                   // nWeight
        FALSE,                     // bItalic
        FALSE,                     // bUnderline
        0,                         // cStrikeOut
        DEFAULT_CHARSET,           // nCharSet
        OUT_DEFAULT_PRECIS,        // nOutPrecision
        CLIP_DEFAULT_PRECIS,       // nClipPrecision
        DEFAULT_QUALITY,           // nQuality
        DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
        TEXT("黑体"));
    m_btn_begin.SetFont(font16.get());;
    m_static_now_time.SetFont(font16.get());;
    m_edit_coin_left.SetFont(font40.get());;
    m_edit_coin_right.SetFont(font40.get());;
    m_edit_time_left.SetFont(font60.get());;
    m_edit_time_all.SetFont(font16.get());;
    m_static_notice.SetFont(font16.get());;

    m_static_notice.SetWindowTextW(L"说明：本工具处于测试阶段，目前暂不支持幸运类礼物以及女神酒店类礼物统计,如果房间人数过多，会出现无法进入房间的问题。后续新版本会完善功能。");

    SetTimer(TIME_SHOW, 1000, NULL);
    return TRUE;
}
// CDlgGiftNotify 消息处理程序
void CDlgGiftNotify::OnBnClickedBtnBegin()
{
    UpdateData(TRUE);  
    ClearList();
    m_btn_begin.EnableWindow(FALSE);
 
    // 获取房间礼物列表
    if (!networkLeft_->GetGiftList(m_room_left))
    {
        ::MessageBoxW(0, L"获取房间礼物种类失败", L"错误", 0);
        m_btn_begin.EnableWindow(TRUE);
        return;
    }
    if (!networkRight_->GetGiftList(m_room_right))
    {
        ::MessageBoxW(0, L"获取房间礼物种类失败", L"错误", 0);
        m_btn_begin.EnableWindow(TRUE);
        return;
    }

    // 分别进入房间
    networkLeft_->SetNotify(
        std::bind(&CDlgGiftNotify::Notify, this, std::placeholders::_1));
    networkLeft_->SetNotify601(
        std::bind(&CDlgGiftNotify::Notify601, this, ROOM_TYPE::ROOM_LEFT, 
        std::placeholders::_1, std::placeholders::_2));

    if (!networkLeft_->EnterRoom(m_room_left))
    {
        ::MessageBoxW(0, L"进入房间失败", L"错误", 0);
        assert(false && L"进入房间失败");
        m_btn_begin.EnableWindow(TRUE);
        return;
    }

    // 分别进入房间
    networkRight_->SetNotify(
        std::bind(&CDlgGiftNotify::Notify, this, std::placeholders::_1));
    networkRight_->SetNotify601(
        std::bind(&CDlgGiftNotify::Notify601, this, ROOM_TYPE::ROOM_RIGHT,
        std::placeholders::_1, std::placeholders::_2));

    if (!networkRight_->EnterRoom(m_room_right))
    {
        ::MessageBoxW(0, L"进入房间失败", L"错误", 0);
        assert(false && L"进入房间失败");
        m_btn_begin.EnableWindow(TRUE);
        return;
    }
    SetTimer(TIME_SKIP, 1000, NULL);
}

void CDlgGiftNotify::OnTimer(UINT_PTR nIDEvent)
{
    UpdateData(TRUE);
    std::wstring showtime;
    switch (nIDEvent)
    {
    case TIME_SHOW:// 显示时间
        showtime = base::UTF8ToWide(MakeFormatTimeString(base::Time::Now()));
        m_static_time = showtime.c_str();
        break;
    case TIME_SKIP:// 显示剩余时间
        m_time_left--;
        if (m_time_left<=0)
        {
            KillTimer(TIME_SKIP);
            StopAccumulative();
            m_btn_begin.EnableWindow(TRUE);
        }
        break;
    default:
        break;
    }
    UpdateData(FALSE);
    return;
}


LRESULT CDlgGiftNotify::OnAddGiftInfo(WPARAM wParam, LPARAM lParam)
{
    std::vector<UserGiftAccumulative> newmessage;
    messageLock_.lock();
    newmessage.swap(messageQueue_);
    messageLock_.unlock();

    for (const auto&it : newmessage)
    {
        std::wstring wstr = base::UTF8ToWide(it.displaymsg);
        switch (it.roomtype)
        {
        case ROOM_TYPE::ROOM_LEFT:
            m_coin_left += it.giftcoin;
            m_list_left.InsertString(m_list_left.GetCount(), wstr.c_str());
            break;;
        case ROOM_TYPE::ROOM_RIGHT:
            m_coin_right += it.giftcoin;
            m_list_right.InsertString(m_list_right.GetCount(),wstr.c_str());
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
    giftAccumulative.displaymsg = std::string("[") +
        base::UintToString(income) + std::string("] ");
    if (!roomgiftinfo.tips.empty())
    {
        giftAccumulative.displaymsg += roomgiftinfo.tips;
    }
    else
    {
        giftAccumulative.displaymsg += roomgiftinfo.sendername + 
            base::WideToUTF8(L"送给艺人") +
            base::UintToString(roomgiftinfo.gitfnumber) + 
            base::WideToUTF8(L"个") + roomgiftinfo.giftname;
    }
    
    giftAccumulative.accumulative = 0;//暂时不使用
    messageLock_.lock();
    messageQueue_.push_back(giftAccumulative);
    messageLock_.unlock();
    SendMessage(WM_USER_ADD_GIFT_INFO, 0, 0);

    if (!display_)
        return;

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
    

}

void CDlgGiftNotify::ClearList()
{
    messageLock_.lock();
    messageQueue_.clear();
    messageLock_.unlock();

    display_ = true;
    m_time_left = m_time_all;
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

void CDlgGiftNotify::StopAccumulative()
{
    display_ = false;
}