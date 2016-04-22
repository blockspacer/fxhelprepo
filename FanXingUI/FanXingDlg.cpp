
// FanXingDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <xutility>
#include "FanXing.h"
#include "FanXingDlg.h"
#include "afxdialogex.h"
#include "NetworkHelper.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/utf_string_conversions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace
{
    const wchar_t* columnlist[] = {
        L"昵称",
        L"财富等级",
        L"用户id",
        L"进房时间",
        L"房间号",
        L"进入次数"
    };
}

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CFanXingDlg 对话框



CFanXingDlg::CFanXingDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFanXingDlg::IDD, pParent)
    , network_(nullptr)
    , infoListCount_(0)
    , listCtrlRowIndex_(0)
    , m_query_key(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME); 
}

CFanXingDlg::~CFanXingDlg()
{
    if (network_)
    {
        network_->Finalize();
    }  
}

void CFanXingDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST1, InfoList_);
    DDX_Control(pDX, IDC_LIST_USER_STATUS, m_ListCtrl_UserStatus);
    DDX_Text(pDX, IDC_EDIT_QUERY_KEYWORD, m_query_key);
    DDX_Control(pDX, IDC_CHECK_REMEMBER, m_check_remember);
}

BEGIN_MESSAGE_MAP(CFanXingDlg, CDialogEx)
    ON_WM_SYSCOMMAND()
    ON_WM_PAINT()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BUTTON_LOGIN, &CFanXingDlg::OnBnClickedButtonLogin)
    ON_BN_CLICKED(IDC_BUTTON_CLICK, &CFanXingDlg::OnBnClickedButtonClick)
    ON_BN_CLICKED(IDC_BUTTON_REWARSTAR, &CFanXingDlg::OnBnClickedButtonRewarstar)
    ON_BN_CLICKED(IDC_BUTTON_REWARDGIFT, &CFanXingDlg::OnBnClickedButtonRewardgift)
    ON_BN_CLICKED(IDC_BUTTON_NAV, &CFanXingDlg::OnBnClickedButtonNav)
    ON_WM_LBUTTONDOWN()
    ON_BN_CLICKED(IDC_BTN_GETMSG, &CFanXingDlg::OnBnClickedBtnGetmsg)
    ON_BN_CLICKED(IDC_BTN_ADD, &CFanXingDlg::OnBnClickedBtnAdd)
    ON_MESSAGE(WM_USER_01, &CFanXingDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_ADD_ENTER_ROOM_INFO, &CFanXingDlg::OnDisplayDataToGrid)
    ON_NOTIFY(HDN_ITEMCLICK, 0, &CFanXingDlg::OnHdnItemclickListUserStatus)
    ON_BN_CLICKED(IDC_BUTTON_REMOVE, &CFanXingDlg::OnBnClickedButtonRemove)
    ON_BN_CLICKED(IDC_BTN_MODIFY, &CFanXingDlg::OnBnClickedBtnModify)
    ON_BN_CLICKED(IDC_BTN_QUERY, &CFanXingDlg::OnBnClickedBtnQuery)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL, &CFanXingDlg::OnBnClickedBtnSelectAll)
    ON_BN_CLICKED(IDC_BTN_SELECT_REVERSE, &CFanXingDlg::OnBnClickedBtnSelectReverse)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_MONTH, &CFanXingDlg::OnBnClickedBtnKickoutMonth)
    ON_BN_CLICKED(IDC_BTN_KICKOUT_HOUR, &CFanXingDlg::OnBnClickedBtnKickoutHour)
    ON_BN_CLICKED(IDC_BTN_SILENT, &CFanXingDlg::OnBnClickedBtnSilent)
    ON_BN_CLICKED(IDC_BTN_UNSILENT, &CFanXingDlg::OnBnClickedBtnUnsilent)
    ON_BN_CLICKED(IDC_BTN_CLEAR, &CFanXingDlg::OnBnClickedBtnClear)
END_MESSAGE_MAP()


// CFanXingDlg 消息处理程序

