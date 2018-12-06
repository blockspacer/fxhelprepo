
// SingerRankDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "SingerRank.h"
#include "SingerRankDlg.h"
#include "afxdialogex.h"
#include "PhoneRank.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/time/time.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSingerRankDlg 对话框
namespace
{
	const wchar_t* singercolumnlist[] = {
		L"时间",
		L"主播",
		L"主播等级",
		L"开播次数",
		L"累计直播",
		L"有效直播",
		L"直播间最高人气",
		L"星豆收入"
	};
}


CSingerRankDlg::CSingerRankDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSingerRankDlg::IDD, pParent)
    , worker_thread_("WorkerThread")
    , message_index(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSingerRankDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_SINGER_ID, m_edit_roomid);
    DDX_Control(pDX, IDC_LIST_SINGERS, m_singer_list);
    DDX_Control(pDX, IDC_LIST_INFO, m_list_info);
    DDX_Control(pDX, IDC_CHK_BEAUTY, m_chk_beautiful);
    DDX_Control(pDX, IDC_CHK_NEW_SINGER, m_chk_new_singer);
    DDX_Control(pDX, IDC_EDIT_CLANID, m_edit_clan);
}

BEGIN_MESSAGE_MAP(CSingerRankDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_SEARCH_RANK, &CSingerRankDlg::OnBnClickedBtnSearchRank)
    ON_BN_CLICKED(IDC_BTN_GET_ROOM_RANK, &CSingerRankDlg::OnBnClickedBtnGetRoomRank)
    ON_BN_CLICKED(IDC_BTN_CITY_RANK, &CSingerRankDlg::OnBnClickedBtnCityRank)
    ON_MESSAGE(WM_USER_MSG, &CSingerRankDlg::OnMessage)
    ON_MESSAGE(WM_USER_PROGRESS, &CSingerRankDlg::OnProgress)
    ON_MESSAGE(WM_USER_FOUND_RESULT, &CSingerRankDlg::OnFoundResult)
	ON_MESSAGE(WM_USER_SINGER_RESULT, &CSingerRankDlg::OnDisplaySingerResult)
    ON_BN_CLICKED(IDC_BTN_CLAN_RETRIVE, &CSingerRankDlg::OnBnClickedBtnClanRetrive)
	ON_NOTIFY(LVN_GETDISPINFO, IDC_LIST_SINGERS, &CSingerRankDlg::OnLvnGetdispinfoListSummaryData)
END_MESSAGE_MAP()


// CSingerRankDlg 消息处理程序

BOOL CSingerRankDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码
    CurlWrapper::CurlInit();

    auto singer_info_callback = base::Bind(&CSingerRankDlg::SingerInfoCallback, base::Unretained(this));

    auto message_callback = base::Bind(&CSingerRankDlg::MessageCallback, base::Unretained(this));

    if (!worker_thread_.Start())
        return false;
    
    phone_rank_.Initialize(worker_thread_.task_runner().get(), singer_info_callback, message_callback);
    singer_retriver_.Initialize(worker_thread_.task_runner().get());

	
	// TODO:  在此添加额外的初始化代码
	DWORD dwStyle = m_singer_list.GetExtendedStyle();
