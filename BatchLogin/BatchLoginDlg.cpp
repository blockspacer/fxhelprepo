
// BatchLoginDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include <string>
#include <map>
#include "BatchLogin.h"
#include "BatchLoginDlg.h"
#include "UserRoomManager.h"
#include "Network/WebsocketClientController.h"
#include "Network/TcpClient.h"
#include "BlacklistHelper.h"
#include "afxdialogex.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace{
    const wchar_t* usercolumnlist[] = {
        L"�û���",
        L"����",
        L"Cookie",
        L"�û�id",
        L"�ǳ�",
        L"�ȼ�",
        L"�Ǳ�",
        L"����",
        L"��Ʊ",
        L"����Ʊ"
    };

    const wchar_t* roomcolumnlist[] = {
        L"�����",
        L"Э������",
        L"��ʾ����",
        L"��Ч��",
    };

    const wchar_t* proxycolumnlist[] = {
        L"����Э��",
        L"����ip",
        L"����˿�"
    };

    const wchar_t* blackcolumnlist[] = {
        L"�ǳ�",
        L"�û�id"
    };
}

// CBatchLoginDlg �Ի���

CBatchLoginDlg::CBatchLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBatchLoginDlg::IDD, pParent)
    , userRoomManager_(nullptr)
{
    tcpManager_.reset(new WebsocketClientController());
    userRoomManager_.reset(new UserRoomManager(tcpManager_.get()));
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBatchLoginDlg::~CBatchLoginDlg()
{
    tcpManager_->Finalize();
    userRoomManager_->Finalize();
}

void CBatchLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_USERS, m_ListCtrl_Users);
    //DDX_Control(pDX, IDC_LIST_ROOM, m_ListCtrl_Rooms);
    DDX_Control(pDX, IDC_LIST_INFO, InfoList_);
    DDX_Control(pDX, IDC_LIST_PROXY, m_list_proxy);
    DDX_Control(pDX, IDC_EDIT_ROOMID, m_roomid);
    //DDX_Control(pDX, IDC_EDIT_GIFT_COUNT, m_gift_count);
    DDX_Control(pDX, IDC_EDIT_NICKNAME_PRE, m_nickname);
    DDX_Control(pDX, IDC_EDIT_PIC_PATH, m_logo_path);
    DDX_Control(pDX, IDC_EDIT_SONG_NAME, m_edit_singlike);
    DDX_Control(pDX, IDC_CHK_USE_COOKIE, m_chk_use_cookie);
    DDX_Control(pDX, IDC_EDIT_DELTA, m_edit_delta);
    DDX_Control(pDX, IDC_EDIT_CHAT_MESSAGE, m_edit_chat_message);
    DDX_Control(pDX, IDC_EDIT_MV_ID, m_mv_id);
    DDX_Control(pDX, IDC_LIST_GUEST, m_ListCtrl_Blacks);
}

