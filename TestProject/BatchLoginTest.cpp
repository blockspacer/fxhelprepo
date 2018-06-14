#include "stdafx.h"
#include "Network/EncodeHelper.h"
#include "Network/User.h"
#include "Network/CurlWrapper.h"


void TestConnectBreakCallback()
{
}

void SingleUserSingleRoomTest(WebsocketClientController* tcp_manager)
{
    bool result = true;
    User user;
    std::string errormsg;
    user.SetWebsocketClientController(tcp_manager);
    result &= user.Login("fanxingtest002", "1233211234567","", &errormsg);
    result &= user.EnterRoomFopAlive(1201793, base::Bind(&TestConnectBreakCallback));

    while (1);
}


void SingleUserMultiRoomTest(std::shared_ptr<User> user)
{
    bool result = true;
    result &= user->Login();
    result &= user->EnterRoomFopOperation(1084594, nullptr,
        base::Bind(&TestConnectBreakCallback));
    result &= user->EnterRoomFopOperation(1053564, nullptr,
        base::Bind(&TestConnectBreakCallback));
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
        std::shared_ptr<User> shared_user(new User);
        shared_user->SetUsername(username);
        shared_user->SetPassword(password);
        userlist.push_back(shared_user);
        SingleUserMultiRoomTest(shared_user);
    }
    while (1);
}
//int _tmain(int argc, _TCHAR* argv[])
//{
//    base::AtExitManager atExitManager;
//
//    CurlWrapper::CurlInit();
//    //SingleUserSingleRoomTest();
//    MultiUserMultiRoomTest();
//
//    CurlWrapper::CurlCleanup();
//    return 0;
//}

