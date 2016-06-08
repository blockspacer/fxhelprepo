// DlgRegister.cpp : 实现文件
//

#include "stdafx.h"
#include "DlgRegister.h"
#include "atlimage.h"
#include "afxdialogex.h"
#include "RegisterHelper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

namespace
{
    const wchar_t* proxycolumnlist[] = {
        L"代理协议",
        L"代理ip",
        L"代理端口"
    };

    // 因为验证码位置对着小键盘有上下翻转的差别，所以适应键盘设计输入，然后在这里反转
    bool TranslateVerifyCode(const std::string& src, std::string* dest)
    {
        if (src.size() != 4)
            return false;
        
        for (auto& it : src)
        {
            switch (it)
            {
            case '1':
                dest->push_back('7');
                break;
            case '2':
                dest->push_back('8');
                break;
            case '3':
                dest->push_back('9');
                break;
            case '7':
                dest->push_back('1');
                break;
            case '8':
                dest->push_back('2');
                break;
            case '9':
                dest->push_back('3');
                break;
            default:
                dest->push_back(it);
                break;
            }
        }
        return true;
    }

    bool IpProxysToGridData(const std::vector<IpProxy>& ipproxys,
                            GridData* griddata)
    {
        for (const auto& ipproxy : ipproxys)
        {
            RowData rowdata;
            std::wstring proxytype = base::UintToString16(static_cast<uint32>(ipproxy.GetProxyType()));
            std::wstring proxyip = base::UTF8ToWide(ipproxy.GetProxyIp());
            std::wstring proxyport = base::UintToString16(static_cast<uint32>(ipproxy.GetProxyPort()));
            rowdata.push_back(proxytype);
            rowdata.push_back(proxyip);
            rowdata.push_back(proxyport);
            griddata->push_back(rowdata);
        }
        return true;
    }
}
// DlgRegister 对话框

IMPLEMENT_DYNAMIC(CDlgRegister, CDialogEx)

CDlgRegister::CDlgRegister(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDlgRegister::IDD, pParent)
    , infoListCount_(0)
    , font18_(new CFont)
{
    registerHelper_.reset(new RegisterHelper);
    registerHelper_->Initialize();

    font18_->CreateFont(18,                        // nHeight
        0,                         // nWidth
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

}

CDlgRegister::~CDlgRegister()
{
    registerHelper_->Finalize();
}

void CDlgRegister::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_REGISTER_NAME, m_register_username);
    DDX_Control(pDX, IDC_EDIT_REGISTER_PASSWORD, m_register_password);
    DDX_Control(pDX, IDC_EDIT_VERIFY_CODE, m_register_verifycode);
    DDX_Control(pDX, IDC_STATIC_VERIFYCODE, m_static_verifycode);
    DDX_Control(pDX, IDC_LIST_REGISTER_INFO, m_register_info_list);
    DDX_Control(pDX, IDC_LIST_IP_PROXY, m_listctrl_ip_proxy);
}

void CDlgRegister::OnOK()
{
    // 快捷键盘功能：注册用户
    OnBnClickedBtnRegister();
    // 直接设置下一个用户名字和密码       
    m_register_username.SetWindowTextW(registerHelper_->GetNewName().c_str());
    m_register_password.SetWindowTextW(registerHelper_->GetPassword().c_str());
    // 直接显示新的验证码
    OnBnClickedBtnVerifyCode();
    return;
}

BEGIN_MESSAGE_MAP(CDlgRegister, CDialogEx)
    ON_WM_PAINT()
    ON_MESSAGE(WM_HOTKEY, OnHotKey)//注册热键
    ON_BN_CLICKED(IDC_BTN_CHECK_EXIST, &CDlgRegister::OnBnClickedBtnCheckExist)
    ON_BN_CLICKED(IDC_BTN_REGISTER, &CDlgRegister::OnBnClickedBtnRegister)
    ON_BN_CLICKED(IDC_BTN_VERIFY_CODE, &CDlgRegister::OnBnClickedBtnVerifyCode)
    ON_MESSAGE(WM_USER_REGISTER_INFO, &CDlgRegister::OnNotifyMessage)
    ON_BN_CLICKED(IDC_BTN_IMPORT_PROXY, &CDlgRegister::OnBnClickedBtnImportProxy)
    ON_BN_CLICKED(IDC_BTN_CHANGE_PROXY, &CDlgRegister::OnBnClickedBtnChangeProxy)
END_MESSAGE_MAP()


// DlgRegister 消息处理程序


BOOL CDlgRegister::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_register_verifycode.SetFont(font18_.get(),FALSE);
    //RegisterHotKey(this->m_hWnd, 1001, 0, VK_F5);
    //RegisterHotKey(this->m_hWnd, 1002, 0, VK_RETURN);
    DWORD dwStyle = m_listctrl_ip_proxy.GetExtendedStyle();
    //dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）

    m_listctrl_ip_proxy.SetExtendedStyle(dwStyle); //设置扩展风格
    int nColumnCount = m_listctrl_ip_proxy.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_listctrl_ip_proxy.DeleteColumn(i);
    uint32 index = 0;
    for (const auto& it : proxycolumnlist)
        m_listctrl_ip_proxy.InsertColumn(index++, it, LVCFMT_LEFT, 70);//插入列

    m_register_username.SetWindowTextW(registerHelper_->GetNewName().c_str());
    m_register_password.SetWindowTextW(registerHelper_->GetPassword().c_str());
    OnBnClickedBtnVerifyCode();
    
    return TRUE;
}