BEGIN_MESSAGE_MAP(CBatchLoginDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_IMPORT_USER, &CBatchLoginDlg::OnBnClickedBtnImportUser)
    ON_BN_CLICKED(IDC_BTN_GET_PROXY, &CBatchLoginDlg::OnBnClickedBtnGetProxy)
    ON_BN_CLICKED(IDC_BTN_BATCH_ENTER_ROOM, &CBatchLoginDlg::OnBnClickedBtnBatchEnterRoom)
    //ON_BN_CLICKED(IDC_BTN_IMPORT_ROOM, &CBatchLoginDlg::OnBnClickedBtnImportRoom)
    ON_MESSAGE(WM_USER_NOTIFY_MESSAGE, &CBatchLoginDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_USER_LIST_INFO, &CBatchLoginDlg::OnDisplayDataToUserList)
    ON_MESSAGE(WM_USER_ROOM_LIST_INFO, &CBatchLoginDlg::OnDisplayDataToRoomList)

    ON_BN_CLICKED(IDC_BTN_LOGIN, &CBatchLoginDlg::OnBnClickedBtnLogin)
    ON_BN_CLICKED(IDC_BTN_SAVE_USER_PWD_COOKIE, &CBatchLoginDlg::OnBnClickedBtnSaveUserPwdCookie)
    //ON_NOTIFY(NM_CLICK, IDC_LIST_ROOM, &CBatchLoginDlg::OnNMClickListRoom)
    ON_BN_CLICKED(IDC_BTN_SEND_AWARD, &CBatchLoginDlg::OnBnClickedBtnSendAward)
    ON_BN_CLICKED(IDC_BTN_LOTTERY, &CBatchLoginDlg::OnBnClickedBtnLottery)
    ON_BN_CLICKED(IDC_BTN_SEND_SINGLE, &CBatchLoginDlg::OnBnClickedBtnSendSingle)
    ON_BN_CLICKED(IDC_BTN_BREAK, &CBatchLoginDlg::OnBnClickedBtnBreak)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL, &CBatchLoginDlg::OnBnClickedBtnSelectAll)
    ON_BN_CLICKED(IDC_BTN_REVERSE_SELECT, &CBatchLoginDlg::OnBnClickedBtnReverseSelect)
    ON_BN_CLICKED(IDC_BTN_DELETE, &CBatchLoginDlg::OnBnClickedBtnDelete)
    ON_BN_CLICKED(IDC_BTN_GET_USERINFO, &CBatchLoginDlg::OnBnClickedBtnGetUserinfo)
    ON_BN_CLICKED(IDC_BTN_CHANGE_NICKNAME, &CBatchLoginDlg::OnBnClickedBtnChangeNickname)
    ON_BN_CLICKED(IDC_BTN_CHANGE_LOGO, &CBatchLoginDlg::OnBnClickedBtnChangeLogo)
    ON_BN_CLICKED(IDC_BTN_SINGELIKE, &CBatchLoginDlg::OnBnClickedBtnSingelike)
    ON_BN_CLICKED(IDC_BTN_CHANGE_CONFIG_NICKNAME, &CBatchLoginDlg::OnBnClickedBtnChangeConfigNickname)
    ON_BN_CLICKED(IDC_BTN_BATCH_CHAT, &CBatchLoginDlg::OnBnClickedBtnBatchChat)
    ON_BN_CLICKED(IDC_BTN_SEND_STAR, &CBatchLoginDlg::OnBnClickedBtnSendStar)
    ON_BN_CLICKED(IDC_BTN_MV_Billboard, &CBatchLoginDlg::OnBnClickedBtnMvBillboard)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL_BLACK, &CBatchLoginDlg::OnBnClickedBtnSelectAllBlack)
    ON_BN_CLICKED(IDC_BTN_SELECT_REVERSE_BLACK, &CBatchLoginDlg::OnBnClickedBtnSelectReverseBlack)
    ON_BN_CLICKED(IDC_BTN_REMOVE_BLACK, &CBatchLoginDlg::OnBnClickedBtnRemoveBlack)
    ON_BN_CLICKED(IDC_BTN_LOAD_BLACK, &CBatchLoginDlg::OnBnClickedBtnLoadBlack)
    ON_BN_CLICKED(IDC_BTN_SAVE_BLACK, &CBatchLoginDlg::OnBnClickedBtnSaveBlack)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_MONTH_BLACK, &CBatchLoginDlg::OnBnClickedBtnKickoutMonthBlack)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_HOUR_BLACK, &CBatchLoginDlg::OnBnClickedBtnKickoutHourBlack)
    ON_BN_CLICKED(IDC_BTN_SILENT_BLACK, &CBatchLoginDlg::OnBnClickedBtnSilentBlack)
    ON_BN_CLICKED(IDC_BTN_UNSILENT_BLACK, &CBatchLoginDlg::OnBnClickedBtnUnsilentBlack)
    ON_BN_CLICKED(IDC_BTN_BAN_ENTER, &CBatchLoginDlg::OnBnClickedBtnBanEnter)
    ON_BN_CLICKED(IDC_BTN_UNBAN_ENTER, &CBatchLoginDlg::OnBnClickedBtnUnbanEnter)
END_MESSAGE_MAP()


// CBatchLoginDlg ��Ϣ�������

