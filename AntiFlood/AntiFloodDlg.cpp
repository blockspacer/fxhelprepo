
// FanXingDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include <xutility>
#include "AntiFlood.h"
#include "AntiFloodDlg.h"
#include "NormalWelcomeSettingDlg.h"
#include "WelcomeSettingDlg.h"
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "BlacklistHelper.h"
#include "Config.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    const wchar_t* viewcolumnlist[] = {
        L"�ǳ�",
        L"�Ƹ��ȼ�",
        L"�û�id",
        L"����ʱ��",
        L"�����",
        L"�������"
    };

    const wchar_t* blackcolumnlist[] = {
        L"�ǳ�",
        L"�û�id"
    };

    const wchar_t* vestcolumnlist[] = {
        L"���",
        L"���",
        L"��������",
        L"������"
    };

    const wchar_t* userstrategylist[] = {
        L"�û�id", 
        L"�ǳ�",
        L"��ӭ����"
    };
}

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg();

// �Ի�������
    enum { IDD = IDD_ABOUTBOX };

    protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CAntiFloodDlg �Ի���

#define NOPRIVILEGE_NOTICE L"��û�в���Ȩ��"

CAntiFloodDlg::CAntiFloodDlg(CWnd* pParent /*=NULL*/)
    : CDialogEx(CAntiFloodDlg::IDD, pParent)
    , network_(nullptr)
    , blacklistHelper_(nullptr)
    , infoListCount_(0)
    , listCtrlRowIndex_(0)
    , m_radiogroup(0)
{
    blacklistHelper_.reset(new BlacklistHelper);
    antiStrategy_.reset(new AntiStrategy);
    giftStrategy_.reset(new GiftStrategy);
    enterRoomStrategy_.reset(new EnterRoomStrategy);

    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); 
}

CAntiFloodDlg::~CAntiFloodDlg()
{
    if (network_)
    {
        network_->RemoveNotify();
        network_->RemoveNotify201();
        network_->RemoveNotify501();
        network_->Finalize();
    }  
}

void CAntiFloodDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, InfoList_);
    DDX_Control(pDX, IDC_LIST_USER_STATUS, m_ListCtrl_Viewers);
    DDX_Control(pDX, IDC_CHECK_REMEMBER, m_check_remember);
    DDX_Control(pDX, IDC_LIST_USER_STATUS_BLACK, m_ListCtrl_Blacks);
    DDX_Control(pDX, IDC_STATIC_AUTH_INFO, m_static_auth_info);
    DDX_Control(pDX, IDC_STATIC_LOGIN_INFO, m_static_login_info);
    DDX_Control(pDX, IDC_EDIT_VEST, m_edit_vest);
    DDX_Control(pDX, IDC_EDIT_CHAT_MSG, m_edit_chatmsg);
    DDX_Control(pDX, IDC_LIST_VEST, m_list_vest);
    DDX_Radio(pDX, IDC_RADIO_NOACTION, m_radiogroup);
    DDX_Control(pDX, IDC_CHK_HANDLE_ALL, m_chk_handle_all);
    DDX_Control(pDX, IDC_EDIT_VERIFYCODE, m_edit_verifycode);
    DDX_Control(pDX, IDC_STATIC_VERIFYCODE, m_static_verifycode);
    DDX_Control(pDX, IDC_CHK_ROBOT, m_chk_robot);
    DDX_Control(pDX, IDC_EDIT_API_KEY, m_edit_api_key);
    DDX_Control(pDX, IDC_CHK_WELCOME, m_chk_welcome);
    DDX_Control(pDX, IDC_CHK_THANKS, m_chk_thanks);
    DDX_Control(pDX, IDC_LIST_USER_STRATEGE, m_list_user_strategy);
    DDX_Control(pDX, IDC_CHK_REPEAT_CHAT, m_chk_repeat_chat);
    DDX_Control(pDX, IDC_COMBO_SECONDS, m_combo_seconds);
    DDX_Control(pDX, IDC_EDIT_AUTO_CHAT, m_edit_auto_chat);
    DDX_Control(pDX, IDC_COMBO_THANKS, m_combo_thanks);
    DDX_Control(pDX, IDC_COMBO_WELCOME, m_combo_welcome);
    DDX_Control(pDX, IDC_EDIT_SENSITIVE, m_edit_sensitive);
    DDX_Control(pDX, IDC_COMBO_HANDLE_LEVEL, m_combo_handle_level);
    DDX_Control(pDX, IDC_BTN_SENSITIVE, m_btn_add_sensitive);
    DDX_Control(pDX, IDC_BTN_ADD_VEST, m_btn_add_vest);
    DDX_Control(pDX, IDC_BTN_REMOVE_VEST, m_btn_remove_vest_sensitive);
    DDX_Control(pDX, IDC_CHK_CHECK_VIP_V, m_chk_vip_v);
    DDX_Control(pDX, IDC_CHK_PRIVATE_NOTIFY, m_chk_private_notify);
    DDX_Control(pDX, IDC_BTN_WELCOME_SETTING, m_btn_welcome_setting);
    DDX_Control(pDX, IDC_BTN_THANKS_SETTING, m_btn_thanks_setting);
    DDX_Control(pDX, IDC_EDIT_RETRIVE_GIFT_COIN, m_edit_retrive_gift_coin);
    DDX_Control(pDX, IDC_EDIT_ONCE_MESSAGE, m_edit_once_message);
    DDX_Control(pDX, IDC_EDIT_RECEIVEID, m_edit_receiveid);
    DDX_Control(pDX, IDC_EDIT_COOKIE, m_edit_cookie);
    DDX_Control(pDX, IDC_CHK_USE_COOKIE, m_chk_use_cookie);
    DDX_Control(pDX, IDC_EDIT_GIFT_LEVET, m_edit_gift_level);
}

