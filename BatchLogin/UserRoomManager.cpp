#include "stdafx.h"
#include <string>
#include <map>
#include <set>
#include "UserRoomManager.h"

#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace
{
    const uint32 roomusercount = 200;
}

UserRoomManager::UserRoomManager()
    :userController_(new UserController)
    , roomController_(new RoomController)
{
}


UserRoomManager::~UserRoomManager()
{
}

bool UserRoomManager::LoadUserConfig()
{
    // ╤анд╪Ч
    std::map<std::wstring, std::wstring> usermap;
    for (const auto& it: usermap)
    {
        std::string username = base::WideToUTF8(it.first);
        std::string password = base::WideToUTF8(it.second);
        userController_->AddUser(username, password);
    }
    
    return false;
}

bool UserRoomManager::LoadRoomConfig()
{
    std::set<uint32> roomset;
    for (const auto& it : roomset)
    {
        AddRoom(it);
    }
    return true;
}

bool UserRoomManager::AddRoom(uint32 roomid)
{
    roomController_->AddRoom(roomid);
    userController_->FillRoom(roomid, roomusercount);
    return false;
}