BOOL CBatchLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	ShowWindow(SW_NORMAL);

    DWORD dwStyle = m_ListCtrl_Users.GetExtendedStyle();
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//ѡ��ĳ��ʹ���и�����ֻ������report����listctrl��
    dwStyle |= LVS_EX_GRIDLINES;//�����ߣ�ֻ������report����listctrl��

    m_ListCtrl_Users.SetExtendedStyle(dwStyle); //������չ���
    int nColumnCount = m_ListCtrl_Users.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_Users.DeleteColumn(i);
    uint32 index = 0;
    for (const auto& it : usercolumnlist)
    {
        int len = 60;
        if (index == 0 || index == 1 || index == 4)
            len = 110;

        m_ListCtrl_Users.InsertColumn(index++, it, LVCFMT_LEFT, len);//������
    }

    m_ListCtrl_Blacks.SetExtendedStyle(dwStyle);
    nColumnCount = m_ListCtrl_Blacks.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_Blacks.DeleteColumn(i);
    index = 0;
    for (const auto& it : blackcolumnlist)
        m_ListCtrl_Blacks.InsertColumn(index++, it, LVCFMT_LEFT, 80);//������

    //m_ListCtrl_Rooms.SetExtendedStyle(dwStyle); //������չ���
    //nColumnCount = m_ListCtrl_Rooms.GetHeaderCtrl()->GetItemCount();
    //for (int i = nColumnCount - 1; i >= 0; i--)
    //    m_ListCtrl_Rooms.DeleteColumn(i);
    //index = 0;
    //for (const auto& it : roomcolumnlist)
    //    m_ListCtrl_Rooms.InsertColumn(index++, it, LVCFMT_LEFT, 100);//������
    
    m_list_proxy.SetExtendedStyle(dwStyle); //������չ���
    nColumnCount = m_list_proxy.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_list_proxy.DeleteColumn(i);
    index = 0;
    for (const auto& it : proxycolumnlist)
        m_list_proxy.InsertColumn(index++, it, LVCFMT_LEFT, 70);//������

    tcpManager_->Initialize();
    userRoomManager_->Initialize();
    userRoomManager_->SetNotify(
        std::bind(&CBatchLoginDlg::Notify,this,std::placeholders::_1));
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CBatchLoginDlg::OnPaint()
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
HCURSOR CBatchLoginDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CBatchLoginDlg::OnBnClickedBtnImportUser()
{
    GridData griddata;
    uint32 total = 0;
    if (!userRoomManager_->LoadUserConfig(&griddata, &total))
        return;
    
    assert(griddata.size() == total);

    // ��ʾ���ݵ�����
    int itemcount = m_ListCtrl_Users.GetItemCount();

    for (uint32 i = 0; i < griddata.size(); ++i)
    {
        bool exist = false;
        // ����Ƿ������ͬ�û�id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_ListCtrl_Users.GetItemText(index, 0);
            if (griddata[i][0].compare(text.GetBuffer()) == 0) // ��ͬ�û���
            {
                exist = true;
                break;
            }
        }

        if (!exist) // ��������ڣ���Ҫ����������
        {
            int nitem = m_ListCtrl_Users.InsertItem(itemcount + i, griddata[i][0].c_str());
            //m_ListCtrl_UserStatus.SetItemData(nitem, i);
            for (uint32 j = 0; j < griddata[i].size(); ++j)
            {
                m_ListCtrl_Users.SetItemText(nitem, j, griddata[i][j].c_str());
            }
        }
    }
}

void CBatchLoginDlg::OnBnClickedBtnLogin()
{
    userRoomManager_->SetBreakRequest(false);
    bool use_cookie = !!m_chk_use_cookie.GetCheck();
    int itemcount = m_ListCtrl_Users.GetItemCount();
    std::map<std::wstring, std::wstring> accountPassword;
    std::map<std::wstring, std::wstring> accountCookies;
    for (int32 index = 0; index < itemcount; ++index)
    {
        CString account = m_ListCtrl_Users.GetItemText(index, 0);
        if (!!m_ListCtrl_Users.GetCheck(index))
        {
            CString account = m_ListCtrl_Users.GetItemText(index, 0);
            CString password = m_ListCtrl_Users.GetItemText(index, 1);
            CString cookies = m_ListCtrl_Users.GetItemText(index, 2);

            // ��ʱȫ�����û��������¼����
            if (use_cookie)
                accountCookies[account.GetBuffer()] = cookies.GetBuffer(); 
            else
                accountPassword[account.GetBuffer()] = password.GetBuffer();
        }
    }
    if (!accountCookies.empty())
        userRoomManager_->BatchLogUsersWithCookie(accountCookies);

    if (!accountPassword.empty())
        userRoomManager_->BatchLogUsers(accountPassword);


}