BOOL CFanXingDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

    DWORD dwStyle = m_ListCtrl_UserStatus.GetExtendedStyle();
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
    m_ListCtrl_UserStatus.SetExtendedStyle(dwStyle); //设置扩展风格

    SetDlgItemText(IDC_EDIT_NAV, L"1062091");
    SetDlgItemInt(IDC_EDIT_X, 0);
    SetDlgItemInt(IDC_EDIT_Y, 0);

    int nColumnCount = m_ListCtrl_UserStatus.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_ListCtrl_UserStatus.DeleteColumn(i);

    uint32 i = 0;
    for (const auto& it : columnlist)
    {
        m_ListCtrl_UserStatus.InsertColumn(i++, it, LVCFMT_LEFT, 100);//插入列
    }

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFanXingDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CFanXingDlg::OnPaint()
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

void CFanXingDlg::OnClose()
{
    if (network_)
    {
        network_->RemoveNotify();
    }  
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFanXingDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

// 登录功能
void CFanXingDlg::OnBnClickedButtonLogin()
{
    //UserController usercontroller;
    //usercontroller.Run();
    CString username;
    CString password;
    GetDlgItemText(IDC_EDIT_Username, username);
    GetDlgItemText(IDC_EDIT_Password, password);

    // 测试通过的curl登录方式
    bool result = LoginByRequest(username.GetBuffer(), password.GetBuffer());
    std::wstring message = std::wstring(L"login ") + (result ? L"success" : L"failed");
    Notify(message);
}

//跳转页面功能
void CFanXingDlg::OnBnClickedButtonNav()
{
    // 先清空原来数据
    m_ListCtrl_UserStatus.DeleteAllItems();

    CString strRoomid;
    GetDlgItemText(IDC_EDIT_NAV, strRoomid);
    network_->SetNotify(
        std::bind(&CFanXingDlg::Notify, this, std::placeholders::_1));

    network_->SetNotify201(
        std::bind(&CFanXingDlg::Notify201, this, std::placeholders::_1));

    network_->EnterRoom(strRoomid.GetBuffer());
}

//指定位置点击功能
void CFanXingDlg::OnBnClickedButtonClick()
{

}

// 送星星功能
void CFanXingDlg::OnBnClickedButtonRewarstar()
{
}

// 送礼物功能
void CFanXingDlg::OnBnClickedButtonRewardgift()
{
}

void CFanXingDlg::OnLButtonDown(UINT nFlags, CPoint point)
{
    CDialogEx::OnLButtonDown(nFlags, point);
}

// 获取公屏信息
void CFanXingDlg::OnBnClickedBtnGetmsg()
{
}

// 用来做测试的函数
void CFanXingDlg::OnBnClickedBtnAdd()
{
    RowData rowdata;
    rowdata.push_back(L"1");
    rowdata.push_back(L"2");
    rowdata.push_back(L"3");
    rowdata.push_back(L"4");
    rowdata.push_back(L"5");
    rowdata.push_back(L"1");

    int nitem = m_ListCtrl_UserStatus.InsertItem(0, rowdata[0].c_str());
    m_ListCtrl_UserStatus.SetItemData(nitem, listCtrlRowIndex_++);
    for (uint32 j = 1; j < rowdata.size(); ++j)
    {
        m_ListCtrl_UserStatus.SetItemText(nitem, j, rowdata[j].c_str());
    }
}

void CFanXingDlg::Notify(const std::wstring& message)
{
    // 发送数据给窗口
    messageMutex_.lock();
    messageQueen_.push_back(message);
    messageMutex_.unlock();
    this->PostMessage(WM_USER_01, 0, 0);
}

void CFanXingDlg::Notify201(const RowData& rowdata)
{
    // 发送数据给窗口
    rowdataMutex_.lock();
    rowdataQueue_.push_back(rowdata);
    rowdataMutex_.unlock();
    this->PostMessage(WM_USER_ADD_ENTER_ROOM_INFO, 0, 0);
}

bool CFanXingDlg::LoginByRequest(const std::wstring& username, const std::wstring& password)
{
    if (network_)
    {
        network_->Finalize();
    }
    network_.reset(new NetworkHelper);
    network_->Initialize();
    network_->SetNotify(
        std::bind(&CFanXingDlg::Notify, this, std::placeholders::_1));

    return network_->Login(username, password);
}

// 界面线程执行
LRESULT CFanXingDlg::OnDisplayDataToGrid(WPARAM wParam, LPARAM lParam)
{
    if (rowdataQueue_.empty())
        return 0;
    
    std::vector<RowData> rowdatas;
    rowdataMutex_.lock();
    rowdatas.swap(rowdataQueue_);
    rowdataMutex_.unlock();

    int itemcount = m_ListCtrl_UserStatus.GetItemCount();

    for (uint32 i = 0; i < rowdatas.size(); ++i)
    {
        bool exist = false;
        // 检测是否存在相同用户id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_ListCtrl_UserStatus.GetItemText(index, 2);
            if (rowdatas[i][2].compare(text.GetBuffer()) == 0) // 相同用户id
            {
                // 更新进入次数显示，其他的数据都全部默认更新的
                CString itemText = m_ListCtrl_UserStatus.GetItemText(index, 5);
                std::string temp = base::WideToUTF8(itemText.GetBuffer());
                uint32 entercount = 0;
                base::StringToUint(temp, &entercount);
                entercount++;
                CString strEnterCount = base::UintToString16(entercount).c_str();
                m_ListCtrl_UserStatus.SetItemText(index, 5, strEnterCount);

                for (uint32 j = 0; j < rowdatas[i].size(); ++j)
                {
                    m_ListCtrl_UserStatus.SetItemText(itemcount + i, j, rowdatas[i][j].c_str());
                }
                exist = true;
                break;
            }
        }

        if (!exist) // 如果不存在，需要插入新数据
        {
            int nitem = m_ListCtrl_UserStatus.InsertItem(itemcount + i, rowdatas[i][0].c_str());
            m_ListCtrl_UserStatus.SetItemData(nitem, i);
            for (uint32 j = 1; j < rowdatas[i].size(); ++j)
            {
                m_ListCtrl_UserStatus.SetItemText(itemcount + nitem, j, rowdatas[i][j].c_str());
            }
            m_ListCtrl_UserStatus.SetItemText(nitem, 5, L"1"); // 第一次记录数据
        }
    }

    return 0;
}

