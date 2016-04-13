#include "stdafx.h"
#include "UserController.h"
#include "Network/User.h"

UserController::UserController()
{
}


UserController::~UserController()
{
}

void UserController::Run()
{
    // 读取配置文件内容

    // 增加用户
    std::shared_ptr<User> user(new User);
    user->Login("fanxingtest001", "1233211234567");
    user->EnterRoom(1084594);
    users_.push_back(user);
}
