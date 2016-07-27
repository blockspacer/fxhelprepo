
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
#

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
        L"房间数"
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
    DDX_Control(pDX, IDC_LIST_ROOM, m_ListCtrl_Rooms);
    DDX_Control(pDX, IDC_LIST_INFO, InfoList_);
    DDX_Control(pDX, IDC_EDIT_MV_COLLECTION_ID, m_mv_collection_id);
    DDX_Control(pDX, IDC_EDIT_MV_ID, m_mv_id);
    DDX_Control(pDX, IDC_LIST_PROXY, m_list_proxy);
}

BEGIN_MESSAGE_MAP(CBatchLoginDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_IMPORT_USER, &CBatchLoginDlg::OnBnClickedBtnImportUser)
    ON_BN_CLICKED(IDC_BTN_GET_PROXY, &CBatchLoginDlg::OnBnClickedBtnGetProxy)
    ON_BN_CLICKED(IDC_BTN_BATCH_ENTER_ROOM, &CBatchLoginDlg::OnBnClickedBtnBatchEnterRoom)
    ON_BN_CLICKED(IDC_BTN_IMPORT_ROOM, &CBatchLoginDlg::OnBnClickedBtnImportRoom)
    ON_MESSAGE(WM_USER_NOTIFY_MESSAGE, &CBatchLoginDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_USER_LIST_INFO, &CBatchLoginDlg::OnDisplayDataToUserList)
    ON_MESSAGE(WM_USER_ROOM_LIST_INFO, &CBatchLoginDlg::OnDisplayDataToRoomList)

    ON_BN_CLICKED(IDC_BTN_LOGIN, &CBatchLoginDlg::OnBnClickedBtnLogin)
    ON_BN_CLICKED(IDC_BTN_UP_MV_BILLBOARD, &CBatchLoginDlg::OnBnClickedBtnUpMvBillboard)
    ON_BN_CLICKED(IDC_BTN_SAVE_USER_PWD_COOKIE, &CBatchLoginDlg::OnBnClickedBtnSaveUserPwdCookie)
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
        m_ListCtrl_Users.InsertColumn(index++, it, LVCFMT_LEFT, 100);//插入列


    m_ListCtrl_Rooms.SetExtendedStyle(dwStyle); //设置扩展风格
    nColumnCount = m_ListCtrl_Rooms.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_Rooms.DeleteColumn(i);
    index = 0;
    for (const auto& it : roomcolumnlist)
        m_ListCtrl_Rooms.InsertColumn(index++, it, LVCFMT_LEFT, 100);//插入列

    
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
    int itemcount = m_ListCtrl_Users.GetItemCount();
    std::map<std::wstring, std::wstring> accountPassword;
    std::map<std::wstring, std::wstring> accountCookies;
    for (int32 index = 0; index < itemcount; ++index)
    {
        CString account = m_ListCtrl_Users.GetItemText(index, 0);
        CString password = m_ListCtrl_Users.GetItemText(index, 1);
        CString cookies = m_ListCtrl_Users.GetItemText(index, 2);

        //// 暂时全部走用户名密码登录流程
        //accountPassword[account.GetBuffer()] = password.GetBuffer();
        if (cookies.IsEmpty())
            accountPassword[account.GetBuffer()] = password.GetBuffer();
        else
            accountCookies[account.GetBuffer()] = cookies.GetBuffer();
    }
    if (!accountPassword.empty())
        userRoomManager_->BatchLogUsers(accountPassword);

    if (!accountCookies.empty())
        userRoomManager_->BatchLogUsersWithCookie(accountCookies);
}

void CBatchLoginDlg::OnBnClickedBtnImportRoom()
{
    GridData griddata;
    uint32 total = 0;
    if (!userRoomManager_->LoadRoomConfig(&griddata, &total))
        return;

    assert(griddata.size() == total);

    // 显示数据到界面
    int itemcount = m_ListCtrl_Rooms.GetItemCount();

    for (uint32 i = 0; i < griddata.size(); ++i)
    {
        bool exist = false;
        // 检测是否存在相同用户id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_ListCtrl_Rooms.GetItemText(index, 0);
            if (griddata[i][0].compare(text.GetBuffer()) == 0) // 相同房间号
            {
                exist = true;
                break;
            }
        }

        if (!exist) // 如果不存在，需要插入新数据
        {
            int nitem = m_ListCtrl_Rooms.InsertItem(itemcount + i, griddata[i][0].c_str());
            //m_ListCtrl_UserStatus.SetItemData(nitem, i);
            for (uint32 j = 0; j < griddata[i].size(); ++j)
            {
                m_ListCtrl_Rooms.SetItemText(nitem, j, griddata[i][j].c_str());
            }
        }
    }
}

void CBatchLoginDlg::OnBnClickedBtnGetProxy()
{
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
    int itemcount = m_ListCtrl_Rooms.GetItemCount();
    std::vector<std::wstring> roomids;
    for (int32 index = 0; index < itemcount; ++index)
    {
        if (!!m_ListCtrl_Rooms.GetCheck(index))
        {
            CString roomid = m_ListCtrl_Rooms.GetItemText(index, 0);
            roomids.push_back(roomid.GetBuffer());
        }
    }
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
