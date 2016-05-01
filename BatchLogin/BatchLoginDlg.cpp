
// BatchLoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "BatchLogin.h"
#include "BatchLoginDlg.h"
#include "UserRoomManager.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace{
    const wchar_t* usercolumnlist[] = {
        L"用户名",
        L"密码",
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
}

// CBatchLoginDlg 对话框

CBatchLoginDlg::CBatchLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CBatchLoginDlg::IDD, pParent)
    , userRoomManager_(nullptr)
{
    userRoomManager_.reset(new UserRoomManager);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CBatchLoginDlg::~CBatchLoginDlg()
{

}

void CBatchLoginDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST_USERS, m_ListCtrl_Users);
    DDX_Control(pDX, IDC_LIST_ROOM, m_ListCtrl_Rooms);
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
END_MESSAGE_MAP()


// CBatchLoginDlg 消息处理程序

BOOL CBatchLoginDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MINIMIZE);

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
    // TODO:  在此添加控件通知处理程序代码
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

}

void CBatchLoginDlg::OnBnClickedBtnBatchEnterRoom()
{
    userRoomManager_->FillConfigRooms();
}

LRESULT CBatchLoginDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
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



