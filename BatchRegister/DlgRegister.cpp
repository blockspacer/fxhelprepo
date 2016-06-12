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
    DDX_Control(pDX, IDC_CHK_USE_PROX, m_use_proxy);
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
    ON_NOTIFY(NM_CUSTOMDRAW, IDC_LIST_IP_PROXY, &CDlgRegister::OnNMCustomdrawListIpProxy)
    ON_BN_CLICKED(IDC_BTN_ADD_PROXY, &CDlgRegister::OnBnClickedBtnAddProxy)
    ON_NOTIFY(NM_CLICK, IDC_LIST_IP_PROXY, &CDlgRegister::OnNMClickListIpProxy)
    ON_BN_CLICKED(IDC_BTN_SELECT_ALL, &CDlgRegister::OnBnClickedBtnSelectAll)
    ON_BN_CLICKED(IDC_BTN_SELECT_REVERSE, &CDlgRegister::OnBnClickedBtnSelectReverse)
    ON_BN_CLICKED(IDC_BTN_REMOVE_SELECT, &CDlgRegister::OnBnClickedBtnRemoveSelect)
    ON_BN_CLICKED(IDC_BTN_SAVE_PROXY, &CDlgRegister::OnBnClickedBtnSaveProxy)
    ON_BN_CLICKED(IDC_BTN_CHK_PROXY, &CDlgRegister::OnBnClickedBtnChkProxy)
END_MESSAGE_MAP()


// DlgRegister 消息处理程序