LRESULT CFanXingDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::vector<std::wstring> messages;
    messageMutex_.lock();
    messages.swap(messageQueen_);
    messageMutex_.unlock();

    for (auto str : messages)
    {
        InfoList_.InsertString(infoListCount_++, str.c_str());
    }
    
    InfoList_.SetCurSel(infoListCount_-1);
    return 0;
}

void CFanXingDlg::OnBnClickedButton2()
{
    m_ListCtrl_UserStatus.SetCheck(0);
}

// 比较方法
int CFanXingDlg::CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
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

// 排序
void CFanXingDlg::OnHdnItemclickListUserStatus(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    
    m_ListCtrl_UserStatus.SortItems(CompareFunc, reinterpret_cast <DWORD> (this));

    *pResult = 0;}


void CFanXingDlg::OnBnClickedButtonRemove()
{
    int count = m_ListCtrl_UserStatus.GetItemCount();

    // 从后往前删除
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_UserStatus.GetCheck(i))
        {
            // 把要删除的消息发到日志记录列表上
            CString itemtext = m_ListCtrl_UserStatus.GetItemText(i, 2);
            itemtext + L"被从列表中删除";
            InfoList_.InsertString(infoListCount_++, itemtext.GetBuffer());

            // 删除已经勾选的记录
            m_ListCtrl_UserStatus.DeleteItem(i);
        }
    }
}


void CFanXingDlg::OnBnClickedBtnModify()
{
    // TODO:  在此添加控件通知处理程序代码
}


void CFanXingDlg::OnBnClickedBtnQuery()
{
    UpdateData(TRUE);
    CString key = m_query_key;
    int count = m_ListCtrl_UserStatus.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        CString temp = m_ListCtrl_UserStatus.GetItemText(i, 1);
        if (temp.Find(key)>=0)
        {
            m_ListCtrl_UserStatus.SetCheck(i, TRUE);
        }
    }
}


void CFanXingDlg::OnBnClickedBtnSelectAll()
{
    int count = m_ListCtrl_UserStatus.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        m_ListCtrl_UserStatus.SetCheck(i, 1);
    }
}