//void CBatchLoginDlg::OnBnClickedBtnImportRoom()
//{
//    assert(false && L"��Ȳ���Ҫ����");
//    GridData griddata;
//    uint32 total = 0;
//    if (!userRoomManager_->LoadRoomConfig(&griddata, &total))
//        return;
//
//    assert(griddata.size() == total);
//
//    // ��ʾ���ݵ�����
//    int itemcount = m_ListCtrl_Rooms.GetItemCount();
//
//    for (uint32 i = 0; i < griddata.size(); ++i)
//    {
//        bool exist = false;
//        // ����Ƿ������ͬ�û�id
//        for (int index = 0; index < itemcount; index++)
//        {
//            CString text = m_ListCtrl_Rooms.GetItemText(index, 0);
//            if (griddata[i][0].compare(text.GetBuffer()) == 0) // ��ͬ�����
//            {
//                exist = true;
//                break;
//            }
//        }
//
//        if (!exist) // ��������ڣ���Ҫ����������
//        {
//            int nitem = m_ListCtrl_Rooms.InsertItem(itemcount + i, griddata[i][0].c_str());
//            //m_ListCtrl_UserStatus.SetItemData(nitem, i);
//            for (uint32 j = 0; j < griddata[i].size(); ++j)
//            {
//                m_ListCtrl_Rooms.SetItemText(nitem, j, griddata[i][j].c_str());
//            }
//        }
//    }
//}

void CBatchLoginDlg::OnBnClickedBtnGetProxy()
{
    assert(false && L"��Ȳ���Ҫ����");
    GridData griddata;
    if (!userRoomManager_->LoadIpProxy(&griddata))
        return;

    // ��ʾ���ݵ�����
    int itemcount = m_list_proxy.GetItemCount();

    for (uint32 i = 0; i < griddata.size(); ++i)
    {
        bool exist = false;
        // ����Ƿ������ͬip
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_list_proxy.GetItemText(index, 1);
            if (griddata[i][1].compare(text.GetBuffer()) == 0) // ��ͬip
            {
                exist = true;
                break;
            }
        }

        if (!exist) // ��������ڣ���Ҫ����������
        {
            int nitem = m_list_proxy.InsertItem(itemcount + i, griddata[i][0].c_str());
            for (uint32 j = 0; j < griddata[i].size(); ++j)
            {
                m_list_proxy.SetItemText(nitem, j, griddata[i][j].c_str());
            }
        }
    }
}

void CBatchLoginDlg::OnBnClickedBtnBatchEnterRoom()
{
    userRoomManager_->SetBreakRequest(false);
    //int itemcount = m_ListCtrl_Rooms.GetItemCount();
    //std::vector<std::wstring> roomids;
    //for (int32 index = 0; index < itemcount; ++index)
    //{
    //    if (!!m_ListCtrl_Rooms.GetCheck(index))
    //    {
    //        CString roomid = m_ListCtrl_Rooms.GetItemText(index, 0);
    //        roomids.push_back(roomid.GetBuffer());
    //    }
    //}

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    if (users.empty())
    {
        Notify(L"û�������û���ͶƱ����");
        return;
    }

    std::vector<std::wstring> roomids;
    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);
    roomids.push_back(cs_roomid.GetBuffer());
    userRoomManager_->FillRooms(users, roomids);
}

void CBatchLoginDlg::Notify(const std::wstring& message)
{
    // �������ݸ�����
    messageMutex_.lock();
    messageQueen_.push_back(message);
    messageMutex_.unlock();
    this->PostMessage(WM_USER_NOTIFY_MESSAGE, 0, 0);
}

LRESULT CBatchLoginDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::vector<std::wstring> messages;
    messageMutex_.lock();
    messages.swap(messageQueen_);
    messageMutex_.unlock();

    for (auto str : messages)
    {
        InfoList_.InsertString(infoListCount_++, str.c_str());
    }

    InfoList_.SetCurSel(infoListCount_ - 1);
    return 0;
}