BEGIN_MESSAGE_MAP(CAntiFloodDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CAntiFloodDlg::OnBnClickedButtonLogin)
    ON_BN_CLICKED(IDC_BUTTON_REWARSTAR, &CAntiFloodDlg::OnBnClickedButtonRewarstar)
    ON_BN_CLICKED(IDC_BUTTON_REWARDGIFT, &CAntiFloodDlg::OnBnClickedButtonRewardgift)
    ON_BN_CLICKED(IDC_BUTTON_NAV, &CAntiFloodDlg::OnBnClickedButtonEnterRoom)
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BTN_GETMSG, &CAntiFloodDlg::OnBnClickedBtnGetmsg)
    ON_BN_CLICKED(IDC_BTN_ADD, &CAntiFloodDlg::OnBnClickedBtnAdd)
    ON_MESSAGE(WM_USER_01, &CAntiFloodDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_ADD_ENTER_ROOM_INFO, &CAntiFloodDlg::OnDisplayDataToViewerList)
    ON_MESSAGE(WM_USER_ADD_TO_BLACK_LIST, &CAntiFloodDlg::OnDisplayDtatToBlackList)
    ON_MESSAGE(WM_USER_ADD_RETRIVE_GIFT_COIN, &CAntiFloodDlg::OnRetriveGiftCoin)
    ON_NOTIFY(HDN_ITEMCLICK, 0, &CAntiFloodDlg::OnHdnItemclickListUserStatus)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CAntiFloodDlg::OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL, &CAntiFloodDlg::OnBnClickedBtnSelectAll)
    ON_BN_CLICKED(IDC_BTN_SELECT_REVERSE, &CAntiFloodDlg::OnBnClickedBtnSelectReverse)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_MONTH, &CAntiFloodDlg::OnBnClickedBtnKickoutMonth)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_HOUR, &CAntiFloodDlg::OnBnClickedBtnKickoutHour)
    ON_BN_CLICKED(IDC_BTN_SILENT, &CAntiFloodDlg::OnBnClickedBtnSilent)
    ON_BN_CLICKED(IDC_BTN_UNSILENT, &CAntiFloodDlg::OnBnClickedBtnUnsilent)
    ON_BN_CLICKED(IDC_BTN_CLEAR, &CAntiFloodDlg::OnBnClickedBtnClear)
    ON_BN_CLICKED(IDC_BTN_GET_VIEWER_LIST, &CAntiFloodDlg::OnBnClickedBtnGetViewerList)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_MONTH_BLACK, &CAntiFloodDlg::OnBnClickedBtnKickoutMonthBlack)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_HOUR_BLACK, &CAntiFloodDlg::OnBnClickedBtnKickoutHourBlack)
    ON_BN_CLICKED(IDC_BTN_SILENT_BLACK, &CAntiFloodDlg::OnBnClickedBtnSilentBlack)
    ON_BN_CLICKED(IDC_BTN_UNSILENT_BLACK, &CAntiFloodDlg::OnBnClickedBtnUnsilentBlack)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL_BLACK, &CAntiFloodDlg::OnBnClickedBtnSelectAllBlack)
    ON_BN_CLICKED(IDC_BTN_SELECT_REVERSE_BLACK, &CAntiFloodDlg::OnBnClickedBtnSelectReverseBlack)
    ON_BN_CLICKED(IDC_BTN_REMOVE_BLACK, &CAntiFloodDlg::OnBnClickedBtnRemoveBlack)
    ON_BN_CLICKED(IDC_BTN_LOAD_BLACK, &CAntiFloodDlg::OnBnClickedBtnLoadBlack)
    ON_BN_CLICKED(IDC_BTN_ADD_TO_BLACK, &CAntiFloodDlg::OnBnClickedBtnAddToBlack)
    ON_BN_CLICKED(IDC_BTN_SAVE_BLACK, &CAntiFloodDlg::OnBnClickedBtnSaveBlack)
    ON_BN_CLICKED(IDC_BTN_CLEAR_INFO, &CAntiFloodDlg::OnBnClickedBtnClearInfo)
    ON_BN_CLICKED(IDCANCEL, &CAntiFloodDlg::OnBnClickedCancel)
    ON_BN_CLICKED(IDC_BTN_ADD_VEST, &CAntiFloodDlg::OnBnClickedBtnAddVest)
    ON_BN_CLICKED(IDC_BTN_REMOVE_VEST, &CAntiFloodDlg::OnBnClickedBtnRemoveVest)
    ON_BN_CLICKED(IDC_BTN_SEND_CHAT, &CAntiFloodDlg::OnBnClickedBtnSendChat)
    ON_BN_CLICKED(IDC_RADIO_NOACTION, &CAntiFloodDlg::OnBnClickedRadioNoaction)
    ON_BN_CLICKED(IDC_RADIO_BANCHAT, &CAntiFloodDlg::OnBnClickedRadioNoaction)
    ON_BN_CLICKED(IDC_RADIO_KICKOUT, &CAntiFloodDlg::OnBnClickedRadioNoaction)
    ON_BN_CLICKED(IDC_CHK_HANDLE_ALL, &CAntiFloodDlg::OnBnClickedChkHandleAll)
    ON_BN_CLICKED(IDC_CHK_ROBOT, &CAntiFloodDlg::OnBnClickedChkRobot)
    ON_BN_CLICKED(IDC_CHK_THANKS, &CAntiFloodDlg::OnBnClickedChkThanks)
    ON_BN_CLICKED(IDC_CHK_WELCOME, &CAntiFloodDlg::OnBnClickedChkWelcome)
    ON_BN_CLICKED(IDC_CHK_REPEAT_CHAT, &CAntiFloodDlg::OnBnClickedChkRepeatChat)
    ON_BN_CLICKED(IDC_BTN_ADD_WELCOME, &CAntiFloodDlg::OnBnClickedBtnAddWelcome)
    ON_BN_CLICKED(IDC_BTN_REMOVE_WELCOME, &CAntiFloodDlg::OnBnClickedBtnRemoveWelcome)
    ON_BN_CLICKED(IDC_BTN_SENSITIVE, &CAntiFloodDlg::OnBnClickedBtnSensitive)
    ON_BN_CLICKED(IDC_CHK_CHECK_VIP_V, &CAntiFloodDlg::OnBnClickedChkCheckVipV)
    ON_BN_CLICKED(IDC_CHK_PRIVATE_NOTIFY, &CAntiFloodDlg::OnBnClickedChkPrivateNotify)
    ON_BN_CLICKED(IDC_BTN_THANKS_SETTING, &CAntiFloodDlg::OnBnClickedBtnThanksSetting)
    ON_BN_CLICKED(IDC_BTN_WELCOME_SETTING, &CAntiFloodDlg::OnBnClickedBtnWelcomeSetting)
    ON_BN_CLICKED(IDC_BTN_PHONE_CITY_RANK, &CAntiFloodDlg::OnBnClickedBtnPhoneCityRank)
    ON_BN_CLICKED(IDC_BTN_RECEIVEID, &CAntiFloodDlg::OnBnClickedBtnReceiveid)
    ON_BN_CLICKED(IDC_BTN_SET_GIFT_DISPLAY_LEVEL, &CAntiFloodDlg::OnBnClickedBtnSetGiftDisplayLevel)
END_MESSAGE_MAP()


// CAntiFloodDlg ��Ϣ�������

BOOL CAntiFloodDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // ��������...���˵�����ӵ�ϵͳ�˵��С�

    // IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
    ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
    ASSERT(IDM_ABOUTBOX < 0xF000);

    CMenu* pSysMenu = GetSystemMenu(FALSE);
    if (pSysMenu != NULL)
    {
        BOOL bNameValid;
        CString strAboutMenu;
        bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
        ASSERT(bNameValid);
        if (!strAboutMenu.IsEmpty())
        {
            pSysMenu->AppendMenu(MF_SEPARATOR);
            pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
        }
    }

    // ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
    //  ִ�д˲���

    SetWindowText(L"���������6.0_֧��pk��");

    SetIcon(m_hIcon, TRUE);            // ���ô�ͼ��
    SetIcon(m_hIcon, FALSE);        // ����Сͼ��
    SetDlgItemText(IDC_EDIT_NAV, L"0");
    SetDlgItemInt(IDC_EDIT_X, 0);
    SetDlgItemInt(IDC_EDIT_Y, 0);

    DWORD dwStyle = m_ListCtrl_Viewers.GetExtendedStyle();
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
    dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��

    m_ListCtrl_Viewers.SetExtendedStyle(dwStyle); //������չ���
    int nColumnCount = m_ListCtrl_Viewers.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_Viewers.DeleteColumn(i);
    uint32 index = 0;
    for (const auto& it : viewcolumnlist)
        m_ListCtrl_Viewers.InsertColumn(index++, it, LVCFMT_LEFT, 80);//������

    m_ListCtrl_Blacks.SetExtendedStyle(dwStyle);
    nColumnCount = m_ListCtrl_Blacks.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_Blacks.DeleteColumn(i);
    index = 0;
    for (const auto& it : blackcolumnlist)
        m_ListCtrl_Blacks.InsertColumn(index++, it, LVCFMT_LEFT, 80);//������

    // ��ʼ����������
    Config config;
    std::wstring w_cookie;
    if (config.GetCookie(&w_cookie))
    {
        SetDlgItemText(IDC_EDIT_COOKIE, w_cookie.c_str());
    }
    
    bool remember = config.GetRemember();
    if (remember)
    {
        m_check_remember.SetCheck(remember);
        std::wstring username,password;
        config.GetUserName(&username);
        config.GetPassword(&password);
        SetDlgItemText(IDC_EDIT_Username, username.c_str());
        SetDlgItemText(IDC_EDIT_Password, password.c_str());
    }

    // ��ȡ��Ȩ��Ϣ��ʾ�ڽ�����
    AuthorityHelper authorityHelper;
    std::wstring authorityDisplayInfo = L"���δ��Ȩ,��������";
    authorityHelper.GetAuthorityDisplayInfo(&authorityDisplayInfo);
    m_static_auth_info.SetWindowTextW(authorityDisplayInfo.c_str());

    // ��ʼ�������Ϣ��ͷ
    m_list_vest.SetExtendedStyle(dwStyle);
    index = 0;
    for (const auto& it : vestcolumnlist)
        m_list_vest.InsertColumn(index++, it, LVCFMT_LEFT, 80);//������   
    m_radiogroup = 1;
    std::vector<RowData> rowdatas;
    antiStrategy_->LoadAntiSetting(&rowdatas);
    int itemcount = m_list_vest.GetItemCount();
    for (const auto& rowdata : rowdatas)
    {
        int nitem = m_list_vest.InsertItem(itemcount + 1, L"���д�");
        int index = 0;
        for (const auto& column : rowdata)
        {
            m_list_vest.SetItemText(nitem, index++, column.c_str());
        }
    }

    m_combo_handle_level.AddString(L"1");
    m_combo_handle_level.AddString(L"3");
    m_combo_handle_level.AddString(L"5");
    m_combo_handle_level.AddString(L"8");
    m_combo_handle_level.SelectString(0, L"3");

    m_list_user_strategy.SetExtendedStyle(dwStyle);
    index = 0;
    for (const auto& it : userstrategylist)
        m_list_user_strategy.InsertColumn(index++, it, LVCFMT_LEFT, 100);//������

    std::map<uint32, WelcomeInfo> welcome_info_map;
    enterRoomStrategy_->GetSpecialWelcomeContent(&welcome_info_map);
    int welcomecount = 0;
    for (auto& it : welcome_info_map)
    {
        std::wstring ws_fanxingid = base::UTF8ToWide(base::UintToString(it.first));
        int nitem = m_list_user_strategy.InsertItem(welcomecount, ws_fanxingid.c_str());
        m_list_user_strategy.SetItemText(nitem, 0, ws_fanxingid.c_str());
        m_list_user_strategy.SetItemText(nitem, 1, it.second.name.c_str()); // nameֻ���û���ʾʹ�ã��߼���ʹ��
        m_list_user_strategy.SetItemText(nitem, 2, it.second.content.c_str());
        welcomecount++;
    }

    // ����������ʾ��ʼ��
    m_edit_retrive_gift_coin.SetWindowTextW(L"0");

    std::wstring roomid;
    config.GetRoomid(&roomid);
    SetDlgItemText(IDC_EDIT_NAV, roomid.c_str());

    std::wstring apikey;
    bool enable_robot = false;
    config.GetRobot(&enable_robot, &apikey);
    m_chk_robot.SetCheck(enable_robot);
    m_edit_api_key.SetWindowTextW(apikey.c_str());
    m_edit_api_key.EnableWindow(!enable_robot);

    // ��ȡͨ�û�ӭ��
    m_combo_seconds.AddString(L"60");
    m_combo_seconds.AddString(L"120");
    m_combo_seconds.AddString(L"180");
    m_combo_seconds.SelectString(0, L"60");
    bool enable_repeat = false;
    std::wstring content;
    std::wstring seconds;
    config.GetRepeatChat(&enable_repeat, &content, &seconds);
    m_chk_repeat_chat.SetCheck(enable_repeat);
    m_edit_auto_chat.SetWindowTextW(content.c_str());
    m_combo_seconds.SetWindowTextW(seconds.c_str());
    m_edit_auto_chat.EnableWindow(!enable_repeat);
    m_combo_seconds.EnableWindow(!enable_repeat);
   
    m_combo_thanks.AddString(L"5");
    m_combo_thanks.AddString(L"100");
    m_combo_thanks.AddString(L"500");
    m_combo_thanks.AddString(L"1000");
    m_combo_thanks.SelectString(0, L"5");
    bool enable_gift_thanks = false;
    uint32 gift_value = 0;
    config.GetGiftThanks(&enable_gift_thanks, &gift_value);
    m_chk_thanks.SetCheck(enable_gift_thanks);
    m_combo_thanks.SetWindowTextW(base::UintToString16(gift_value).c_str());
    m_combo_thanks.EnableWindow(!enable_gift_thanks);

    m_combo_welcome.AddString(L"3");
    m_combo_welcome.AddString(L"5");
    m_combo_welcome.AddString(L"8");
    m_combo_welcome.AddString(L"11");
    m_combo_welcome.SelectString(0, L"3");
    bool enable_welcome = false;
    uint32 level = 0;
    config.GetEnterRoomWelcome(&enable_welcome, &level);
    m_chk_welcome.SetCheck(enable_welcome);
    m_combo_welcome.SetWindowTextW(base::UintToString16(level).c_str());
    m_combo_welcome.EnableWindow(!enable_welcome);

    return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CAntiFloodDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
    if ((nID & 0xFFF0) == IDM_ABOUTBOX)
    {
        CAboutDlg dlgAbout;
        dlgAbout.DoModal();
    }
    else
    {
        CDialogEx::OnSysCommand(nID, lParam);
    }
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CAntiFloodDlg::OnPaint()
{
    if (!image.IsNull())
    {
        int hight = image.GetHeight();
        int width = image.GetWidth();
        CRect rc;
        m_static_verifycode.GetWindowRect(&rc);
        ScreenToClient(rc);
        image.Draw(GetDC()->m_hDC, CRect(rc.left, rc.top, rc.left + width,
            rc.top + hight));
        m_edit_verifycode.SetFocus();
        //CDialogEx::OnPaint();
    }

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

    CDialogEx::OnPaint();
}

void CAntiFloodDlg::OnClose()
{
    if (network_)
    {
        network_->RemoveNotify();
    }  
}

void CAntiFloodDlg::OnOK()
{
}

void CAntiFloodDlg::OnCancel()
{
}
//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CAntiFloodDlg::OnQueryDragIcon()
{
    return static_cast<HCURSOR>(m_hIcon);
}

// ��¼����
void CAntiFloodDlg::OnBnClickedButtonLogin()
{
    CString username;
    CString password;
    CString verifycode;
    GetDlgItemText(IDC_EDIT_Username, username);
    GetDlgItemText(IDC_EDIT_Password, password);
    m_edit_verifycode.GetWindowTextW(verifycode);
    bool remember = !!m_check_remember.GetCheck();

    CString cookie;
    GetDlgItemText(IDC_EDIT_COOKIE, cookie);
    bool use_cookie = !!m_chk_use_cookie.GetCheck();

    // ����ͨ����curl��¼��ʽ
    bool result = LoginByRequest(username.GetBuffer(), password.GetBuffer(), 
        verifycode.GetBuffer(), cookie.GetBuffer(), use_cookie);
    std::wstring message = std::wstring(L"��¼ ") + (result ? L"�ɹ�" : L"ʧ��");
    if (result)
    {
        std::wstring displayinfo;
        network_->GetCurrentUserDisplay(&displayinfo);
        m_static_login_info.SetWindowTextW(displayinfo.c_str());

        Config config;
        config.SaveUserInfo(username.GetBuffer(), password.GetBuffer(), remember);
        config.SaveCookie(cookie.GetBuffer());
    }
    else
    {

    }
    
    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, message);
}

