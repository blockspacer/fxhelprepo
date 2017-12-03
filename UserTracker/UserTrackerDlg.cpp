
// UserTrackerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UserTracker.h"
#include "UserTrackerDlg.h"
#include "afxdialogex.h"
#include "UserTrackerHelper.h"
#include "AuthorityHelper.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_enumerator.h"
#include "Network/EncodeHelper.h"
#include <shellapi.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUserTrackerDlg 对话框
namespace
{
    struct ProgressStruct 
    {
        uint32 current;
        uint32 all;
    };
}


CUserTrackerDlg::CUserTrackerDlg(CWnd* pParent,
    UserTrackerHelper* tracker_helper)
	: CDialogEx(CUserTrackerDlg::IDD, pParent)
    , tracker_helper_(tracker_helper)
    , list_info_count_(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CUserTrackerDlg::~CUserTrackerDlg()
{
}

void CUserTrackerDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_USER_ID, m_edit_target_fanxing_id);
    DDX_Control(pDX, IDC_LIST_MESSAGE, m_list_message);
    DDX_Control(pDX, IDC_EDIT_ACCOUNT, m_edit_account);
    DDX_Control(pDX, IDC_EDIT_PASSWORD, m_edit_password);
    DDX_Control(pDX, IDC_STATIC_ROOM_PROGRESS, m_static_room_progress);
    DDX_Control(pDX, IDC_PROGRESS1, m_progress1);
    DDX_Control(pDX, IDC_LIST_RESULT, m_list_result);
    DDX_Control(pDX, IDC_EDIT_ROOM_ID, m_edit_roomid);
    DDX_Control(pDX, IDC_CHK_STAR, m_check_star);
    DDX_Control(pDX, IDC_CHK_DIAMON, m_check_diamon);
    DDX_Control(pDX, IDC_CHK_1_4_CROWN, m_check_1_3_crown);
    DDX_Control(pDX, IDC_CHK_5_CROWN, m_check_4_crown_up);
    DDX_Control(pDX, IDC_EDIT_LAST_ONLINE, m_edit_last_online_min);
    DDX_Control(pDX, IDC_EDIT_HOT_KEY, m_edit_hot_keys);
    DDX_Control(pDX, IDC_EDIT_HOT_KEY_TIMES, m_edit_hot_key_times);
    DDX_Control(pDX, IDC_EDIT_ROOMID_MIN, m_edit_roomid_min);
    DDX_Control(pDX, IDC_EDIT_ROOMID_MAX, m_edit_roomid_max);
    DDX_Control(pDX, IDC_EDIT_MIN_LEVEL, m_edit_min_star_level);
}

