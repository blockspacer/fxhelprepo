
// UserTracker.cpp : 定义应用程序的类行为。
//

#include "stdafx.h"
#include <memory>

#undef max
#undef min
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/command_line.h"
#include "third_party/chromium/base/at_exit.h"

#include "UserTrackerHelper.h"
#include "UserTracker.h"
#include "UserTrackerDlg.h"
#include "LoginDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUserTrackerApp

BEGIN_MESSAGE_MAP(CUserTrackerApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CUserTrackerApp 构造

CUserTrackerApp::CUserTrackerApp()
    :atExitManager_(nullptr)
{
	// TODO:  在此处添加构造代码，
	// 将所有重要的初始化放置在 InitInstance 中
    atExitManager_.reset(new base::AtExitManager);
    InitAppLog();
    LOG(INFO) << __FUNCTION__;
}


// 唯一的一个 CUserTrackerApp 对象

CUserTrackerApp theApp;


// CUserTrackerApp 初始化

BOOL CUserTrackerApp::InitInstance()
{
	// 如果一个运行在 Windows XP 上的应用程序清单指定要
	// 使用 ComCtl32.dll 版本 6 或更高版本来启用可视化方式，
	//则需要 InitCommonControlsEx()。  否则，将无法创建窗口。
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// 将它设置为包括所有要在应用程序中使用的
	// 公共控件类。
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();


	AfxEnableControlContainer();

	// 创建 shell 管理器，以防对话框包含
	// 任何 shell 树视图控件或 shell 列表视图控件。
	CShellManager *pShellManager = new CShellManager;

	// 激活“Windows Native”视觉管理器，以便在 MFC 控件中启用主题
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	// 标准初始化
	// 如果未使用这些功能并希望减小
	// 最终可执行文件的大小，则应移除下列
	// 不需要的特定初始化例程
	// 更改用于存储设置的注册表项
	// TODO:  应适当修改该字符串，
	// 例如修改为公司或组织名
	SetRegistryKey(_T("应用程序向导生成的本地应用程序"));

    UserTrackerHelper tracker_helper;
    tracker_helper.Initialize();

    if (LoginProcedure(&tracker_helper))
    {
        TrackerProcedure(&tracker_helper);
    }
    tracker_helper.Finalize();

	// 删除上面创建的 shell 管理器。
	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

    
	// 由于对话框已关闭，所以将返回 FALSE 以便退出应用程序，
	//  而不是启动应用程序的消息泵。
	return FALSE;
}

bool CUserTrackerApp::LoginProcedure(UserTrackerHelper* tracker_helper)
{
    CLoginDlg login_dlg(NULL, tracker_helper);
    INT_PTR nResponse = login_dlg.DoModal();
    if (nResponse == IDOK)
    {
        return true;
    }
    else if (nResponse == IDCANCEL)
    {
        return false;
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
        TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
    }
    return false;
}

bool CUserTrackerApp::TrackerProcedure(UserTrackerHelper* tracker_helper)
{
    CUserTrackerDlg tracker_dlg(NULL, tracker_helper);
    m_pMainWnd = &tracker_dlg;
    INT_PTR nResponse = tracker_dlg.DoModal();
    if (nResponse == IDOK)
    {
        return true;
    }
    else if (nResponse == IDCANCEL)
    {
        return false;
    }
    else if (nResponse == -1)
    {
        TRACE(traceAppMsg, 0, "警告: 对话框创建失败，应用程序将意外终止。\n");
        TRACE(traceAppMsg, 0, "警告: 如果您在对话框上使用 MFC 控件，则无法 #define _AFX_NO_MFC_CONTROLS_IN_DIALOGS。\n");
    }
    return false;
}

void CUserTrackerApp::InitAppLog()
{
    CommandLine::Init(0, NULL);
    base::FilePath path;
    PathService::Get(base::DIR_APP_DATA, &path);
    path = path.Append(L"FanXingHelper").Append(L"fanxinghelper.log");
    logging::LoggingSettings setting;
#ifdef DEBUG
    setting.logging_dest = logging::LOG_TO_ALL;
    setting.lock_log = logging::LOCK_LOG_FILE;
#else
    setting.logging_dest = logging::LOG_NONE;
    setting.lock_log = logging::DONT_LOCK_LOG_FILE;
#endif
    setting.log_file = path.value().c_str();
    setting.delete_old = logging::APPEND_TO_OLD_LOG_FILE;
    logging::InitLogging(setting);
    logging::SetLogItems(false, true, true, true);
}