LRESULT CBatchLoginDlg::OnDisplayDataToUserList(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CBatchLoginDlg::OnDisplayDataToRoomList(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

void CBatchLoginDlg::OnBnClickedBtnMvBillboard()
{
    //assert(false && L"��Ȳ���Ҫ����");
    userRoomManager_->SetBreakRequest(false);
    CString collectionid;
    CString mvid;
    m_mv_id.GetWindowTextW(mvid);
    collectionid = mvid;
    userRoomManager_->UpMVBillboard(collectionid.GetBuffer(), mvid.GetBuffer());
}

void CBatchLoginDlg::OnBnClickedBtnSaveUserPwdCookie()
{
    userRoomManager_->SaveUserLoginConfig();
}

//void CBatchLoginDlg::OnNMClickListRoom(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
//    // TODO:  �ڴ���ӿؼ�֪ͨ����������
//    int row_id = m_ListCtrl_Rooms.GetSelectionMark();
//    
//    assert(pNMItemActivate->iItem == row_id);
//
//    for (int i = 0; i < sizeof(roomcolumnlist) / sizeof(roomcolumnlist[0]);i++)
//    {
//        CString text = m_ListCtrl_Rooms.GetItemText(row_id, i);
//    }
//    *pResult = 0;
//}


void CBatchLoginDlg::OnBnClickedBtnSendAward()
{
    userRoomManager_->SetBreakRequest(false);

    // 869�Ǵ�Ʊ
    uint32 gift_id = 869;// õ���id��1
#ifdef _DEBUG
    gift_id = 1;
#endif
    SendGifts(gift_id);
}

void CBatchLoginDlg::OnBnClickedBtnSendSingle()
{
    userRoomManager_->SetBreakRequest(false);

    // 871�ǵ���Ʊ
    uint32 gift_id = 871;
#ifdef _DEBUG
    gift_id = 1;// õ���id��1
#endif
    SendGifts(gift_id);
}

bool CBatchLoginDlg::SendGifts(uint32 gift_id)
{
    CString cs_gift_count;
    m_gift_count.GetWindowTextW(cs_gift_count);
    uint32 gift_count = 0;
    base::StringToUint(base::WideToUTF8(cs_gift_count.GetBuffer()), &gift_count);

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    if (users.empty())
    {
        Notify(L"û�������û���ͶƱ����");
        return false;
    }

    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);

    userRoomManager_->SendGifts(users,
        cs_roomid.GetBuffer(), gift_id, gift_count);
    return true;
}

void CBatchLoginDlg::OnBnClickedBtnLottery()
{
    userRoomManager_->SetBreakRequest(false);
    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);

    int itemcount = m_ListCtrl_Users.GetItemCount();
    // �ӽ����ȡ��ѡ���û���
    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    if (users.empty())
    {
        Notify(L"û�������û��ĳ齱����");
        return;
    }
    if (cs_roomid.IsEmpty())
    {
        Notify(L"�����뷿���");
        return;
    }

    userRoomManager_->RobVotes(users, cs_roomid.GetBuffer());
}


void CBatchLoginDlg::OnBnClickedBtnBreak()
{
    userRoomManager_->SetBreakRequest(true);
}

void CBatchLoginDlg::GetSelectUsers(std::vector<std::wstring>* users)
{
    int itemcount = m_ListCtrl_Users.GetItemCount();
    // �ӽ����ȡ��ѡ���û���
    for (int32 index = 0; index < itemcount; ++index)
    {
        CString account = m_ListCtrl_Users.GetItemText(index, 0);
        if (!!m_ListCtrl_Users.GetCheck(index))
        {
            users->push_back(account.GetBuffer());
        }
    }
}

void CBatchLoginDlg::OnBnClickedBtnSelectAll()
{
    int itemcount = m_ListCtrl_Users.GetItemCount();
    for (int32 index = 0; index < itemcount; ++index)
    {
        m_ListCtrl_Users.SetCheck(index, TRUE);
    }
}

void CBatchLoginDlg::OnBnClickedBtnReverseSelect()
{
    int itemcount = m_ListCtrl_Users.GetItemCount();
    for (int32 index = 0; index < itemcount; ++index)
    {
        if (m_ListCtrl_Users.GetCheck(index))
        {
            m_ListCtrl_Users.SetCheck(index, FALSE);
        }
        else
        {
            m_ListCtrl_Users.SetCheck(index, TRUE);
        }
    }
}