BOOL CDlgRegister::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_register_verifycode.SetFont(font18_.get(),FALSE);
    //RegisterHotKey(this->m_hWnd, 1001, 0, VK_F5);
    //RegisterHotKey(this->m_hWnd, 1002, 0, VK_RETURN);
    DWORD dwStyle = m_listctrl_ip_proxy.GetExtendedStyle();
    dwStyle |= LVS_EX_CHECKBOXES;
    dwStyle |= LVS_EX_FULLROWSELECT;//选中某行使整行高亮（只适用与report风格的listctrl）
    dwStyle |= LVS_EX_GRIDLINES;//网格线（只适用与report风格的listctrl）

    m_listctrl_ip_proxy.SetExtendedStyle(dwStyle); //设置扩展风格
    int nColumnCount = m_listctrl_ip_proxy.GetHeaderCtrl()->GetItemCount();
    for (int i = nColumnCount - 1; i >= 0; i--)
        m_listctrl_ip_proxy.DeleteColumn(i);
    uint32 index = 0;
    for (const auto& it : proxycolumnlist)
        m_listctrl_ip_proxy.InsertColumn(index++, it, LVCFMT_LEFT, 120);//插入列

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
        image.Draw(GetDC()->m_hDC, CRect(rc.left + 20, rc.top + 18, rc.left + width + 20,
            rc.top + hight + 18));
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
    IpProxy ipProxy;
    if (m_use_proxy.GetCheck())
    {
        GetIpProxy(&ipProxy);
    }
    CString username;
    m_register_username.GetWindowTextW(username);
    bool result = registerHelper_->RegisterCheckUserExist(ipProxy,
        username.GetBuffer());
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


    IpProxy ipProxy;
    if (m_use_proxy.GetCheck())
    {
        GetIpProxy(&ipProxy);
    }

    std::string cookies;
    std::string verifystr;
    std::wstring errorMsg;
    TranslateVerifyCode(base::WideToUTF8(verifycode.GetString()), &verifystr);
    if (!registerHelper_->RegisterUser(ipProxy, username.GetString(),
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
    IpProxy ipProxy;
    if (m_use_proxy.GetCheck())
    {
        GetIpProxy(&ipProxy);
    }

    std::vector<uint8> picture;
    if (!registerHelper_->RegisterGetVerifyCode(ipProxy, &picture))
    {
        Notify(L"获取验证码失败");
        return;
    }
    
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

void CDlgRegister::OnNMCustomdrawListIpProxy(NMHDR *pNMHDR, LRESULT *pResult)
{
    NMLVCUSTOMDRAW* pNMCD = reinterpret_cast<NMLVCUSTOMDRAW*>(pNMHDR);

    *pResult = CDRF_DODEFAULT;

    if (CDDS_PREPAINT == pNMCD->nmcd.dwDrawStage)
    {
        *pResult = CDRF_NOTIFYITEMDRAW;
    }
    else if (CDDS_ITEMPREPAINT == pNMCD->nmcd.dwDrawStage)
    {
        *pResult = CDRF_NOTIFYSUBITEMDRAW;
    }
    else if ((CDDS_ITEMPREPAINT | CDDS_SUBITEM) == pNMCD->nmcd.dwDrawStage)
    {

        COLORREF clrNewTextColor, clrNewBkColor;

        int nItem = static_cast<int>(pNMCD->nmcd.dwItemSpec);

        POSITION pos = m_listctrl_ip_proxy.GetFirstSelectedItemPosition();
        int index = m_listctrl_ip_proxy.GetNextSelectedItem(pos);

        if (index == nItem)//如果要刷新的项为当前选择的项，则将文字设为白色，背景色设为蓝色
        {
            clrNewTextColor = RGB(255, 255, 255);		//Set the text to white
            clrNewBkColor = RGB(49, 106, 197);		//Set the background color to blue
        }
        else
        {
            clrNewTextColor = RGB(0, 0, 0);		//set the text black
            clrNewBkColor = RGB(255, 255, 255);	//leave the background color white
        }

        pNMCD->clrText = clrNewTextColor;
        pNMCD->clrTextBk = clrNewBkColor;

        *pResult = CDRF_DODEFAULT;
    }
}


void CDlgRegister::OnBnClickedBtnAddProxy()
{
    CString csProxyType;
    GetDlgItemText(IDC_EDIT_PROXY_TYPE, csProxyType);
    CString csIP;
    GetDlgItemText(IDC_EDIT_IP, csIP);
    CString csPort;
    GetDlgItemText(IDC_EDIT_PORT, csPort);

    GridData griddata;
    RowData rowdata;
    rowdata.push_back(csProxyType.GetBuffer());
    rowdata.push_back(csIP.GetBuffer());
    rowdata.push_back(csPort.GetBuffer());
    griddata.push_back(rowdata);

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


void CDlgRegister::OnNMClickListIpProxy(NMHDR *pNMHDR, LRESULT *pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    POSITION pos = m_listctrl_ip_proxy.GetFirstSelectedItemPosition();
    if (pos == NULL)
    {
        TRACE0("No items were selected!\n");
        return;
    }
    else
    {
        while (pos)
        {
            int nItem = m_listctrl_ip_proxy.GetNextSelectedItem(pos);
            TRACE1("Item %d was selected!\n", nItem);
            // you could do your own processing on nItem here
            CString csProxyType = m_listctrl_ip_proxy.GetItemText(nItem, 0);
            CString csIP = m_listctrl_ip_proxy.GetItemText(nItem, 1);
            CString csPort = m_listctrl_ip_proxy.GetItemText(nItem, 2);
            SetDlgItemText(IDC_EDIT_PROXY_TYPE, csProxyType);
            SetDlgItemText(IDC_EDIT_IP, csIP);
            SetDlgItemText(IDC_EDIT_PORT, csPort);
        }
    }

    *pResult = 0;
}


void CDlgRegister::OnBnClickedBtnSelectAll()
{
    int count = m_listctrl_ip_proxy.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        m_listctrl_ip_proxy.SetCheck(i, 1);
    }
}


void CDlgRegister::OnBnClickedBtnSelectReverse()
{
    int count = m_listctrl_ip_proxy.GetItemCount();

    for (int i = count - 1; i >= 0; --i)
    {
        if (m_listctrl_ip_proxy.GetCheck(i))
        {
            m_listctrl_ip_proxy.SetCheck(i, FALSE);
        }
        else
        {
            m_listctrl_ip_proxy.SetCheck(i, TRUE);
        }
    }
}


void CDlgRegister::OnBnClickedBtnRemoveSelect()
{
    int count = m_listctrl_ip_proxy.GetItemCount();
    // 从后往前删除
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_listctrl_ip_proxy.GetCheck(i))
        {
            // 把要删除的消息发到日志记录列表上
            CString itemtext = m_listctrl_ip_proxy.GetItemText(i, 1);
            itemtext += L"被从IP代理列表中删除";
            Notify(itemtext.GetBuffer());

            // 删除已经勾选的记录
            m_listctrl_ip_proxy.DeleteItem(i);
        }
    }
}


