// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <assert.h>
#include "Network/User.h"
#include "Network/CurlWrapper.h"

#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/command_line.h"
#include "third_party/chromium/base/at_exit.h"



void SingleUserSingleRoomTest()
{
    bool result = true;
    User user;
    result &= user.Login("fanxingtest001", "123321");
    result &= user.EnterRoom(1084594);

    while (1);
}

void SingleUserMultiRoomTest(std::shared_ptr<User> user)
{
    bool result = true;
    result &= user->Login();
    result &= user->EnterRoom(1084594);
    result &= user->EnterRoom(1053564);
}

void MultiUserMultiRoomTest()
{
    std::vector<std::pair<std::string, std::string>> uservector;
    uservector.push_back(std::make_pair("fanxingtest001", "123321"));

    std::vector<std::shared_ptr<User>> userlist;
    for (const auto& it : uservector)
    {
        std::string username = it.first;
        std::string password = it.second;
        std::shared_ptr<User> shared_user(new User(username, password));
        userlist.push_back(shared_user);
        SingleUserMultiRoomTest(shared_user);
    }
    while (1);
}

void InitAppLog()
{
    CommandLine::Init(0, NULL);
    base::FilePath path;
    PathService::Get(base::DIR_APP_DATA, &path);
    path = path.Append(L"FanXingHelper").Append(L"fanxinghelper.log");
    logging::LoggingSettings setting;
    setting.logging_dest = logging::LOG_TO_ALL;
    setting.lock_log = logging::LOCK_LOG_FILE;
    setting.log_file = path.value().c_str();
    setting.delete_old = logging::APPEND_TO_OLD_LOG_FILE;
    logging::InitLogging(setting);
    logging::SetLogItems(false, true, true, true);
}

int _tmain(int argc, _TCHAR* argv[])
{
    base::AtExitManager atExitManager;
    CurlWrapper::CurlInit();
    //SingleUserSingleRoomTest();
    MultiUserMultiRoomTest();

    CurlWrapper::CurlCleanup();
	return 0;
}