BEGIN_MESSAGE_MAP(CUserTrackerDlg, CDialogEx)
    ON_WM_PAINT()
    ON_WM_CLOSE()
    ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_GETALLROOMDATA, &CUserTrackerDlg::OnBnClickedBtnGetAllRoomData)
    ON_BN_CLICKED(IDC_BTN_FIND_IN_CACHE, &CUserTrackerDlg::OnBnClickedBtnFindInCache)
    ON_BN_CLICKED(IDC_BTN_UPDATA_FIND, &CUserTrackerDlg::OnBnClickedBtnUpdataFind)
    ON_MESSAGE(WM_USER_MSG, &CUserTrackerDlg::OnNotifyMessage)
    ON_MESSAGE(WM_USER_PROGRESS, &CUserTrackerDlg::OnRoomProgress)
    ON_MESSAGE(WM_USER_FOUND_RESULT, &CUserTrackerDlg::OnFoundResult)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CUserTrackerDlg::OnBnClickedBtnLogin)
    ON_BN_CLICKED(IDC_BTN_CANCEL, &CUserTrackerDlg::OnBnClickedBtnCancel)
    ON_NOTIFY(NM_CLICK, IDC_LIST_RESULT, &CUserTrackerDlg::OnNMClickListResult)
    ON_NOTIFY(HDN_BEGINTRACK, 0, &CUserTrackerDlg::OnHdnBegintrackListResult)
    ON_BN_CLICKED(IDC_BTN_CLEAR_LIST, &CUserTrackerDlg::OnBnClickedBtnClearList)
    ON_BN_CLICKED(IDC_BTN_CLEAR_CACHE, &CUserTrackerDlg::OnBnClickedBtnClearCache)
    ON_BN_CLICKED(IDC_BTN_TAGS_BEAUTY, &CUserTrackerDlg::OnBnClickedBtnTagsBeauty)
    ON_BN_CLICKED(IDC_BTN_HOT_SEARCH_HIT, &CUserTrackerDlg::OnBnClickedBtnHotSearchHit)
    ON_BN_CLICKED(IDC_BTN_SEARCH_RANGE, &CUserTrackerDlg::OnBnClickedBtnSearchRange)
    ON_BN_CLICKED(IDC_BTN_IMPORT_ROOMS, &CUserTrackerDlg::OnBnClickedBtnImportRooms)
    ON_BN_CLICKED(IDC_BTN_EXPORT_ROOMS, &CUserTrackerDlg::OnBnClickedBtnExportRooms)
END_MESSAGE_MAP()


// CUserTrackerDlg 消息处理程序

BOOL CUserTrackerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    //tracker_helper_->Initialize();
    tracker_helper_->SetNotifyMessageCallback(
        base::Bind(&CUserTrackerDlg::NotifyMessage, base::Unretained(this)));

    // 在窗口标题增加授权信息
    std::wstring title_post = tracker_helper_->GetAuthorityMessage();
    CString title;
    GetWindowTextW(title);
    title += title_post.c_str();
    SetWindowTextW(title);

    m_progress1.SetRange(0, 100);
    DWORD dwStyle = m_list_result.GetExtendedStyle();
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）

    m_list_result.SetExtendedStyle(dwStyle); //设置扩展风格

    RECT rect;
    m_list_result.GetWindowRect(&rect);
    int width = (rect.right - rect.left) / 2;
    m_list_result.InsertColumn(0, L"id", LVCFMT_LEFT, width);//插入列
    m_list_result.InsertColumn(1, L"房间号", LVCFMT_LEFT, width-2);//插入列

    m_check_star.SetCheck(TRUE);
    m_check_diamon.SetCheck(TRUE);
    m_check_1_3_crown.SetCheck(TRUE);
    m_check_4_crown_up.SetCheck(TRUE);

    m_edit_last_online_min.SetWindowTextW(L"300");
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CUserTrackerDlg::OnClose()
{
    tracker_helper_->CancelCurrentOperation();
    tracker_helper_->Finalize();
    CDialogEx::OnClose();
}

