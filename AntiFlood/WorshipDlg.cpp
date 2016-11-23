// WorshipDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "AntiFlood.h"
#include "WorshipDlg.h"
#include "afxdialogex.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"


// CWorshipDlg 对话框
namespace
{
    
    const wchar_t* worshiplist[] = {
        L"神号", 
        L"房间号",
    };
}
IMPLEMENT_DYNAMIC(WorshipDlg, CDialogEx)

WorshipDlg::WorshipDlg(NetworkHelper* network_helper, CWnd* pParent /*=NULL*/)
	: CDialogEx(WorshipDlg::IDD, pParent),
    network_helper_(network_helper)
{

}

WorshipDlg::~WorshipDlg()
{
}

BOOL WorshipDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    DWORD dwStyle = m_list_worship.GetExtendedStyle();
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）

    m_list_worship.SetExtendedStyle(dwStyle); //设置扩展风格
    int nColumnCount = m_list_worship.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_list_worship.DeleteColumn(i);
    uint32 index = 0;
    for (const auto& it : worshiplist)
        m_list_worship.InsertColumn(index++, it, LVCFMT_LEFT, 80);//插入列

    return TRUE;
}

void WorshipDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_FANXING_ID, m_edit_fanxing_id);
    DDX_Control(pDX, IDC_EDIT_ROOMID, m_edit_roomid);
    DDX_Control(pDX, IDC_DATETIMEPICKER1, m_time_worship);
    DDX_Control(pDX, IDC_CHK_AUTO_WORSHIP, m_check_auto_worship);
    DDX_Control(pDX, IDC_LIST_WORSHIP, m_list_worship);
}


BEGIN_MESSAGE_MAP(WorshipDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_WORSHIP, &WorshipDlg::OnBnClickedBtnWorship)
    ON_BN_CLICKED(IDC_BTN_ADD_TO_LIST, &WorshipDlg::OnBnClickedBtnAddToList)
    ON_BN_CLICKED(IDC_BTN_WORSHIP_SELECT, &WorshipDlg::OnBnClickedBtnWorshipSelect)
    ON_BN_CLICKED(IDC_BTN_DELETE_SELECT, &WorshipDlg::OnBnClickedBtnDeleteSelect)
END_MESSAGE_MAP()


// CWorshipDlg 消息处理程序


void WorshipDlg::OnBnClickedBtnWorship()
{
    CString cs_worship_fanxingid;
    m_edit_fanxing_id.GetWindowTextW(cs_worship_fanxingid);
    CString cs_worship_roomid;
    m_edit_roomid.GetWindowTextW(cs_worship_roomid);

    std::string str_roomid = base::WideToUTF8(cs_worship_roomid.GetBuffer());
    uint32 roomid = 0;
    base::StringToUint(str_roomid, &roomid);

    std::string str_fanxingid = base::WideToUTF8(cs_worship_fanxingid.GetBuffer());
    uint32 fanxingid = 0;
    base::StringToUint(str_fanxingid, &fanxingid);

    if (!roomid || !fanxingid)
    {
        return;
    }

    std::string error_msg;
    if (!network_helper_->Worship(roomid, fanxingid, &error_msg))
    {
        std::wstring w_error_msg = base::UTF8ToWide(error_msg);
        MessageBoxW(w_error_msg.c_str(), L"膜拜失败");
        return;
    }
}

void WorshipDlg::OnBnClickedBtnAddToList()
{
    CString cs_worship_fanxingid;
    m_edit_fanxing_id.GetWindowTextW(cs_worship_fanxingid);
    CString cs_worship_roomid;
    m_edit_roomid.GetWindowTextW(cs_worship_roomid);

    std::string str_roomid = base::WideToUTF8(cs_worship_roomid.GetBuffer());
    uint32 roomid = 0;
    base::StringToUint(str_roomid, &roomid);

    std::string str_fanxingid = base::WideToUTF8(cs_worship_fanxingid.GetBuffer());
    uint32 fanxingid = 0;
    base::StringToUint(str_fanxingid, &fanxingid);

    if (!roomid || !fanxingid)
        return;

    int itemcount = m_list_worship.GetItemCount();

    bool exist = false;
    // 检测是否存在相同用户id
    for (int index = 0; index < itemcount; index++)
    {
        CString temp_fanxing_id = m_list_worship.GetItemText(index, 0);
        CString temp_room_id = m_list_worship.GetItemText(index, 1);
        if ((cs_worship_fanxingid.Compare(temp_fanxing_id) == 0)
            && (cs_worship_roomid.Compare(temp_room_id) == 0))
        {
            exist = true;
            break;
        }
    }

    if (!exist) // 如果不存在，需要插入新数据
    {
        int nitem = m_list_worship.InsertItem(itemcount, cs_worship_fanxingid);
        m_list_worship.SetItemText(nitem, 0, cs_worship_fanxingid);
        m_list_worship.SetItemText(nitem, 1, cs_worship_roomid);
    }
}


void WorshipDlg::OnBnClickedBtnWorshipSelect()
{
    std::vector<worship_pair> vec_data;
    if (!GetSelectItems(&vec_data))
        return;
    
    for (const auto& it : vec_data)
    {
        std::string str_roomid = base::WideToUTF8(it.second);
        uint32 roomid = 0;
        base::StringToUint(str_roomid, &roomid);

        std::string str_fanxingid = base::WideToUTF8(it.first);
        uint32 fanxingid = 0;
        base::StringToUint(str_fanxingid, &fanxingid);

        if (!roomid || !fanxingid)
        {
            return;
        }

        std::string error_msg;
        if (!network_helper_->Worship(roomid, fanxingid, &error_msg))
        {
            std::wstring w_error_msg = base::UTF8ToWide(error_msg);
        }
    }
}

bool WorshipDlg::GetSelectItems(std::vector<worship_pair>* select_items)
{
    if (!select_items)
        return false;

    int count = m_list_worship.GetItemCount();
    for (int index = 0; index < count; ++index)
    {
        if (m_list_worship.GetCheck(index))
        {
            std::wstring temp_fanxing_id = m_list_worship.GetItemText(index, 0).GetBuffer();
            std::wstring temp_room_id = m_list_worship.GetItemText(index, 1).GetBuffer();
            select_items->push_back(std::make_pair(temp_fanxing_id, temp_room_id));
        }
    }
    return true;
}

void WorshipDlg::OnBnClickedBtnDeleteSelect()
{
    int count = m_list_worship.GetItemCount();
    for (int index = count-1; index >=0; --index)
    {
        if (m_list_worship.GetCheck(index))
        {
            std::wstring temp_fanxing_id = m_list_worship.GetItemText(index, 0).GetBuffer();
            std::wstring temp_room_id = m_list_worship.GetItemText(index, 1).GetBuffer();
            m_list_worship.DeleteItem(index);
        }
    }
}