//��תҳ�湦��
void CAntiFloodDlg::OnBnClickedButtonEnterRoom()
{
    if (!network_)
        return;
    
    // �����ԭ������
    m_ListCtrl_Viewers.DeleteAllItems();

    CString strRoomid;
    GetDlgItemText(IDC_EDIT_NAV, strRoomid);
    base::StringToUint(strRoomid.GetBuffer(), &roomid_);

    network_->SetNotify(
        std::bind(&CAntiFloodDlg::Notify, this, std::placeholders::_1, std::placeholders::_2));

    network_->SetNotify201(
        std::bind(&CAntiFloodDlg::NotifyEnterRoom, this, std::placeholders::_1));

    network_->SetNotify501(
        std::bind(&CAntiFloodDlg::NotifyEnterRoom, this, std::placeholders::_1));

    network_->SetRetriveGiftCoin(
        base::Bind(&CAntiFloodDlg::NotifyRetriveGiftCoin, base::Unretained(this)));

    bool result = network_->EnterRoom(strRoomid.GetBuffer());
    std::wstring message = std::wstring(L"���뷿�� ") + (result ? L"�ɹ�" : L"ʧ��");
    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, message);
    if (!result)
        return;

    Config config;
    config.SaveRoomId(strRoomid.GetBuffer());

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        return;
    }

    // Ϊ���������С��л��׼��
    std::string content;
    network_->GetGiftList(roomid_, &content);
    giftStrategy_->Initialize(content);

    // �����������
    UpdateRobotSetting();
    UpdateThanksSetting();
    UpdateWelcomeSetting();
    UpdateRepeatChatSetting();
}

// �����ǹ���
void CAntiFloodDlg::OnBnClickedButtonRewarstar()
{
    if (!network_)
        return;
}

// �����﹦��
void CAntiFloodDlg::OnBnClickedButtonRewardgift()
{
    if (!network_)
        return;
}

// ��ȡ������Ϣ
void CAntiFloodDlg::OnBnClickedBtnGetmsg()
{
    if (!network_)
        return;
}

// ���������Եĺ���
void CAntiFloodDlg::OnBnClickedBtnAdd()
{
    RowData rowdata;
    rowdata.push_back(L"1");
    rowdata.push_back(L"2");
    rowdata.push_back(L"3");
    rowdata.push_back(L"4");
    rowdata.push_back(L"5");
    rowdata.push_back(L"1");

    int nitem = m_ListCtrl_Viewers.InsertItem(0, rowdata[0].c_str());
    m_ListCtrl_Viewers.SetItemData(nitem, listCtrlRowIndex_++);
    for (uint32 j = 1; j < rowdata.size(); ++j)
    {
        m_ListCtrl_Viewers.SetItemText(nitem, j, rowdata[j].c_str());
    }
}
void CAntiFloodDlg::SetHScroll()
{
    CDC* dc = GetDC();
    
    CString str;
    int index = InfoList_.GetCount() - 1;
    if (index>=0)
    {
        InfoList_.GetText(index, str);
        SIZE s = dc->GetTextExtent(str);
        long temp = (long)SendDlgItemMessage(IDC_LIST1, LB_GETHORIZONTALEXTENT, 0, 0); //temp�õ��������Ŀ��
        if (s.cx > temp)
        {
            SendDlgItemMessage(IDC_LIST1, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
        }
    }

    ReleaseDC(dc);
}

void CAntiFloodDlg::Notify(MessageLevel level, const std::wstring& message)
{
    // �������ݸ�����
    if (message.empty())
        return;
    
    std::wstring* p_msg(new std::wstring(message));
    uint32 wParam = static_cast<uint32>(level);
    this->PostMessage(WM_USER_01, wParam, (LPARAM)p_msg);
}

void CAntiFloodDlg::NotifyEnterRoom(const RowData& rowdata)
{
    // �������ݸ�����
    viewerRowdataMutex_.lock();
    viewerRowdataQueue_.push_back(rowdata);
    viewerRowdataMutex_.unlock();
    this->PostMessage(WM_USER_ADD_ENTER_ROOM_INFO, 0, 0);
}

// ��ʱ�����߳��л��ˣ�ֱ�Ӵ���
void CAntiFloodDlg::NotifyRetriveGiftCoin(uint32 coin)
{
    this->PostMessage(WM_USER_ADD_RETRIVE_GIFT_COIN, 0, coin);
}

bool CAntiFloodDlg::LoginByRequest(const std::wstring& username, 
    const std::wstring& password, const std::wstring& verifycode,
    const std::wstring& cookie, bool use_cookie)
{
    if (username_ != username)
    {
        if (network_)
        {
            network_->RemoveNotify();
            network_->Finalize();
        }
        network_.reset(new NetworkHelper);
        network_->Initialize();
        network_->SetAntiStrategy(antiStrategy_);
        network_->SetGiftStrategy(giftStrategy_);
        network_->SetEnterRoomStrategy(enterRoomStrategy_);
        network_->SetNotify(
            std::bind(&CAntiFloodDlg::Notify, this, std::placeholders::_1,
            std::placeholders::_2));
        username_ = username;
    }
  
    std::string errormsg;
    //bool result = network_->Login(username, password, verifycode, &errormsg);

    if (!use_cookie)
        return false;

    // ��ΪĿǰû�ƹ���֤�룬ֻ��ʹ��cookie��¼����ʽ
    std::string cookiess = base::WideToUTF8(cookie);
    bool result = network_->LoginWithCookies(cookiess, &errormsg);


    std::wstring message = username + L" ��¼";
    if (!result)
    {
        message += L"ʧ��," + base::UTF8ToWide(errormsg);
    }
    else
    {
        message += L"�ɹ�!";
    }
    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, message);

    if (base::UTF8ToWide(errormsg).find(L"��֤��")!=std::string::npos)
    {
        RefreshVerifyCode();
    }
    
    return result;
}

bool CAntiFloodDlg::RefreshVerifyCode()
{
    std::vector<uint8> picture;
    if (!network_->LoginGetVerifyCode(&picture))
    {
        return false;
    }

    // ��ʾ��֤��
    if (!image.IsNull())
    {
        image.Destroy();
    }
    COleStreamFile osf;
    osf.CreateMemoryStream(NULL);
    osf.Write(picture.data(), picture.size());
    osf.SeekToBegin();
    image.Load(osf.GetStream());
    int hight = image.GetHeight();
    int width = image.GetWidth();
    CRect rc;
    m_static_verifycode.GetWindowRect(&rc);
    ScreenToClient(rc);
    image.Draw(GetDC()->m_hDC, CRect(rc.left, rc.top, rc.left + width,
        rc.top + hight));
    m_edit_verifycode.SetWindowTextW(L"");
    return true;
}

bool CAntiFloodDlg::GetSelectViewers(std::vector<EnterRoomUserInfo>* enterRoomUserInfos)
{
    if (!enterRoomUserInfos)
        return false;

    int count = m_ListCtrl_Viewers.GetItemCount();
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_Viewers.GetCheck(i))
        {
            // �����߳��������������
            EnterRoomUserInfo enterRoomUserInfo;
            uint32 roomid = 0;
            base::StringToUint(m_ListCtrl_Viewers.GetItemText(i, 4).GetBuffer(), &roomid);
            enterRoomUserInfo.roomid = roomid_;
            uint32 richlevel = 0;
            base::StringToUint(m_ListCtrl_Viewers.GetItemText(i, 1).GetBuffer(), &richlevel);
            enterRoomUserInfo.richlevel = richlevel;
            enterRoomUserInfo.nickname = base::WideToUTF8(m_ListCtrl_Viewers.GetItemText(i, 0).GetBuffer());
            uint32 userid = 0;
            base::StringToUint(m_ListCtrl_Viewers.GetItemText(i, 2).GetBuffer(), &userid);
            enterRoomUserInfo.userid = userid;
            enterRoomUserInfos->push_back(enterRoomUserInfo);
        }
    }
    return true;
}

bool CAntiFloodDlg::GetSelectBlacks(std::vector<EnterRoomUserInfo>* enterRoomUserInfos)
{
    if (!enterRoomUserInfos)
        return false;

    int count = m_ListCtrl_Blacks.GetItemCount();
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_Blacks.GetCheck(i))
        {
            // �����߳��������������
            EnterRoomUserInfo enterRoomUserInfo;
            enterRoomUserInfo.roomid = roomid_;
            enterRoomUserInfo.nickname = base::WideToUTF8(m_ListCtrl_Blacks.GetItemText(i, 0).GetBuffer());
            uint32 userid = 0;
            base::StringToUint(m_ListCtrl_Blacks.GetItemText(i, 1).GetBuffer(), &userid);
            enterRoomUserInfo.userid = userid;
            enterRoomUserInfos->push_back(enterRoomUserInfo);
        }
    }

    return true;
}

