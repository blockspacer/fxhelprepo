// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "Network/Network.h"
#include "Network/TcpManager.h"
#include "third_party/chromium/base/at_exit.h"
#include "third_party/chromium/base/run_loop.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/command_line.h"
#include "third_party/chromium/base/at_exit.h"

extern void SingleUserSingleRoomTest(TcpManager* tcp_manager);
void InitAppLog();

int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;
    InitAppLog();
    NetworkInitialize();

    std::unique_ptr<TcpManager> tcp_manager(new TcpManager);
    tcp_manager->Initialize();
    SingleUserSingleRoomTest(tcp_manager.get());
    tcp_manager->Finalize();
    NetworkFainalize();
	return 0;
}

void InitAppLog()
{
    CommandLine::Init(0, NULL);
    base::FilePath path;
    PathService::Get(base::DIR_APP_DATA, &path);
    path = path.Append(L"FanXingHelper").Append(L"fanxinghelper.log");
    logging::LoggingSettings setting;
#ifdef _DEBUG
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