void CDlgRegister::OnBnClickedBtnSaveProxy()
{
    std::vector<IpProxy> ipproxys;

    int count = m_listctrl_ip_proxy.GetItemCount();
    for (int i = 0; i < count; ++i)
    {
        CString csProxyType = m_listctrl_ip_proxy.GetItemText(i, 0);
        CString csIP = m_listctrl_ip_proxy.GetItemText(i, 1);
        CString csPort = m_listctrl_ip_proxy.GetItemText(i, 2);
        uint32 proxytype;
        if (!base::StringToUint(base::WideToUTF8(csProxyType.GetBuffer()), &proxytype))
            continue;

        std::string ip = base::WideToUTF8(csIP.GetBuffer());
        uint32 port;
        if (!base::StringToUint(base::WideToUTF8(csPort.GetBuffer()), &port))
            continue;

        IpProxy ipproxy;
        ipproxy.SetProxyType(static_cast<IpProxy::PROXY_TYPE>(proxytype));
        ipproxy.SetProxyIp(ip);
        ipproxy.SetProxyPort(static_cast<uint16>(port));
        ipproxys.push_back(ipproxy);
    }

    if (!registerHelper_->SaveIpProxy(ipproxys))
    {
        Notify(L"保存代理信息失败");
        return;
    }
    Notify(L"保存代理信息成功");
}

bool CDlgRegister::GetIpProxy(IpProxy* ipproxy)
{
    // 获取代理信息
    CString csProxyType;
    GetDlgItemText(IDC_EDIT_PROXY_TYPE, csProxyType);
    CString csIP;
    GetDlgItemText(IDC_EDIT_IP, csIP);
    CString csPort;
    GetDlgItemText(IDC_EDIT_PORT, csPort);
    uint32 proxytype;
    if (!base::StringToUint(base::WideToUTF8(csProxyType.GetBuffer()), &proxytype))
    {
        MessageBox(L"代理类型错误", L"错误", 0);
        return false;
    }

    std::string ip = base::WideToUTF8(csIP.GetBuffer());
    uint32 port;
    if (!base::StringToUint(base::WideToUTF8(csPort.GetBuffer()), &port))
    {
        MessageBox(L"代理端口数据错误", L"错误", 0);
        return false;
    }
    ipproxy->SetProxyType(static_cast<IpProxy::PROXY_TYPE>(proxytype));
    ipproxy->SetProxyIp(ip);
    ipproxy->SetProxyPort(static_cast<uint16>(port));

    Notify(L"使用代理信息" +
        std::wstring(csIP.GetBuffer()) + L":" +
        std::wstring(csPort.GetBuffer()));
    return true;
}

void CDlgRegister::OnBnClickedBtnChkProxy()
{
    std::vector<IpProxy> ipproxys;
    uint32 aviliable = 0;
    int count = m_listctrl_ip_proxy.GetItemCount();
    for (int i = 0; i < count; ++i)
    {
        CString csProxyType = m_listctrl_ip_proxy.GetItemText(i, 0);
        CString csIP = m_listctrl_ip_proxy.GetItemText(i, 1);
        CString csPort = m_listctrl_ip_proxy.GetItemText(i, 2);
        uint32 proxytype;
        if (!base::StringToUint(base::WideToUTF8(csProxyType.GetBuffer()), &proxytype))
            continue;

        std::string ip = base::WideToUTF8(csIP.GetBuffer());
        uint32 port;
        if (!base::StringToUint(base::WideToUTF8(csPort.GetBuffer()), &port))
            continue;

        IpProxy ipproxy;
        ipproxy.SetProxyType(static_cast<IpProxy::PROXY_TYPE>(proxytype));
        ipproxy.SetProxyIp(ip);
        ipproxy.SetProxyPort(static_cast<uint16>(port));

        std::vector<uint8> picture;
        if (!registerHelper_->RegisterGetVerifyCode(ipproxy, &picture))
        {
            Notify(L"IP代理不可用" + std::wstring(csIP.GetBuffer()));
            m_listctrl_ip_proxy.SetCheck(i, TRUE);//失败的勾选上,然后要批量删除
            continue;
        }
        Notify(L"IP代理检测可用--" + std::wstring(csIP.GetBuffer()));
        aviliable++;
    }
    Notify(L"IP代理检测全部完成,有效IP数量: " + base::UintToString16(aviliable) 
        + L"/" + base::UintToString16(count));

    // 从后往前删除
    for (int i = count - 1; i >= 0; --i)
    {
        if (m_listctrl_ip_proxy.GetCheck(i))
        {
            // 把要删除的消息发到日志记录列表上
            CString itemtext = m_listctrl_ip_proxy.GetItemText(i, 1);
            itemtext += L"无效,被从IP代理列表中删除";
            Notify(itemtext.GetBuffer());

            // 删除已经勾选的记录
            m_listctrl_ip_proxy.DeleteItem(i);
        }
    }
}