bool CAntiFloodDlg::KickOut_(
    const std::vector<EnterRoomUserInfo>& enterRoomUserInfos,
    KICK_TYPE kicktype)
{
    if (!network_)
        return false;

    for (const auto& enterRoomUserInfo : enterRoomUserInfos)
    {
        std::wstring msg = base::UTF8ToWide(enterRoomUserInfo.nickname);
        if (!network_->KickoutUsers(kicktype,
            enterRoomUserInfo.roomid, enterRoomUserInfo))
        {
            msg += L"�߳�ʧ��!Ȩ�޲������������!";          
        }
        else
        {
            // ��Ҫɾ������Ϣ������־��¼�б���, id = 2 ���û�id                
            msg += L"���߳�";
        }
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg);
    }

    return true;
}

bool CAntiFloodDlg::BanChat_(const std::vector<EnterRoomUserInfo>& enterRoomUserInfos)
{
    if (!network_)
        return false;

    for (const auto& enterRoomUserInfo : enterRoomUserInfos)
    {
        std::wstring msg = base::UTF8ToWide(enterRoomUserInfo.nickname);
        if (!network_->BanChat(
            enterRoomUserInfo.roomid, enterRoomUserInfo))
        {
            msg += L"����ʧ��!Ȩ�޲������������!";
        }
        else
        {
            // ��Ҫɾ������Ϣ������־��¼�б���, id = 2 ���û�id                
            msg += L"�����������";
        }
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg);
    }
    return true;
}

bool CAntiFloodDlg::UnbanChat_(const std::vector<EnterRoomUserInfo>& enterRoomUserInfos)
{
    if (!network_)
        return false;

    for (const auto& enterRoomUserInfo : enterRoomUserInfos)
    {
        std::wstring msg = base::UTF8ToWide(enterRoomUserInfo.nickname);
        if (!network_->UnbanChat(
            enterRoomUserInfo.roomid, enterRoomUserInfo))
        {
            msg += L"�ָ�����ʧ��!Ȩ�޲������������!";
        }
        else
        {
            // ��Ҫɾ������Ϣ������־��¼�б���, id = 2 ���û�id                
            msg += L"���ָ�����";
        }
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg);
    }
    return true;
}

bool CAntiFloodDlg::SendChatMessage_(uint32 roomid, const std::wstring& message)
{
    if (!network_)
        return false;

    if (!roomid || message.empty())
        return false;

    std::string utf8message = base::WideToUTF8(message);
    return network_->SendChatMessage(roomid, utf8message);
}

// �����߳�ִ��
LRESULT CAntiFloodDlg::OnDisplayDataToViewerList(WPARAM wParam, LPARAM lParam)
{
    if (viewerRowdataQueue_.empty())
        return 0;
    
    std::vector<RowData> rowdatas;
    viewerRowdataMutex_.lock();
    rowdatas.swap(viewerRowdataQueue_);
    viewerRowdataMutex_.unlock();

    int itemcount = m_ListCtrl_Viewers.GetItemCount();

    for (uint32 i = 0; i < rowdatas.size(); ++i)
    {
        bool exist = false;
        // ����Ƿ������ͬ�û�id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_ListCtrl_Viewers.GetItemText(index, 2);
            if (rowdatas[i][2].compare(text.GetBuffer()) == 0) // ��ͬ�û�id
            {
                // ���½��������ʾ�����������ݶ�ȫ��Ĭ�ϸ��µ�
                CString itemText = m_ListCtrl_Viewers.GetItemText(index, 5);
                std::string temp = base::WideToUTF8(itemText.GetBuffer());
                uint32 entercount = 0;
                base::StringToUint(temp, &entercount);
                entercount++;
                CString strEnterCount = base::UintToString16(entercount).c_str();
                m_ListCtrl_Viewers.SetItemText(index, 5, strEnterCount);

                for (uint32 j = 0; j < rowdatas[i].size(); ++j)
                {
                    m_ListCtrl_Viewers.SetItemText(itemcount + i, j, rowdatas[i][j].c_str());
                }
                exist = true;
                break;
            }
        }

        if (!exist) // ��������ڣ���Ҫ����������
        {
            int nitem = m_ListCtrl_Viewers.InsertItem(itemcount + i, rowdatas[i][0].c_str());
            //m_ListCtrl_UserStatus.SetItemData(nitem, i);
            for (uint32 j = 0; j < rowdatas[i].size(); ++j)
            {
                m_ListCtrl_Viewers.SetItemText(nitem, j, rowdatas[i][j].c_str());
            }
            m_ListCtrl_Viewers.SetItemText(nitem, 5, L"1"); // ��һ�μ�¼����
        }
    }

    return 0;
}

LRESULT CAntiFloodDlg::OnDisplayDtatToBlackList(WPARAM wParam, LPARAM lParam)
{
    if (blackRowdataQueue_.empty())
        return 0;

    std::vector<RowData> rowdatas;
    blackRowdataMutex_.lock();
    rowdatas.swap(blackRowdataQueue_);
    blackRowdataMutex_.unlock();

    int itemcount = m_ListCtrl_Blacks.GetItemCount();

    for (uint32 i = 0; i < rowdatas.size(); ++i)
    {
        bool exist = false;
        // ����Ƿ������ͬ�û�id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_ListCtrl_Blacks.GetItemText(index, 1);
            if (rowdatas[i][1].compare(text.GetBuffer()) == 0) // ��ͬ�û�id
            {
                exist = true;
                break;
            }
        }

        if (!exist) // ��������ڣ���Ҫ����������
        {
            int nitem = m_ListCtrl_Blacks.InsertItem(itemcount + i, rowdatas[i][0].c_str());
            for (uint32 j = 0; j < rowdatas[i].size(); ++j)
            {
                m_ListCtrl_Blacks.SetItemText(nitem, j, rowdatas[i][j].c_str());
            }
        }
    }

    return 0;
}

LRESULT CAntiFloodDlg::OnRetriveGiftCoin(WPARAM wParam, LPARAM lParam)
{
    uint32 coin = (uint32)lParam;
    CString cstr_coin;
    m_edit_retrive_gift_coin.GetWindowTextW(cstr_coin);
    std::string str_coin = base::WideToUTF8(cstr_coin.GetBuffer());

    uint32 display_coin = 0;
    base::StringToUint(str_coin, &display_coin);
    display_coin += coin;

    std::wstring wstr_coin = base::UintToString16(display_coin);

    m_edit_retrive_gift_coin.SetWindowTextW(wstr_coin.c_str());
    return 0;
}

LRESULT CAntiFloodDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::wstring* messages = reinterpret_cast<std::wstring*>(lParam);
    MessageLevel level = static_cast<MessageLevel>(wParam);

    switch (level)
    {
    case MessageLevel::MESSAGE_LEVEL_DEBUG:
#ifdef _DEBUG
        InfoList_.InsertString(infoListCount_++, messages->c_str());

        InfoList_.SetCurSel(infoListCount_ - 1);
        SetHScroll();
#endif // _DEBUG
        break;
    case MessageLevel::MESSAGE_LEVEL_ONCE:
        m_edit_once_message.SetWindowTextW(messages->c_str());
        break;
    case MessageLevel::MESSAGE_LEVEL_DISPLAY:
        InfoList_.InsertString(infoListCount_++, messages->c_str());
        InfoList_.SetCurSel(infoListCount_ - 1);
        SetHScroll();
        break;
    default:
        break;
    }

    delete messages;
    return 0;
}