void CBatchLoginDlg::OnBnClickedBtnDelete()
{
    int count = m_ListCtrl_Users.GetItemCount();
    int delete_count = 0;
    for (int index = count - 1; index >= 0; --index)
    {
        if (m_ListCtrl_Users.GetCheck(index))
        {
            m_ListCtrl_Users.DeleteItem(index);
            delete_count++;
        }  
    }
    std::wstring msg = L"�Ѿ�ɾ " + base::IntToString16(delete_count) + L"/" + base::IntToString16(count);
    Notify(msg);
}


void CBatchLoginDlg::OnBnClickedBtnGetUserinfo()
{
    userRoomManager_->SetBreakRequest(false);

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    std::vector<UserStorageInfo> user_storage_infos;
    userRoomManager_->GetUserStorageInfos(users, &user_storage_infos);


    int count = m_ListCtrl_Users.GetItemCount();
    for (int index = count - 1; index >= 0; --index)
    {
        if (!m_ListCtrl_Users.GetCheck(index))
            continue;

        CString cs_username = m_ListCtrl_Users.GetItemText(index, 0);
        std::string username = base::WideToUTF8(cs_username.GetBuffer());
        for (const auto& user_storage_info : user_storage_infos)
        {
            if (username.compare(user_storage_info.accountname) == 0)
            {
                m_ListCtrl_Users.SetItemText(index, 4,
                    base::UTF8ToWide(user_storage_info.nickname).c_str());
                m_ListCtrl_Users.SetItemText(index, 5,
                    base::UintToString16(user_storage_info.rich_level).c_str());
                m_ListCtrl_Users.SetItemText(index, 6,
                    base::UintToString16(user_storage_info.coin).c_str());
                m_ListCtrl_Users.SetItemText(index, 7,
                    base::UintToString16(user_storage_info.star_count).c_str());
                m_ListCtrl_Users.SetItemText(index, 8,
                    base::UintToString16(user_storage_info.gift_award).c_str());
                m_ListCtrl_Users.SetItemText(index, 9,
                    base::UintToString16(user_storage_info.gift_single).c_str());
                break;
            }
        }
    }
}

void CBatchLoginDlg::OnBnClickedBtnChangeNickname()
{
    CString nickname_pre;
    m_nickname.GetWindowTextW(nickname_pre);
    if (nickname_pre.IsEmpty())
    {
        Notify(L"����ʧ��");
        return;
    }

    if (nickname_pre.GetLength() > 5)
    {
        Notify(L"����ʧ��,����ǰ׺���ܳ���5���ַ�");
        return;
    }

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    userRoomManager_->SetBreakRequest(false);
    userRoomManager_->BatchChangeNickname(users, nickname_pre.GetBuffer());
}


void CBatchLoginDlg::OnBnClickedBtnChangeLogo()
{
    userRoomManager_->SetBreakRequest(false);

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    CString cs_logo_path;
    m_logo_path.GetWindowTextW(cs_logo_path);
    if (cs_logo_path.IsEmpty())
    {
        Notify(L"������ͷ��·��");
        return;
    }

    userRoomManager_->BatchChangeLogo(users, cs_logo_path.GetBuffer());
}


void CBatchLoginDlg::OnBnClickedBtnSingelike()
{
    userRoomManager_->SetBreakRequest(false);

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);

    CString cs_delta;
    m_edit_delta.GetWindowTextW(cs_delta);

    CString songname;
    m_edit_singlike.GetWindowTextW(songname);
    userRoomManager_->RealSingLike(users, cs_roomid.GetBuffer(), songname.GetBuffer(), cs_delta.GetBuffer());
}


void CBatchLoginDlg::OnBnClickedBtnChangeConfigNickname()
{
    BlacklistHelper blacklist_helper;

    std::map<uint32, BlackInfo> blackInfoMap;
    blacklist_helper.LoadFromFile(&blackInfoMap);

    std::vector<std::wstring> name_list;
    for (const auto& black : blackInfoMap)
    {
        name_list.push_back(base::UTF8ToWide(black.second.nickname));
    }

    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    userRoomManager_->SetBreakRequest(false);
    userRoomManager_->BatchChangeNicknameList(users, name_list);
}


