#pragma once
#include <string>
#include <functional>
#include "UserController.h"
#include "RoomController.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/threading/thread.h"


// 带线程控制
class UserRoomManager 
    :public std::enable_shared_from_this<UserRoomManager>
{
public:
    UserRoomManager();
    ~UserRoomManager();

    static void AddRef() {}
    static void Release() {}

    bool Initialize();
    bool Finalize();

    void SetNotify(std::function<void(std::wstring)> notify);

    bool LoadUserConfig(GridData* userpwd, uint32* total);
    bool LoadRoomConfig(GridData* roomgrid, uint32* total);

    bool BatchLogUsers(const std::map<std::wstring, std::wstring>& userAccountPassword);
    
    bool FillRooms(const std::vector<std::wstring>& roomids);

protected:
    void DoBatchLogUsers(const std::map<std::wstring, std::wstring>& userAccountPassword);
    void DoFillRooms(const std::vector<uint32>& roomids);
    void FillSingleRoom(uint32 roomid);

private:
    base::Thread workerThread_;
    std::function<void(std::wstring)> notify_ = nullptr;
    std::unique_ptr<UserController> userController_ = nullptr;
    std::unique_ptr<RoomController> roomController_ = nullptr;
};