// �ȽϷ���
int CAntiFloodDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    CListCtrl* ctlList = reinterpret_cast <CListCtrl*> (lParamSort);

    LV_FINDINFO lvi;
    memset(&lvi, 0, sizeof(lvi));
    lvi.flags = LVFI_PARAM;

    lvi.lParam = lParam1;
    int nItem1(ctlList->FindItem(&lvi));

    lvi.lParam = lParam2;
    int nItem2(ctlList->FindItem(&lvi));

    CString s1(ctlList->GetItemText(nItem1, 0));

    CString s2(ctlList->GetItemText(nItem2, 0));

    int nReturn(s1.CompareNoCase(s2));

    return nReturn > 0 ? 0:1;
}

// ����
void CAntiFloodDlg::OnHdnItemclickListUserStatus(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    
    m_ListCtrl_Viewers.SortItems(CompareFunc, reinterpret_cast <DWORD> (this));

    *pResult = 0;}


void CAntiFloodDlg::OnBnClickedButtonRemove()
{
    int count = m_ListCtrl_Viewers.GetItemCount();

    // �Ӻ���ǰɾ��
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_Viewers.GetCheck(i))
        {
            // ��Ҫɾ������Ϣ������־��¼�б���
            CString itemtext = m_ListCtrl_Viewers.GetItemText(i, 2);
            itemtext + L"�����б���ɾ��";
            Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, itemtext.GetBuffer());

            // ɾ���Ѿ���ѡ�ļ�¼
            m_ListCtrl_Viewers.DeleteItem(i);
        }
    }
}

void CAntiFloodDlg::OnBnClickedBtnSelectAll()
{
    int count = m_ListCtrl_Viewers.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        m_ListCtrl_Viewers.SetCheck(i, 1);
    }
}

void CAntiFloodDlg::OnBnClickedBtnSelectReverse()
{
    int count = m_ListCtrl_Viewers.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_Viewers.GetCheck(i))
        {
            m_ListCtrl_Viewers.SetCheck(i, FALSE);
        }
        else
        {
            m_ListCtrl_Viewers.SetCheck(i, TRUE);
        }
    }
}


void CAntiFloodDlg::OnBnClickedBtnKickoutMonth()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    { 
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectViewers(&enterRoomUserInfos);
    KickOut_(enterRoomUserInfos, KICK_TYPE::KICK_TYPE_MONTH);
}


void CAntiFloodDlg::OnBnClickedBtnKickoutHour()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectViewers(&enterRoomUserInfos);
    KickOut_(enterRoomUserInfos, KICK_TYPE::KICK_TYPE_HOUR);
}


void CAntiFloodDlg::OnBnClickedBtnSilent()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectViewers(&enterRoomUserInfos);
    BanChat_(enterRoomUserInfos);
}

void CAntiFloodDlg::OnBnClickedBtnUnsilent()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectViewers(&enterRoomUserInfos);
    UnbanChat_(enterRoomUserInfos);
}

void CAntiFloodDlg::OnBnClickedBtnClear()
{
    int count = m_ListCtrl_Viewers.GetItemCount();
    // �Ӻ���ǰɾ��
    for (int i = count - 1; i >= 0; --i)
    {
        m_ListCtrl_Viewers.DeleteItem(i);
    }

    CString itemtext = L"����б�";
    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, itemtext.GetBuffer());
}


void CAntiFloodDlg::OnBnClickedBtnGetViewerList()
{
    if (!network_)
        return;

    std::vector<RowData> enterRoomUserInfoRowdata;
    if (!network_->GetViewerList(roomid_, &enterRoomUserInfoRowdata))
    {
        return;
    }

    viewerRowdataMutex_.lock();
    for (const auto& rowdata : enterRoomUserInfoRowdata)
    {
        viewerRowdataQueue_.push_back(rowdata);
    }   
    viewerRowdataMutex_.unlock();
    this->PostMessage(WM_USER_ADD_ENTER_ROOM_INFO, 0, 0);
}


void CAntiFloodDlg::OnBnClickedBtnKickoutMonthBlack()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectBlacks(&enterRoomUserInfos);
    KickOut_(enterRoomUserInfos, KICK_TYPE::KICK_TYPE_MONTH);
}


void CAntiFloodDlg::OnBnClickedBtnKickoutHourBlack()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectBlacks(&enterRoomUserInfos);
    KickOut_(enterRoomUserInfos, KICK_TYPE::KICK_TYPE_HOUR);
}


void CAntiFloodDlg::OnBnClickedBtnSilentBlack()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectBlacks(&enterRoomUserInfos);
    BanChat_(enterRoomUserInfos);
}

void CAntiFloodDlg::OnBnClickedBtnUnsilentBlack()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectBlacks(&enterRoomUserInfos);
    UnbanChat_(enterRoomUserInfos);
}

void CAntiFloodDlg::OnBnClickedBtnSelectAllBlack()
{
    int count = m_ListCtrl_Blacks.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        m_ListCtrl_Blacks.SetCheck(i, 1);
    }
}


void CAntiFloodDlg::OnBnClickedBtnSelectReverseBlack()
{
    int count = m_ListCtrl_Blacks.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_Blacks.GetCheck(i))
        {
            m_ListCtrl_Blacks.SetCheck(i, FALSE);
        }
        else
        {
            m_ListCtrl_Blacks.SetCheck(i, TRUE);
        }
    }
}

void CAntiFloodDlg::OnBnClickedBtnRemoveBlack()
{
    int count = m_ListCtrl_Blacks.GetItemCount();

    // �Ӻ���ǰɾ��
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_Blacks.GetCheck(i))
        {
            // ��Ҫɾ������Ϣ������־��¼�б���
            CString itemtext = m_ListCtrl_Blacks.GetItemText(i, 2);
            itemtext + L"���Ӻ������б���ɾ��";
            Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, itemtext.GetBuffer());

            // ɾ���Ѿ���ѡ�ļ�¼
            m_ListCtrl_Blacks.DeleteItem(i);
        }
    }
}

void CAntiFloodDlg::OnBnClickedBtnLoadBlack()
{
    std::vector<RowData> rowdatas;
    if (!blacklistHelper_->LoadBlackList(&rowdatas))
    {
        CString itemtext = L"��ȡ������ʧ��";
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, itemtext.GetBuffer());
        return;
    }
    
    blackRowdataMutex_.lock();
    for (const auto& rowdata : rowdatas)
    {
        assert(rowdata.size() == 2);
        blackRowdataQueue_.push_back(rowdata);
    }
    blackRowdataMutex_.unlock();

    this->PostMessage(WM_USER_ADD_TO_BLACK_LIST, 0, 0);
}


void CAntiFloodDlg::OnBnClickedBtnAddToBlack()
{
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectViewers(&enterRoomUserInfos);

    blackRowdataMutex_.lock();
    for (const auto& enterRoomUserInfo : enterRoomUserInfos)
    {
        RowData rowdata;
        rowdata.push_back(base::UTF8ToWide(enterRoomUserInfo.nickname));
        rowdata.push_back(base::UintToString16(enterRoomUserInfo.userid));      
        blackRowdataQueue_.push_back(rowdata);
    }
    blackRowdataMutex_.unlock();

    this->PostMessage(WM_USER_ADD_TO_BLACK_LIST, 0, 0);
}


void CAntiFloodDlg::OnBnClickedBtnSaveBlack()
{
    std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    GetSelectBlacks(&enterRoomUserInfos);

    std::vector<RowData> rowdatas;
    for (const auto& enterRoomUserInfo : enterRoomUserInfos)
    {
        RowData rowdata;
        rowdata.push_back(base::UTF8ToWide(enterRoomUserInfo.nickname));
        rowdata.push_back(base::UintToString16(enterRoomUserInfo.userid));
        rowdatas.push_back(rowdata);
    }

    blacklistHelper_->SaveBlackList(rowdatas);
}