void CUserTrackerDlg::OnOK()
{
    // 避免按回车直接退出
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CUserTrackerDlg::OnPaint()
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
HCURSOR CUserTrackerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CUserTrackerDlg::SetHScroll()
{
    CDC* dc = GetDC();

    CString str;
    int index = m_list_message.GetCount() - 1;
    if (index >= 0)
    {
        m_list_message.GetText(index, str);
        SIZE s = dc->GetTextExtent(str);
        long temp = (long)SendDlgItemMessage(IDC_LIST_MESSAGE, LB_GETHORIZONTALEXTENT, 0, 0); //temp得到滚动条的宽度
        if (s.cx > temp)
        {
            SendDlgItemMessage(IDC_LIST_MESSAGE, LB_SETHORIZONTALEXTENT, (WPARAM)s.cx, 0);
        }
    }

    ReleaseDC(dc);
}

void CUserTrackerDlg::OnBnClickedBtnGetAllRoomData()
{
    CString cs_star_level;
    m_edit_min_star_level.GetWindowTextW(cs_star_level);
    uint32 min_star_level = 0;
    base::StringToUint(base::WideToUTF8(cs_star_level.GetBuffer()), &min_star_level);

    tracker_helper_->SetSearchConfig(
        min_star_level,
        !!m_check_star.GetCheck(),
        !!m_check_diamon.GetCheck(),
        !!m_check_1_3_crown.GetCheck(),
        !!m_check_4_crown_up.GetCheck());

    if (!tracker_helper_->UpdataAllStarRoomForNoClan(
        base::Bind(&CUserTrackerDlg::RoomProgress, base::Unretained(this)),
        base::Bind(&CUserTrackerDlg::FoundResult, base::Unretained(this))))
    {
        NotifyMessage(L"操作失败, 请先登录");
    } 
}

void CUserTrackerDlg::OnBnClickedBtnFindInCache()
{
    CString cs_user_id;
    m_edit_target_fanxing_id.GetWindowTextW(cs_user_id);
    uint32 user_id = 0;
    base::StringToUint(base::WideToUTF8(cs_user_id.GetBuffer()), &user_id);
    std::vector<uint32> users;
    users.push_back(user_id);
    if (!tracker_helper_->GetUserLocationByUserId(users,
        base::Bind(&CUserTrackerDlg::RoomProgress, base::Unretained(this)),
        base::Bind(&CUserTrackerDlg::FoundResult, base::Unretained(this))))
    {
        NotifyMessage(L"操作失败, 请先登录");
    }
}

void CUserTrackerDlg::OnBnClickedBtnClearCache()
{
    tracker_helper_->ClearCache();
}

void CUserTrackerDlg::OnBnClickedBtnUpdataFind()
{
    CString cs_star_level;
    m_edit_min_star_level.GetWindowTextW(cs_star_level);
    uint32 min_star_level = 0;
    base::StringToUint(base::WideToUTF8(cs_star_level.GetBuffer()), &min_star_level);

    tracker_helper_->SetSearchConfig(
        min_star_level,
        !!m_check_star.GetCheck(),
        !!m_check_diamon.GetCheck(),
        !!m_check_1_3_crown.GetCheck(),
        !!m_check_4_crown_up.GetCheck());

    CString cs_user_id;
    m_edit_target_fanxing_id.GetWindowTextW(cs_user_id);
    uint32 user_id = 0;
    base::StringToUint(base::WideToUTF8(cs_user_id.GetBuffer()), &user_id);
    std::vector<uint32> users;
    users.push_back(user_id);
    if (!tracker_helper_->UpdateForFindUser(users,
        base::Bind(&CUserTrackerDlg::RoomProgress, base::Unretained(this)),
        base::Bind(&CUserTrackerDlg::FoundResult, base::Unretained(this))))
    {
        NotifyMessage(L"操作失败, 请先登录");
    }

}

void CUserTrackerDlg::NotifyMessage(const std::wstring& message)
{
    // 发送数据给窗口
    message_mutex_.lock();
    message_queen_.push_back(message);
    message_mutex_.unlock();
    this->PostMessage(WM_USER_MSG, 0, 0);
}

void CUserTrackerDlg::RoomProgress(uint32 current, uint32 all)
{
    ProgressStruct* progress = new ProgressStruct;
    progress->all = all;
    progress->current = current;
    this->PostMessage(WM_USER_PROGRESS, (WPARAM)(PROGRESSTYPE_ROOM), (LPARAM)progress);
}

void CUserTrackerDlg::FoundResult(uint32 user_id, uint32 room_id)
{
    this->PostMessage(WM_USER_FOUND_RESULT, (WPARAM)(user_id), (LPARAM)room_id);
}

LRESULT CUserTrackerDlg::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::vector<std::wstring> messages;
    message_mutex_.lock();
    messages.swap(message_queen_);
    message_mutex_.unlock();

    for (auto str : messages)
    {
        m_list_message.InsertString(list_info_count_++, str.c_str());
    }

    m_list_message.SetCurSel(list_info_count_ - 1);
    SetHScroll();
    return 0;
}