void CDlgRegister::OnPaint()
{
    if (!image.IsNull())
    {
        int hight = image.GetHeight();
        int width = image.GetWidth();
        CRect rc;
        m_static_verifycode.GetWindowRect(&rc);
        ScreenToClient(rc);
        image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 30, rc.left + width + 20,
            rc.top + hight + 30));
        m_register_verifycode.SetFocus();
    }
    CDialogEx::OnPaint();
}

LRESULT CDlgRegister::OnHotKey(WPARAM wp, LPARAM lp)
{
    UINT vk = (UINT)(wp);
    switch (vk)
    {
    case 1001:
        m_register_verifycode.SetWindowTextW(L"");
        OnBnClickedBtnVerifyCode(); 
        break;
    case 1002:
        // 快捷键盘功能：注册用户
        OnBnClickedBtnRegister();
        // 直接设置下一个用户名字和密码       
        m_register_username.SetWindowTextW(registerHelper_->GetNewName().c_str());
        m_register_password.SetWindowTextW(registerHelper_->GetPassword().c_str());
        // 直接显示新的验证码
        OnBnClickedBtnVerifyCode();
        break;
    default:
        break;
    }
    return S_OK;
}


LRESULT CDlgRegister::OnNotifyMessage(WPARAM wParam, LPARAM lParam)
{
    std::vector<std::wstring> messages;
    messageMutex_.lock();
    messages.swap(messageQueen_);
    messageMutex_.unlock();

    for (auto str : messages)
    {
        m_register_info_list.InsertString(infoListCount_++, str.c_str());
    }

    m_register_info_list.SetCurSel(infoListCount_ - 1);
    return 0;
}

void CDlgRegister::Notify(const std::wstring& message)
{
    // 发送数据给窗口
    messageMutex_.lock();
    messageQueen_.push_back(message);
    messageMutex_.unlock();
    this->PostMessage(WM_USER_REGISTER_INFO, 0, 0);
}

void CDlgRegister::OnBnClickedBtnCheckExist()
{
    CString username;
    m_register_username.GetWindowTextW(username);
    bool result = registerHelper_->RegisterCheckUserExist(username.GetBuffer());
    if (!result)
    {
        Notify(L"网络出错或用户已经存在!");
        return;
    }
    Notify(L"用户名可用");
}

void CDlgRegister::OnBnClickedBtnRegister()
{
    CString username;
    CString password;
    CString verifycode;

    m_register_username.GetWindowTextW(username);
    m_register_password.GetWindowTextW(password);
    m_register_verifycode.GetWindowTextW(verifycode);

    if (username.IsEmpty() || password.IsEmpty() || verifycode.IsEmpty())
    {
        Notify(L"输入的注册信息不完整");
        AfxMessageBox(L"请输入完整注册信息");
        return;
    }

    std::string cookies;
    std::string verifystr;
    std::wstring errorMsg;
    TranslateVerifyCode(base::WideToUTF8(verifycode.GetString()), &verifystr);
    if (!registerHelper_->RegisterUser(username.GetString(),
        password.GetString(), verifystr, &cookies, &errorMsg))
    {
        Notify(errorMsg + L" 注册失败");
        return;
    }
    Notify(L"注册成功");

    if (!registerHelper_->SaveAccountToFile(username.GetString(), 
        password.GetString(), cookies))
    {
        Notify(L"保存注册信息到文件失败!");
    }
    Notify(L"保存注册信息到文件成功!");
}


void CDlgRegister::OnBnClickedBtnVerifyCode()
{
    std::vector<uint8> picture;
    registerHelper_->RegisterGetVerifyCode(&picture);
    std::wstring pathname;
    if (!registerHelper_->SaveVerifyCodeImage(picture, &pathname))
    {
        assert(false);
        return;
    }

    if (!image.IsNull())
    {
        image.Destroy();
    }

    image.Load(pathname.c_str());
    int hight = image.GetHeight();
    int width = image.GetWidth();
    CRect rc;
    m_static_verifycode.GetWindowRect(&rc);
    ScreenToClient(rc);
    image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 30, rc.left + width +20, 
        rc.top + hight + 30));
    m_register_verifycode.SetWindowTextW(L"");
}

void CDlgRegister::OnBnClickedBtnImportProxy()
{
    ipProxys_.clear();
    if (!registerHelper_->LoadIpProxy(&ipProxys_))
        return;

    GridData griddata;
    IpProxysToGridData(ipProxys_, &griddata);

    // 显示在界面

    int itemcount = m_listctrl_ip_proxy.GetItemCount();

    for (uint32 i = 0; i < griddata.size(); ++i)
    {
        bool exist = false;
        // 检测是否存在相同用户id
        for (int index = 0; index < itemcount; index++)
        {
            CString text = m_listctrl_ip_proxy.GetItemText(index, 1);
            if (griddata[i][1].compare(text.GetBuffer()) == 0) // 相同用户id
            {
                exist = true;
                break;
            }
        }

        if (!exist) // 如果不存在，需要插入新数据
        {
            int nitem = m_listctrl_ip_proxy.InsertItem(itemcount + i, griddata[i][0].c_str());
            for (uint32 j = 0; j < griddata[i].size(); ++j)
            {
                m_listctrl_ip_proxy.SetItemText(nitem, j, griddata[i][j].c_str());
            }
        }
    }
}

void CDlgRegister::OnBnClickedBtnChangeProxy()
{
    m_listctrl_ip_proxy.GetFirstSelectedItemPosition();
}