void CAntiFloodDlg::OnBnClickedBtnClearInfo()
{
    while (infoListCount_)
    {
        InfoList_.DeleteString(infoListCount_--);
    }
    InfoList_.DeleteString(0);
}


void CAntiFloodDlg::OnBnClickedCancel()
{
    CDialogEx::OnCancel();
}

void CAntiFloodDlg::OnBnClickedBtnSensitive()
{
    CString sensitive;
    m_edit_sensitive.GetWindowTextW(sensitive);
    std::string utfsensitive = base::WideToUTF8(sensitive.GetBuffer());
    if (!antiStrategy_->AddSensitive(utfsensitive))
        return; // �Ѿ����ڣ�����Ҫ�������

    int itemcount = m_list_vest.GetItemCount();
    bool exist = false;
    // ����Ƿ������ͬ�û�id
    for (int index = 0; index < itemcount; index++)
    {
        CString text = m_list_vest.GetItemText(index, 2);
        if (sensitive.CompareNoCase(text.GetBuffer()) == 0)
        {
            exist = true;
            break;
        }
    }

    if (!exist) // ��������ڣ���Ҫ����������
    {
        int nitem = m_list_vest.InsertItem(itemcount + 1, L"���д�");
        m_list_vest.SetItemText(nitem, 2, sensitive);
        CString msg = sensitive + L"���дʱ����뵽�Զ������б���";
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg.GetBuffer());
    }
}

void CAntiFloodDlg::OnBnClickedBtnAddVest()
{
    CString vestname;
    m_edit_vest.GetWindowTextW(vestname);
    std::string utfvestname = base::WideToUTF8(vestname.GetBuffer());
    if (!antiStrategy_->AddNickname(utfvestname))
        return; // �Ѿ����ڣ�����Ҫ�������

    int itemcount = m_list_vest.GetItemCount();
    bool exist = false;
    // ����Ƿ������ͬ�û�id
    for (int index = 0; index < itemcount; index++)
    {
        CString text = m_list_vest.GetItemText(index, 1);
        if (vestname.CompareNoCase(text.GetBuffer()) == 0)
        {
            exist = true;
            break;
        }
    }

    if (!exist) // ��������ڣ���Ҫ����������
    {
        int nitem = m_list_vest.InsertItem(itemcount + 1, L"���");
        m_list_vest.SetItemText(nitem, 1, vestname);
        CString msg = vestname + L"��ױ����뵽�Զ������б���";
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg.GetBuffer());
    }
}


void CAntiFloodDlg::OnBnClickedBtnRemoveVest()
{
    int count = m_list_vest.GetItemCount();

    // �Ӻ���ǰɾ��
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_list_vest.GetCheck(i))
        {
            // ��Ҫɾ������Ϣ������־��¼�б���
            CString itemtext = m_list_vest.GetItemText(i, 1);
            std::string utfvestname = base::WideToUTF8(itemtext.GetBuffer());
            antiStrategy_->RemoveNickname(utfvestname);

            itemtext = m_list_vest.GetItemText(i, 2);
            std::string utfsensitive = base::WideToUTF8(itemtext.GetBuffer());
            antiStrategy_->RemoveSensitive(utfsensitive);

            // ɾ���Ѿ���ѡ�ļ�¼
            m_list_vest.DeleteItem(i);
            CString msg = itemtext + L"�����Զ������б���ɾ��";
            Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg.GetBuffer());
        }
    }
}


void CAntiFloodDlg::OnBnClickedBtnSendChat()
{
    CString message;
    m_edit_chatmsg.GetWindowTextW(message);
    SendChatMessage_(roomid_, message.GetBuffer());
}


void CAntiFloodDlg::OnBnClickedRadioNoaction()
{
    UpdateData(TRUE);
    CString cs_handle_level;
    m_combo_handle_level.GetWindowTextW(cs_handle_level);
    uint32 rich_level = 3;
    base::StringToUint(base::WideToUTF8(cs_handle_level.GetBuffer()), &rich_level);
    antiStrategy_->SetHandleRichLevel(rich_level);
    switch (m_radiogroup)
    {
    case 0:
        antiStrategy_->SetHandleType(HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE);
        break;
    case 1:
        antiStrategy_->SetHandleType(HANDLE_TYPE::HANDLE_TYPE_BANCHAT);
        break;
    case 2:
        antiStrategy_->SetHandleType(HANDLE_TYPE::HANDLE_TYPE_KICKOUT);
        break;
    default:
        antiStrategy_->SetHandleType(HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE);
        break;
    }

    m_combo_handle_level.EnableWindow(!m_radiogroup);
    m_btn_add_sensitive.EnableWindow(!m_radiogroup);
    m_btn_add_vest.EnableWindow(!m_radiogroup);
    m_btn_remove_vest_sensitive.EnableWindow(!m_radiogroup);
}


void CAntiFloodDlg::OnBnClickedChkHandleAll()
{
    bool handleall = !!m_chk_handle_all.GetCheck();
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        m_chk_handle_all.SetCheck(FALSE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }

    CString cs_handle_level;
    m_combo_handle_level.GetWindowTextW(cs_handle_level);
    uint32 rich_level = 3;
    base::StringToUint(base::WideToUTF8(cs_handle_level.GetBuffer()), &rich_level);
    antiStrategy_->SetHandleRichLevel(rich_level);
    network_->SetHandleChatUsers(handleall);
}


void CAntiFloodDlg::OnBnClickedChkRobot()
{
    UpdateRobotSetting();
}

void CAntiFloodDlg::UpdateRobotSetting()
{
    bool enablerobot = !!m_chk_robot.GetCheck();
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        m_chk_robot.SetCheck(FALSE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }

    CString apiKey;
    m_edit_api_key.GetWindowTextW(apiKey);
    if (apiKey.GetLength() != 32)
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, L"������Key����ȷ,�޷����û�����");
        m_chk_robot.SetCheck(FALSE);
        return;
    }

    m_edit_api_key.EnableWindow(!enablerobot);

    network_->SetRobotApiKey(apiKey.GetBuffer());
    network_->SetRobotHandle(enablerobot);
    Config config;
    config.SaveRobot(enablerobot, apiKey.GetBuffer());
}

void CAntiFloodDlg::OnBnClickedChkThanks()
{
    UpdateThanksSetting();
}

void CAntiFloodDlg::UpdateThanksSetting()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        m_chk_thanks.SetCheck(FALSE);
        m_combo_thanks.EnableWindow(TRUE);
        giftStrategy_->SetThanksFlag(false);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }

    bool enable = !!m_chk_thanks.GetCheck();
    m_combo_thanks.EnableWindow(!enable);
    giftStrategy_->SetThanksFlag(enable);
    m_btn_thanks_setting.EnableWindow(!enable);

    CString cs_gift_value;
    m_combo_thanks.GetWindowTextW(cs_gift_value);
    uint32 gift_value = 0;
    base::StringToUint(base::WideToUTF8(cs_gift_value.GetBuffer()), &gift_value);
    giftStrategy_->SetGiftValue(gift_value);

    Config config;
    config.SaveGiftThanks(enable, gift_value);
}

void CAntiFloodDlg::OnBnClickedChkWelcome()
{
    UpdateWelcomeSetting();
}

void CAntiFloodDlg::UpdateWelcomeSetting()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        m_chk_welcome.SetCheck(FALSE);
        m_combo_welcome.EnableWindow(TRUE);
        enterRoomStrategy_->SetWelcomeFlag(false);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }

    bool enable = !!m_chk_welcome.GetCheck();
    m_combo_welcome.EnableWindow(!enable);
    enterRoomStrategy_->SetWelcomeFlag(enable);
    m_btn_welcome_setting.EnableWindow(!enable);

    CString welcome_level;
    m_combo_welcome.GetWindowTextW(welcome_level);
    uint32 level = 0;
    base::StringToUint(base::WideToUTF8(welcome_level.GetBuffer()), &level);
    enterRoomStrategy_->SetWelcomeLevel(level);

    // ���Ǻ�Ư����ʵ�ַ���
    std::wstring normal_welcome;
    enterRoomStrategy_->GetNormalWelcomeContent(&normal_welcome);

    Config config;
    config.SaveEnterRoomWelcome(enable, level);   
}