LRESULT CUserTrackerDlg::OnRoomProgress(WPARAM wParam, LPARAM lParam)
{
    ProgressType pt = (ProgressType)(wParam);
    ProgressStruct* ps = (ProgressStruct*)(lParam);

    int pos = static_cast<int>(ps->current*100.0 / ps->all);
    std::wstring show_msg = base::UintToString16(ps->current) +
        L" / " + base::UintToString16(ps->all);
    switch (pt)
    {
    case CUserTrackerDlg::PROGRESSTYPE_ROOM:   
        m_progress1.SetPos(pos);
        m_static_room_progress.SetWindowTextW(show_msg.c_str());
        break;
    case CUserTrackerDlg::PROGRESSTYPE_USER:
        break;
    default:
        break;
    }

    delete ps;
    return 0;
}

LRESULT CUserTrackerDlg::OnFoundResult(WPARAM wParam, LPARAM lParam)
{
    uint32 user_id = (uint32)(wParam);
    uint32 room_id = (uint32)(lParam);

    int nitem = m_list_result.InsertItem(0, L"");
    m_list_result.SetItemData(nitem, result_index_++);
    m_list_result.SetItemText(nitem, 0, base::UintToString16(user_id).c_str());
    m_list_result.SetItemText(nitem, 1, base::UintToString16(room_id).c_str());

    return 0;
}

void CUserTrackerDlg::OnBnClickedBtnLogin()
{
    CString cs_account;
    CString cs_password;
    m_edit_account.GetWindowText(cs_account);
    m_edit_password.GetWindowText(cs_password);

    std::string account = base::WideToUTF8(cs_account.GetBuffer());
    std::string password = base::WideToUTF8(cs_password.GetBuffer());
    std::string verifycode = "";

    auto callback = base::Bind(&CUserTrackerDlg::LoginResult,
        base::Unretained(this));
    if (!tracker_helper_->LoginUser(account, password, verifycode, callback))
    {
        AfxMessageBox(L"登录失败");
        return;
    }
}

void CUserTrackerDlg::LoginResult(bool result, uint32 server_time, const std::string& errormsg)
{
    if (!result)
        AfxMessageBox(L"登录失败");
}

void CUserTrackerDlg::OnBnClickedBtnCancel()
{
    tracker_helper_->CancelCurrentOperation();
}

void CUserTrackerDlg::OnNMClickListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO:  在此添加控件通知处理程序代码
    NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;
    if (pNMListView->iItem != -1)
    {
        CString cs_item = m_list_result.GetItemText(
            pNMListView->iItem, pNMListView->iSubItem);
        std::wstring url = L"http://fanxing.kugou.com/";
        url += cs_item.GetBuffer();
        ShellExecute(NULL, _T("open"), url.c_str(), NULL, NULL, SW_SHOWNORMAL);
        m_edit_roomid.SetWindowTextW(cs_item);
    }

    *pResult = 0;
}

void CUserTrackerDlg::OnHdnBegintrackListResult(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMHEADER phdr = reinterpret_cast<LPNMHEADER>(pNMHDR);
    // 禁止拖动列头
    *pResult = TRUE;
}


void CUserTrackerDlg::OnBnClickedBtnClearList()
{
    int itemcount = m_list_result.GetItemCount();
    for (int index = itemcount - 1; index >= 0; --index)
    {
        m_list_result.DeleteItem(index);
        result_index_--;
    }
}

void CUserTrackerDlg::OnBnClickedBtnTagsBeauty()
{
    CString cs_last_online_min;
    m_edit_last_online_min.GetWindowText(cs_last_online_min);

    uint32 days = 0;
    base::StringToUint(base::WideToUTF8(cs_last_online_min.GetBuffer()), &days);

    tracker_helper_->GetAllBeautyStarForNoClan(days,
        base::Bind(&CUserTrackerDlg::RoomProgress, base::Unretained(this)),
        base::Bind(&CUserTrackerDlg::FoundResult, base::Unretained(this)));
}