void CBatchLoginDlg::OnBnClickedBtnBatchChat()
{
    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);

    CString cs_chat_message;
    m_edit_chat_message.GetWindowTextW(cs_chat_message);

    userRoomManager_->SetBreakRequest(false);
    userRoomManager_->BatchSendChat(cs_roomid.GetBuffer(), users, cs_chat_message.GetBuffer());
}


void CBatchLoginDlg::OnBnClickedBtnSendStar()
{
    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);

    CString cs_delta;
    m_edit_delta.GetWindowTextW(cs_delta);

    uint32 count = 1;
    userRoomManager_->BatchSendStar(users, cs_roomid.GetBuffer(), count);
}


void CBatchLoginDlg::OnBnClickedBtnSelectAllBlack()
{
    int count = m_ListCtrl_Blacks.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        m_ListCtrl_Blacks.SetCheck(i, 1);
    }
}


void CBatchLoginDlg::OnBnClickedBtnSelectReverseBlack()
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


void CBatchLoginDlg::OnBnClickedBtnRemoveBlack()
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
            Notify(itemtext.GetBuffer());

            // ɾ���Ѿ���ѡ�ļ�¼
            m_ListCtrl_Blacks.DeleteItem(i);
        }
    }
}


void CBatchLoginDlg::OnBnClickedBtnLoadBlack()
{
    //std::vector<RowData> rowdatas;
    //if (!blacklistHelper_->LoadBlackList(&rowdatas))
    //{
    //    CString itemtext = L"��ȡ������ʧ��";
    //    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, itemtext.GetBuffer());
    //    return;
    //}

    //blackRowdataMutex_.lock();
    //for (const auto& rowdata : rowdatas)
    //{
    //    assert(rowdata.size() == 2);
    //    blackRowdataQueue_.push_back(rowdata);
    //}
    //blackRowdataMutex_.unlock();

    //this->PostMessage(WM_USER_ADD_TO_BLACK_LIST, 0, 0);
}


void CBatchLoginDlg::OnBnClickedBtnSaveBlack()
{
    //std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    //GetSelectBlacks(&enterRoomUserInfos);

    //std::vector<RowData> rowdatas;
    //for (const auto& enterRoomUserInfo : enterRoomUserInfos)
    //{
    //    RowData rowdata;
    //    rowdata.push_back(base::UTF8ToWide(enterRoomUserInfo.nickname));
    //    rowdata.push_back(base::UintToString16(enterRoomUserInfo.userid));
    //    rowdatas.push_back(rowdata);
    //}

    //blacklistHelper_->SaveBlackList(rowdatas);
}


void CBatchLoginDlg::OnBnClickedBtnKickoutMonthBlack()
{
    //if (!network_)
    //    return;

    //std::wstring privilegeMsg;
    //if (!network_->GetActionPrivilege(&privilegeMsg))
    //{
    //    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
    //    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, privilegeMsg);
    //    return;
    //}
    //std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    //GetSelectBlacks(&enterRoomUserInfos);
    //KickOut_(enterRoomUserInfos, KICK_TYPE::KICK_TYPE_MONTH);
}


void CBatchLoginDlg::OnBnClickedBtnKickoutHourBlack()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CBatchLoginDlg::OnBnClickedBtnSilentBlack()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CBatchLoginDlg::OnBnClickedBtnUnsilentBlack()
{
    // TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CBatchLoginDlg::OnBnClickedBtnBanEnter()
{
    //if (!network_)
    //    return;

    //std::wstring privilegeMsg;
    //if (!network_->GetActionPrivilege(&privilegeMsg))
    //{
    //    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
    //    return;
    //}
    //std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    //GetSelectBlacks(&enterRoomUserInfos);
    //BanEnter_(enterRoomUserInfos);
}


void CBatchLoginDlg::OnBnClickedBtnUnbanEnter()
{
    //if (!network_)
    //    return;

    //std::wstring privilegeMsg;
    //if (!network_->GetActionPrivilege(&privilegeMsg))
    //{
    //    Notify(MessageLevel::MESSAGE_LEVEL_DISPLAY, NOPRIVILEGE_NOTICE);
    //    return;
    //}
    //std::vector<EnterRoomUserInfo> enterRoomUserInfos;
    //GetSelectBlacks(&enterRoomUserInfos);
    //UnbanEnter_(enterRoomUserInfos);
}
