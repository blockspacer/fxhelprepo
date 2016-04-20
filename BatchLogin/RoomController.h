#pragma once
#include <string>
#include <vector>
#include "third_party/chromium/base/basictypes.h"

class RoomController
{
public:
    RoomController();
    ~RoomController();

    bool AddRoom(uint32 roomid);
};