void CUserTrackerDlg::OnBnClickedBtnHotSearchHit()
{
    CString cs_hot_keys;
    m_edit_hot_keys.GetWindowText(cs_hot_keys);

    CString cs_hot_key_times;
    m_edit_hot_key_times.GetWindowText(cs_hot_key_times);

    uint32 hot_key_times = 0;
    base::StringToUint(base::WideToUTF8(cs_hot_key_times.GetBuffer()), &hot_key_times);
    tracker_helper_->RunSearchHotKey(cs_hot_keys.GetBuffer(), hot_key_times,
        base::Bind(&CUserTrackerDlg::RoomProgress, base::Unretained(this)));

}

void CUserTrackerDlg::OnBnClickedBtnSearchRange()
{
    CString cs_last_online_min;
    m_edit_last_online_min.GetWindowText(cs_last_online_min);
    uint32 days = 0;
    base::StringToUint(base::WideToUTF8(cs_last_online_min.GetBuffer()), &days);

    CString cs_roomid_min;
    m_edit_roomid_min.GetWindowText(cs_roomid_min);
    uint32 roomid_min = 0;
    base::StringToUint(base::WideToUTF8(cs_roomid_min.GetBuffer()), &roomid_min);

    CString cs_roomid_max;
    m_edit_roomid_max.GetWindowText(cs_roomid_max);
    uint32 roomid_max = 0;
    base::StringToUint(base::WideToUTF8(cs_roomid_max.GetBuffer()), &roomid_max);

    tracker_helper_->SetRangeSearchCallback(
        base::Bind(&CUserTrackerDlg::RoomProgress, base::Unretained(this)),
        base::Bind(&CUserTrackerDlg::RangeSearchResult, base::Unretained(this)));
    tracker_helper_->RunSearchRoomIdRange(roomid_min, roomid_max);
}

void CUserTrackerDlg::RangeSearchResult(uint32 roomid, const SingerInfo& singer_info,
    uint32 status, RangSearchErrorCode error)
{

}

void CUserTrackerDlg::OnBnClickedBtnImportRooms()
{
    CString FilePathName;
    CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
        NULL,
        NULL,
        OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
        (LPCTSTR)_TEXT("TXT Files (*.txt)|*.txt|All Files (*.*)|*.*||"),
        NULL);

    if (dlg.DoModal() != IDOK)
        return;

    FilePathName = dlg.GetPathName(); //文件名保存在了FilePathName里
    if (FilePathName.IsEmpty())
        return;

    base::FilePath path(FilePathName.GetBuffer());
    std::vector<std::wstring> roomids;
    tracker_helper_->LoadRooms(path.value(), &roomids);

    for (auto sroomid : roomids)
    {
        uint32 roomid = 0;
        base::StringToUint(sroomid,&roomid);
        FoundResult(0, roomid);
    }
}


void CUserTrackerDlg::OnBnClickedBtnExportRooms()
{
    base::FilePath path;
    if (!UserTrackerHelper::GetOutputFileName(&path))
        return;

    base::File file;
    file.Initialize(path, base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);

    int itemcount = m_list_result.GetItemCount();
    for (int index = 0; index < itemcount; ++index)
    {
        
        std::wstring w_roomid = m_list_result.GetItemText(index, 1).GetBuffer();
        std::string roomid = base::WideToUTF8(w_roomid);
        LOG(INFO) << "=======" << roomid;
        file.WriteAtCurrentPos(roomid.c_str(), roomid.size());
        file.WriteAtCurrentPos("\r\n", 2);
    }

    file.Close();

    MessageBoxW(path.value().c_str(), L"导入完成", 0);
}
