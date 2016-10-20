
// BatchLoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <string>
#include <map>
#include "BatchLogin.h"
#include "BatchLoginDlg.h"
#include "UserRoomManager.h"
#include "Network/TcpManager.h"
#include "Network/TcpClient.h"
#include "afxdialogex.h"

#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace{
    const wchar_t* usercolumnlist[] = {
        L"用户名",
        L"密码",
        L"Cookie",
        L"用户id",
        L"昵称",
        L"财富等级",
        L"星币",
        L"免费票"
    };

    const wchar_t* roomcolumnlist[] = {
        L"房间号",
        L"协议人数",
        L"显示人数",
        L"有效期",
    };

    const wchar_t* proxycolumnlist[] = {
        L"代理协议",
        L"代理ip",
        L"代理端口"
    };
}

// CBatchLoginDlg 对话框

CBatchLoginDlg::CBatchLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBatchLoginDlg::IDD, pParent)
    , userRoomManager_(nullptr)
{
    tcpManager_.reset(new TcpManager);
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
    DDX_Control(pDX, IDC_EDIT_GIFT_COUNT, m_gift_count);
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
END_MESSAGE_MAP()


// CBatchLoginDlg 消息处理程序

BOOL CBatchLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_NORMAL);

    DWORD dwStyle = m_ListCtrl_Users.GetExtendedStyle();
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）

    m_ListCtrl_Users.SetExtendedStyle(dwStyle); //设置扩展风格
    int nColumnCount = m_ListCtrl_Users.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_Users.DeleteColumn(i);
    uint32 index = 0;
    for (const auto& it : usercolumnlist)
        m_ListCtrl_Users.InsertColumn(index++, it, LVCFMT_LEFT, 90);//插入列


    //m_ListCtrl_Rooms.SetExtendedStyle(dwStyle); //设置扩展风格
    //nColumnCount = m_ListCtrl_Rooms.GetHeaderCtrl()->GetItemCount();
    //for (int i = nColumnCount - 1; i >= 0; i--)
    //    m_ListCtrl_Rooms.DeleteColumn(i);
    //index = 0;
    //for (const auto& it : roomcolumnlist)
    //    m_ListCtrl_Rooms.InsertColumn(index++, it, LVCFMT_LEFT, 100);//插入列
    
    m_list_proxy.SetExtendedStyle(dwStyle); //设置扩展风格
    nColumnCount = m_list_proxy.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_list_proxy.DeleteColumn(i);
    index = 0;
    for (const auto& it : proxycolumnlist)
        m_list_proxy.InsertColumn(index++, it, LVCFMT_LEFT, 70);//插入列

    tcpManager_->Initialize();
    userRoomManager_->Initialize();
    userRoomManager_->SetNotify(
        std::bind(&CBatchLoginDlg::Notify,this,std::placeholders::_1));
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CBatchLoginDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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

    // 显示数据到界面
    int itemcount = m_ListCtrl_Users.GetItemCount();

    for (uint32 i = 0; i < griddata.size(); ++i)
    {
        bool exist = false;
        // 检测是否存在相同用户id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_ListCtrl_Users.GetItemText(index, 0);
            if (griddata[i][0].compare(text.GetBuffer()) == 0) // 相同用户名
            {
                exist = true;
                break;
            }
        }

        if (!exist) // 如果不存在，需要插入新数据
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

            // 暂时全部走用户名密码登录流程
            //accountPassword[account.GetBuffer()] = password.GetBuffer();
            if (cookies.IsEmpty())
                accountPassword[account.GetBuffer()] = password.GetBuffer();
            else
                accountCookies[account.GetBuffer()] = cookies.GetBuffer();
        }
    }
    if (!accountPassword.empty())
        userRoomManager_->BatchLogUsers(accountPassword);

    if (!accountCookies.empty())
        userRoomManager_->BatchLogUsersWithCookie(accountCookies);
}

//void CBatchLoginDlg::OnBnClickedBtnImportRoom()
//{
//    assert(false && L"年度不需要操作");
//    GridData griddata;
//    uint32 total = 0;
//    if (!userRoomManager_->LoadRoomConfig(&griddata, &total))
//        return;
//
//    assert(griddata.size() == total);
//
//    // 显示数据到界面
//    int itemcount = m_ListCtrl_Rooms.GetItemCount();
//
//    for (uint32 i = 0; i < griddata.size(); ++i)
//    {
//        bool exist = false;
//        // 检测是否存在相同用户id
//        for (int index = 0; index < itemcount; index++)
//        {
//            CString text = m_ListCtrl_Rooms.GetItemText(index, 0);
//            if (griddata[i][0].compare(text.GetBuffer()) == 0) // 相同房间号
//            {
//                exist = true;
//                break;
//            }
//        }
//
//        if (!exist) // 如果不存在，需要插入新数据
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
    assert(false && L"年度不需要操作");
    GridData griddata;
    if (!userRoomManager_->LoadIpProxy(&griddata))
        return;

    // 显示数据到界面
    int itemcount = m_list_proxy.GetItemCount();

    for (uint32 i = 0; i < griddata.size(); ++i)
    {
        bool exist = false;
        // 检测是否存在相同ip
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_list_proxy.GetItemText(index, 1);
            if (griddata[i][1].compare(text.GetBuffer()) == 0) // 相同ip
            {
                exist = true;
                break;
            }
        }

        if (!exist) // 如果不存在，需要插入新数据
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
    std::vector<std::wstring> roomids;
    CString cs_roomid;
    m_roomid.GetWindowTextW(cs_roomid);
    roomids.push_back(cs_roomid.GetBuffer());
    userRoomManager_->FillRooms(roomids);
}

void CBatchLoginDlg::Notify(const std::wstring& message)
{
    // 发送数据给窗口
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

void CBatchLoginDlg::OnBnClickedBtnUpMvBillboard()
{
    assert(false && L"年度不需要操作");
    userRoomManager_->SetBreakRequest(false);
    CString collectionid;
    CString mvid;
    m_mv_collection_id.GetWindowTextW(collectionid);
    m_mv_id.GetWindowTextW(mvid);
    userRoomManager_->UpMVBillboard(collectionid.GetBuffer(), mvid.GetBuffer());
}

void CBatchLoginDlg::OnBnClickedBtnSaveUserPwdCookie()
{
    userRoomManager_->SaveUserLoginConfig();
}

//void CBatchLoginDlg::OnNMClickListRoom(NMHDR *pNMHDR, LRESULT *pResult)
//{
//    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
//    // TODO:  在此添加控件通知处理程序代码
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

    // 869是大奖票
    uint32 gift_id = 869;// 玫瑰的id是1
#ifdef _DEBUG
    gift_id = 1;
#endif
    SendGifts(gift_id);
}

void CBatchLoginDlg::OnBnClickedBtnSendSingle()
{
    userRoomManager_->SetBreakRequest(false);

    // 871是单项票
    uint32 gift_id = 871;
#ifdef _DEBUG
    gift_id = 1;// 玫瑰的id是1
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
        Notify(L"没有设置用户的投票任务");
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
    // 从界面获取勾选的用户名
    std::vector<std::wstring> users;
    GetSelectUsers(&users);

    if (users.empty())
    {
        Notify(L"没有设置用户的抽奖任务");
        return;
    }
    if (cs_roomid.IsEmpty())
    {
        Notify(L"请输入房间号");
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
    // 从界面获取勾选的用户名
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
    std::wstring msg = L"已经删 " + base::IntToString16(delete_count) + L"/" + base::IntToString16(count);
    Notify(msg);
}
