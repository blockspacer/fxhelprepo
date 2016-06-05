
// IpProxyAchieverDlg.cpp : 实现文件
//

#include "stdafx.h"
#include <regex>
#include "IpProxyAchiever.h"
#include "IpProxyAchieverDlg.h"
#include "afxdialogex.h"
#undef min
#undef max
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CIpProxyAchieverDlg 对话框



CIpProxyAchieverDlg::CIpProxyAchieverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIpProxyAchieverDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    CurlWrapper::CurlInit();
    curlWrapper_.reset(new CurlWrapper);
}

void CIpProxyAchieverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CIpProxyAchieverDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_GET_PROXY, &CIpProxyAchieverDlg::OnBnClickedBtnGetProxy)
END_MESSAGE_MAP()


// CIpProxyAchieverDlg 消息处理程序

BOOL CIpProxyAchieverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CIpProxyAchieverDlg::OnPaint()
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
HCURSOR CIpProxyAchieverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CIpProxyAchieverDlg::OnBnClickedBtnGetProxy()
{
    // TODO:  在此添加控件通知处理程序代码
    HttpRequest request;
    request.url = "http://www.youdaili.net/Daili/Socks/";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.useragent = "Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/44.0.2403.130 Safari/537.36";
    HttpResponse response;
    if (!curlWrapper_->Execute(request, &response))
        return ;

    std::string rootdata(response.content.begin(), response.content.end());
    std::regex pattern(R"(http://www\.youdaili.net/Daili/Socks/[0-9]*\.html)");

    std::string s = rootdata;
    std::smatch match;
    std::vector<std::string> urllist;
    while (std::regex_search(s, match, pattern)) {
        for (auto x : match)
            urllist.push_back(x);
        std::cout << std::endl;
        s = match.suffix().str();
    }

    return ;
}