void CFanXingDlg::OnBnClickedBtnSelectReverse()
{
    int count = m_ListCtrl_UserStatus.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_UserStatus.GetCheck(i))
        {
            m_ListCtrl_UserStatus.SetCheck(i,FALSE);
        }
        else
        {
            m_ListCtrl_UserStatus.SetCheck(i, TRUE);
        }
    }
}


void CFanXingDlg::OnBnClickedBtnKickoutMonth()
{
    int count = m_ListCtrl_UserStatus.GetItemCount();

    // 从后往前删除
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_UserStatus.GetCheck(i))
        {
            // 发送踢出房间的网络请求

            // 把要删除的消息发到日志记录列表上
            CString itemtext = m_ListCtrl_UserStatus.GetItemText(i, 2);
            itemtext += L"被踢出一个月，并从列表中删除";
            InfoList_.InsertString(infoListCount_++, itemtext.GetBuffer());

            // 删除已经勾选的记录
            m_ListCtrl_UserStatus.DeleteItem(i);
        }
    }
}


void CFanXingDlg::OnBnClickedBtnKickoutHour()
{
    // 从后往前删除
    int count = m_ListCtrl_UserStatus.GetItemCount();
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_ListCtrl_UserStatus.GetCheck(i))
        {
            // 数据格式
            //RowData rowdata;
            //rowdata.push_back(base::SysUTF8ToWide(enterRoomUserInfo.nickname)); 0
            //rowdata.push_back(base::UintToString16(enterRoomUserInfo.richlevel)); 1
            //rowdata.push_back(base::UintToString16(enterRoomUserInfo.userid)); 2
            //base::Time entertime = base::Time::FromDoubleT(enterRoomUserInfo.unixtime); 3
            //std::wstring time = base::SysUTF8ToWide(MakeFormatTimeString(entertime).c_str());
            //rowdata.push_back(time);
            //rowdata.push_back(base::UintToString16(enterRoomUserInfo.roomid)); 4

            // 需要获取singerid,
            uint32 singerid = singerid_;

            // 发送踢出房间的网络请求
            EnterRoomUserInfo enterRoomUserInfo;
            CString temp = m_ListCtrl_UserStatus.GetItemText(i, 4);
            uint32 roomid = 0;
            base::StringToUint(m_ListCtrl_UserStatus.GetItemText(i, 4).GetBuffer(), &roomid);
            enterRoomUserInfo.roomid = roomid;
            uint32 richlevel = 0;
            base::StringToUint(m_ListCtrl_UserStatus.GetItemText(i, 1).GetBuffer(), &richlevel);
            enterRoomUserInfo.richlevel = richlevel;
            
            enterRoomUserInfo.nickname = base::WideToUTF8(m_ListCtrl_UserStatus.GetItemText(i, 0).GetBuffer());
            uint32 userid = 0;
            base::StringToUint(m_ListCtrl_UserStatus.GetItemText(i, 2).GetBuffer(), &userid);
            enterRoomUserInfo.userid = userid;
            network_->KickoutUsers(enterRoomUserInfo.roomid, enterRoomUserInfo);

            // 把要删除的消息发到日志记录列表上, id = 2 是用户id
            CString itemtext = L"userid=" + m_ListCtrl_UserStatus.GetItemText(i, 2);
            itemtext += L"被踢出一小时，并从列表中删除";
            InfoList_.InsertString(infoListCount_++, itemtext.GetBuffer());

            // 删除已经勾选的记录
            m_ListCtrl_UserStatus.DeleteItem(i);
        }
    }

}


void CFanXingDlg::OnBnClickedBtnSilent()
{
    // TODO:  在此添加控件通知处理程序代码
}


void CFanXingDlg::OnBnClickedBtnUnsilent()
{
    // TODO:  在此添加控件通知处理程序代码
}


void CFanXingDlg::OnBnClickedBtnClear()
{
    int count = m_ListCtrl_UserStatus.GetItemCount();
    // 从后往前删除
    for (int i = count - 1; i >= 0; --i)
    {
        m_ListCtrl_UserStatus.DeleteItem(i);
    }

    CString itemtext = L"清空列表";
    InfoList_.InsertString(infoListCount_++, itemtext.GetBuffer());
}
