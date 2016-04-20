#include "stdafx.h"
#include "UserController.h"
#include "Network/User.h"

UserController::UserController()
{
}


UserController::~UserController()
{
}

bool UserController::AddUser(const std::string& username, 
    const std::string& password)
{
    std::shared_ptr<User> shared_user(new User(username, password));
    bool result = shared_user->Login();
    users_.push_back(shared_user);
    return true;
}
bool UserController::FillRoom(uint32 roomid, uint32 count)
{
    //assert(count < users_.size());
    for (const auto& it:users_)
    {
        it->EnterRoom(roomid);
    }
    return true;
}


void UserController::Run()
{
}
