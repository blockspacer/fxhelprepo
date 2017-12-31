
// VmpHWIDDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VmpHWID.h"
#include "VmpHWIDDlg.h"
#include "afxdialogex.h"
#include <string>
#include <memory>
#include "VMProtect/VMProtectSDK.h"

#undef max
#undef min
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace{

    bool GetCurrentHWID(std::string* hwid)
    {
        //VMProtectBegin("getcurrenthwid");

        int nSize = VMProtectGetCurrentHWID(NULL, 0); // get number of required bytes
        std::unique_ptr<char[]> pBuf(new char[nSize]); // allocate buffer
        if (nSize != VMProtectGetCurrentHWID(pBuf.get(), nSize)) // obtain hardeare identifier
            return false;

        hwid->assign(pBuf.get(), pBuf.get() + nSize);
        //VMProtectEnd();
        return true;
    }

    bool GetSerialKey(std::string* serial_key)
    {
        base::FilePath path;
        PathService::Get(base::DIR_EXE, &path);

        path = path.Append(L"hd_serial.ini");
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

        return true;
    }

}

// CAboutDlg dialog used for App About

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
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


// CVmpHWIDDlg dialog



CVmpHWIDDlg::CVmpHWIDDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CVmpHWIDDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVmpHWIDDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDIT1, m_edit_hwid);
}

BEGIN_MESSAGE_MAP(CVmpHWIDDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDOK, &CVmpHWIDDlg::OnBnClickedOk)
END_MESSAGE_MAP()

// CVmpHWIDDlg message handlers

BOOL CVmpHWIDDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

    //std::string serial;
    //if (GetSerialKey(&serial))
    //{
    //    //VMProtectBegin("setserialkey");
    //    int res = VMProtectSetSerialNumber(serial.c_str());
    //    if (res)
    //    {
    //        MessageBox(L"授权码无效", L"验证结果", 0);
    //        printf("serial number is bad\n");
    //        return TRUE;
    //    }
    //    MessageBox(L"授权成功", L"验证结果", 0);
    //    //VMProtectEnd();
    //}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CVmpHWIDDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVmpHWIDDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVmpHWIDDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CVmpHWIDDlg::OnBnClickedOk()
{
    std::string hwid;
    if (!GetCurrentHWID(&hwid))
    {
        MessageBox(L"获取机器码错误", L"错误", 0);
        return;
    }
    CString cs_hwid = base::UTF8ToWide(hwid).c_str();
    m_edit_hwid.SetWindowTextW(cs_hwid);
}