void CAntiFloodDlg::OnBnClickedChkRepeatChat()
{
    UpdateRepeatChatSetting();
}

void CAntiFloodDlg::UpdateRepeatChatSetting()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        m_chk_repeat_chat.SetCheck(FALSE);
        m_combo_seconds.EnableWindow(TRUE);
        m_edit_auto_chat.EnableWindow(TRUE);

        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
        return;
    }

    bool enable = !!m_chk_repeat_chat.GetCheck();
    CString chatmsg;
    m_edit_auto_chat.GetWindowTextW(chatmsg);
    if (enable && (chatmsg.IsEmpty() || chatmsg.GetLength() >= 50))
    {
        m_chk_repeat_chat.SetCheck(FALSE);
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, L"�������ݳ��ȴ���,�벻Ҫ����50���ַ�");
        return;
    }

    m_combo_seconds.EnableWindow(!enable);
    m_edit_auto_chat.EnableWindow(!enable);

    CString seconds;
    m_combo_seconds.GetWindowTextW(seconds);
    network_->SetRoomRepeatChat(enable, seconds.GetBuffer(), chatmsg.GetBuffer());
    Config config;
    config.SaveRepeatChat(enable, chatmsg.GetBuffer(), seconds.GetBuffer());
}

void CAntiFloodDlg::OnBnClickedBtnPhoneCityRank()
{
    if (!network_)
        return;

    std::wstring privilegeMsg;
    if (!network_->GetActionPrivilege(&privilegeMsg))
    {
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
        return;
    }

    network_->GetCityRankInfos(roomid_);
}

void CAntiFloodDlg::OnBnClickedBtnAddWelcome()
{
    WelcomeSettingDlg dlg;
    INT_PTR nResponse = dlg.DoModal();

    if (nResponse == IDCANCEL)
        return;

    if (nResponse != IDOK)
        return;

    std::wstring fanxingid;
    std::wstring name;
    std::wstring content;
    if (!dlg.GetSettingInfo(&fanxingid, &name, &content))
        return;

    int itemcount = m_list_user_strategy.GetItemCount();

    bool exist = false;
    // ����Ƿ������ͬ�û�id
    for (int index = 0; index < itemcount; index++)
    {
        CString text = m_list_user_strategy.GetItemText(index, 0);
        if (fanxingid.compare(text.GetBuffer()) == 0) // ��ͬ�û�id
        {
            exist = true;
            m_list_user_strategy.SetItemText(index, 0, fanxingid.c_str());
            m_list_user_strategy.SetItemText(index, 1, name.c_str());
            m_list_user_strategy.SetItemText(index, 2, content.c_str());
            break;
        }
    }

    if (!exist) // ��������ڣ���Ҫ����������
    {
        int nitem = m_list_user_strategy.InsertItem(itemcount, fanxingid.c_str());
        m_list_user_strategy.SetItemText(nitem, 0, fanxingid.c_str());
        m_list_user_strategy.SetItemText(nitem, 1, name.c_str()); // nameֻ���û���ʾʹ�ã��߼���ʹ��
        m_list_user_strategy.SetItemText(nitem, 2, content.c_str());
    }

    // ���»�ӭ����
    itemcount = m_list_user_strategy.GetItemCount();
    std::map<uint32, WelcomeInfo> welcome_content;
    for (int index = 0; index < itemcount; index++)
    {
        CString cs_fanxingid = m_list_user_strategy.GetItemText(index, 0);
        CString cs_name = m_list_user_strategy.GetItemText(index, 1);
        CString cs_welcome = m_list_user_strategy.GetItemText(index, 2);
        uint32 fanxingid = 0;
        base::StringToUint(base::WideToUTF8(cs_fanxingid.GetBuffer()), &fanxingid);
        WelcomeInfo info = { fanxingid, cs_name.GetBuffer(), cs_welcome.GetBuffer() };
        welcome_content[fanxingid] = info;
    }

    enterRoomStrategy_->SetSpecialWelcomeContent(welcome_content);
}


void CAntiFloodDlg::OnBnClickedBtnRemoveWelcome()
{
    int itemcount = m_list_user_strategy.GetItemCount();
    std::map<uint32, WelcomeInfo> welcome_info_map;
    for (int index = itemcount-1; index >=0; index--)
    {
        if (m_list_user_strategy.GetCheck(index))
        {
            m_list_user_strategy.DeleteItem(index);
            continue;
        }
        CString cs_fanxingid = m_list_user_strategy.GetItemText(index, 0);
        CString cs_name = m_list_user_strategy.GetItemText(index, 1);
        CString cs_welcome = m_list_user_strategy.GetItemText(index, 2);
        uint32 fanxingid = 0;
        base::StringToUint(base::WideToUTF8(cs_fanxingid.GetBuffer()), &fanxingid);
        WelcomeInfo welcome_info = { fanxingid, cs_name.GetBuffer(), cs_welcome.GetBuffer() };
        welcome_info_map[fanxingid] = welcome_info;
    }
    enterRoomStrategy_->SetSpecialWelcomeContent(welcome_info_map);
}

void CAntiFloodDlg::OnBnClickedChkCheckVipV()
{
    bool enable = !!m_chk_vip_v.GetCheck();
    enterRoomStrategy_->SetWelcomeVipV(enable);
}

void CAntiFloodDlg::OnBnClickedChkPrivateNotify()
{
    bool enable = !!m_chk_private_notify.GetCheck();
    enterRoomStrategy_->SetPrivateNotify(enable);
}

void CAntiFloodDlg::OnBnClickedBtnThanksSetting()
{

}

void CAntiFloodDlg::OnBnClickedBtnWelcomeSetting()
{
    std::wstring welcome;
    enterRoomStrategy_->GetNormalWelcomeContent(&welcome);

    NormalWelcomeSettingDlg dlg(welcome);
    INT_PTR nResponse = dlg.DoModal();

    if (nResponse == IDCANCEL)
        return;

    if (nResponse != IDOK)
        return;

    std::wstring new_welcome = dlg.GetNormalWelcome();
    enterRoomStrategy_->SetNormalWelcomeContent(new_welcome);
}


void CAntiFloodDlg::OnBnClickedBtnReceiveid()
{
    CString receiveid;
    m_edit_receiveid.GetWindowTextW(receiveid);
    std::string utfsensitive = base::WideToUTF8(receiveid.GetBuffer());
    if (!antiStrategy_->AddReceiveId(utfsensitive))
        return; // �Ѿ����ڣ�����Ҫ�������

    int itemcount = m_list_vest.GetItemCount();
    bool exist = false;
    // ����Ƿ������ͬ�û�id
    for (int index = 0; index < itemcount; index++)
    {
        CString text = m_list_vest.GetItemText(index, 3);
        if (receiveid.CompareNoCase(text.GetBuffer()) == 0)
        {
            exist = true;
            break;
        }
    }

    if (!exist) // ��������ڣ���Ҫ����������
    {
        int nitem = m_list_vest.InsertItem(itemcount + 1, L"������");
        m_list_vest.SetItemText(nitem, 3, receiveid);
        CString msg = receiveid + L"�����߱����뵽�Զ������б���";
        Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, msg.GetBuffer());
    }
}


void CAntiFloodDlg::OnBnClickedBtnSetGiftDisplayLevel()
{
    CString gift_value;
    m_edit_gift_level.GetWindowTextW(gift_value);
    uint32 gift_level = 0;
    base::StringToUint(base::WideToUTF8(gift_value.GetBuffer()), &gift_level);
    network_->SetRoomGiftNotifyLevel(roomid_, gift_level);
}
