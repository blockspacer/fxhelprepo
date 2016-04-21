#include "stdafx.h"
#include "RoomController.h"


RoomController::RoomController()
{
}


RoomController::~RoomController()
{
}

bool RoomController::AddRoom(uint32 roomid)
{
    roomids_.push_back(roomid);
    return true;
}

std::vector<uint32> RoomController::GetRooms() const
{
    return roomids_;
}
