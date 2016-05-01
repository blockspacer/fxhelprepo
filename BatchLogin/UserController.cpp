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
    if (!shared_user->Login())
    {
        assert(false && L"µÇÂ¼Ê§°Ü");
        return false;
    }
    
    users_.push_back(shared_user);
    return true;
}
bool UserController::FillRoom(uint32 roomid, uint32 count)
{
    for (const auto& it : users_)
    {
        it->EnterRoom(roomid);
    }
    return true;
}


void UserController::Run()
{
}
