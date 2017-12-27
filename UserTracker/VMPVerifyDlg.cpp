// VMPVerifyDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "UserTracker.h"
#include "VMPVerifyDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <memory>
#include "VMProtect/VMProtectSDK.h"
#undef max
#undef min
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace{
    char *read_serial(const char *fname)
    {
        FILE *f;
        if (0 != fopen_s(&f, fname, "rb")) return NULL;
        fseek(f, 0, SEEK_END);
        int s = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *buf = new char[s + 1];
        fread(buf, s, 1, f);
        buf[s] = 0;
        fclose(f);
        return buf;
    }

    bool GetCurrentHWID(std::string* hwid)
    {
        int nSize = VMProtectGetCurrentHWID(NULL, 0); // get number of required bytes
        std::unique_ptr<char[]> pBuf(new char[nSize]); // allocate buffer
        if (nSize != VMProtectGetCurrentHWID(pBuf.get(), nSize)) // obtain hardeare identifier
            return false;

        hwid->assign(pBuf.get(), pBuf.get()+nSize);
        return true;
    }

    bool GetSerialKey(std::string* serial_key)
    {
        base::FilePath path;
        PathService::Get(base::DIR_EXE, &path);

        path = path.Append(L"serial.ini");
        if (!base::PathExists(path))
            return false;

        base::File file;
        file.Initialize(path, base::File::FLAG_OPEN | base::File::FLAG_READ);
        if (!file.IsValid())
            return false;

        const int read_size = 1024;
        std::unique_ptr<char[]> buffer(new char[read_size]);
        int read_count = 0;
        read_count = file.ReadAtCurrentPos(buffer.get(), read_size);
        while (read_count)
        {
            serial_key->append(buffer.get(), read_count);
            read_count = file.ReadAtCurrentPos(buffer.get(), read_size);
        }

        //std::string serial()
        //std::string utf8_path = base::WideToUTF8(path.value());

        return true;
    }
}
// CVMPVerifyDlg 对话框

IMPLEMENT_DYNAMIC(CVMPVerifyDlg, CDialogEx)

CVMPVerifyDlg::CVMPVerifyDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVMPVerifyDlg::IDD, pParent)
{

}

CVMPVerifyDlg::~CVMPVerifyDlg()
{
}

void CVMPVerifyDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT_HARDWARE_CODE, m_edit_hardware_code);
    DDX_Control(pDX, IDC_EDIT_VMP_KEY, m_edit_authority_key);
}

BOOL CVMPVerifyDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    std::string hwid;
    if (!GetCurrentHWID(&hwid))
    {
        MessageBox(L"获取机器码错误", L"错误", 0);
        return FALSE;
    }
    CString cs_hwid = base::UTF8ToWide(hwid).c_str();
    m_edit_hardware_code.SetWindowTextW(cs_hwid);
    
    if (GetSerialKey(&serial_))
    {
        CString cs_serial = base::UTF8ToWide(serial_).c_str();
        m_edit_authority_key.SetWindowTextW(cs_serial);
    }

    return TRUE;
}

BEGIN_MESSAGE_MAP(CVMPVerifyDlg, CDialogEx)
    ON_BN_CLICKED(IDOK, &CVMPVerifyDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CVMPVerifyDlg 消息处理程序


void CVMPVerifyDlg::OnBnClickedOk()
{
    CString cs_serial;
    m_edit_authority_key.GetWindowTextW(cs_serial);
    VMProtectBegin("thisistest");
    if (cs_serial.GetLength()==0)
    {
        MessageBox(L"请输入授权码", L"错误", 0);
        return;
    }
    int res = VMProtectSetSerialNumber(serial_.c_str());
    if (res)
    {
        MessageBox(L"授权码无效", L"验证结果", 0);
        printf("serial number is bad\n");
        return;
    }
    MessageBox(L"授权成功", L"验证结果", 0);
    VMProtectEnd();
    CDialogEx::OnOK();
}