//dwStyle |= LVS_EX_CHECKBOXES;
	dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
	dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）
	dwStyle |= LVS_OWNERDATA;
	dwStyle |= LVS_AUTOARRANGE;

	m_singer_list.SetExtendedStyle(dwStyle); //设置扩展风格
	int nColumnCount = m_singer_list.GetHeaderCtrl()->GetItemCount();
	for (int i = nColumnCount - 1; i >= 0; i--)
		m_singer_list.DeleteColumn(i);
	int index = 0;
	RECT rect;
	m_singer_list.GetWindowRect(&rect);
	int time_width = 60;
	int width = (rect.right - rect.left - time_width) / ((sizeof(singercolumnlist) / sizeof(singercolumnlist[0]) - 1));
	bool first = true;
	for (const auto& it : singercolumnlist)
	{
		if (first)
		{
			first = false;
			m_singer_list.InsertColumn(index++, it, LVCFMT_LEFT, time_width);//插入列
		}
		else
		{
			m_singer_list.InsertColumn(index++, it, LVCFMT_LEFT, width);//插入列
		}
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CSingerRankDlg::OnClose()
{
    singer_retriver_.Finalize();
    phone_rank_.Finalize();
    worker_thread_.Stop();
}

void CSingerRankDlg::OnOK()
{

}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSingerRankDlg::OnPaint()
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
HCURSOR CSingerRankDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSingerRankDlg::OnBnClickedBtnSearchRank()
{
    if (!!m_chk_beautiful.GetCheck())
    {
        // 女神
        phone_rank_.InitBeautifulSingerRankInfos();
    }
    
    if (!!m_chk_new_singer.GetCheck())
    {
        // 新主播
        phone_rank_.InitNewSingerRankInfos();
    }
}


void CSingerRankDlg::OnBnClickedBtnGetRoomRank()
{
    CString cs_roomid;
    m_edit_roomid.GetWindowTextW(cs_roomid);
    uint32 roomid = 0;
    base::StringToUint(base::WideToUTF8(cs_roomid.GetBuffer()), &roomid);
    if (roomid <= 0)
    {
        MessageBox(L"房间号错误", L"请重新输入", 0);
    }

    // 获取新秀排名
    uint32 rank = 0;
    uint32 all = 0;

    auto singer_info_callback = base::Bind(&CSingerRankDlg::SingerInfoCallback, base::Unretained(this));

    auto message_callback = base::Bind(&CSingerRankDlg::MessageCallback, base::Unretained(this));

    if (!phone_rank_.GetBeautifulSingerRankByRoomid(roomid, &rank, &all))
    {
        MessageCallback(L"无法获取女神分类排名");
    }
    else
    {
        std::wstring message;
        message += base::UTF8ToWide(base::UintToString(roomid));
        message += L"女神分类 rank ( ";
        message += base::UTF8ToWide(base::UintToString(rank));
        message += L"/";
        message += base::UTF8ToWide(base::UintToString(all));
        message += L" )";

        MessageCallback(message);
    }

    if (!phone_rank_.GetNewSingerRankByRoomid(roomid, &rank, &all))
    {
        MessageCallback(L"无法获取新秀分类排名");
    }
    else
    {
        std::wstring message;
        message += base::UTF8ToWide(base::UintToString(roomid));
        message += L"新秀分类 rank ( ";
        message += base::UTF8ToWide(base::UintToString(rank));
        message += L"/";
        message += base::UTF8ToWide(base::UintToString(all));
        message += L" )";

        MessageCallback(message);
    }
}

void CSingerRankDlg::OnBnClickedBtnCityRank()
{
    CString cs_roomid;
    m_edit_roomid.GetWindowTextW(cs_roomid);
    uint32 roomid = 0;
    base::StringToUint(base::WideToUTF8(cs_roomid.GetBuffer()), &roomid);
    if (roomid<=0)
    {
        MessageBox(L"房间号错误", L"请重新输入", 0);
    }

    phone_rank_.GetCityRankInfos(roomid);
}

LRESULT CSingerRankDlg::OnMessage(WPARAM wParam, LPARAM lParam)
{
    scoped_ptr<std::wstring> message((std::wstring*)(lParam));
    m_list_info.InsertString(message_index++, message->c_str());
    return 0;
}

LRESULT CSingerRankDlg::OnProgress(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CSingerRankDlg::OnFoundResult(WPARAM wParam, LPARAM lParam)
{
    return 0;
}

LRESULT CSingerRankDlg::OnDisplaySingerResult(WPARAM wParam, LPARAM lParam)
{
	scoped_ptr<RowData> row_data((RowData*)(lParam));

	// 显示增加一条信息
	m_griddata.push_back(*row_data.get());

	int nItemCount = m_griddata.size();
	m_singer_list.SetItemCountEx(nItemCount);
	m_singer_list.Invalidate();
	UpdateData(FALSE);
	return 0;
}

void CSingerRankDlg::SingerInfoCallback(uint32 roomid, bool result, const RowData& singer_infos)
{
	std::string time_string = MakeFormatTimeString(base::Time::Now());

	std::wstring* new_wstring = new std::wstring(base::UTF8ToWide(time_string) + L" " + 
		base::UintToString16(roomid) + L"获取排名" + (result?L"成功":L"失败"));
	this->PostMessage(WM_USER_MSG, 0, (LPARAM)(new_wstring));
	RowData* row = new RowData(singer_infos);
	this->PostMessage(WM_USER_SINGER_RESULT, 0, (LPARAM)(row));
}

void CSingerRankDlg::MessageCallback(const std::wstring& message)
{
    std::string time_string = MakeFormatTimeString(base::Time::Now());
    std::wstring* new_wstring = new std::wstring(base::UTF8ToWide(time_string) + L" " + message);
    this->PostMessage(WM_USER_MSG, 0, (LPARAM)(new_wstring));
}

void CSingerRankDlg::ClanSingerCallback(const std::vector<uint32>& roomids)
{
    for (auto roomid : roomids)
    {
        phone_rank_.RetriveSingerRankResult(roomid);
    }
}

void CSingerRankDlg::OneSingerInfoCallback(const RowData& singer_info)
{

}

void CSingerRankDlg::OnBnClickedBtnClanRetrive()
{
	//删除之前的数据
	if (m_singer_list.GetItemCount()>0)
	{
		m_singer_list.SetItemCountEx(0);
		m_singer_list.Invalidate();
		m_singer_list.UpdateWindow();
	}

    CString cs_clanid;
    m_edit_clan.GetWindowTextW(cs_clanid);

    uint32 clan_id = 0;
    base::StringToUint(base::WideToUTF8(cs_clanid.GetBuffer()), &clan_id);

    singer_retriver_.GetSingerInfoByClan(clan_id, 
        base::Bind(&CSingerRankDlg::ClanSingerCallback, base::Unretained(this)));
}

void CSingerRankDlg::DisplayDataToGrid()
{
	//生成新的数据缓冲区
	int nItemCount = m_griddata.size();
	m_singer_list.SetItemCountEx(nItemCount);
	m_singer_list.Invalidate();
}

void CSingerRankDlg::OnLvnGetdispinfoListSummaryData(NMHDR *pNMHDR, LRESULT *pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	LV_ITEM *pItem = &(pDispInfo)->item;
	if (pItem->mask & LVIF_TEXT)
	{
		//使缓冲区数据与表格子项对应
		pItem->pszText = (LPWSTR)m_griddata[pItem->iItem][pItem->iSubItem].c_str();
	}

	*pResult = 0;
}
