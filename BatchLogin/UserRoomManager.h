#pragma once
#include <string>
#include <functional>
#include "UserController.h"
#include "RoomController.h"
#include "Network/IpProxy.h"
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
    bool SaveUserLoginConfig();

    bool LoadIpProxy(GridData* proxyinfo);

    bool BatchLogUsers(const std::map<std::wstring, std::wstring>& userAccountPassword);
    bool BatchLogUsersWithCookie(const std::map<std::wstring, std::wstring>& accountCookie);
    
    bool FillRooms(const std::vector<std::wstring>& roomids);

    bool UpMVBillboard(const std::wstring& collectionid, const std::wstring& mvid);

protected:
    void DoSaveUserLoginConfig();
    void DoBatchLogUsers(const std::map<std::wstring, std::wstring>& userAccountPassword);
    void DoBatchLogUsersWithCookie(const std::map<std::wstring, std::wstring>& accountCookie);
    void DoFillRooms(const std::vector<uint32>& roomids);
    void FillSingleRoom(uint32 roomid);

    void DoUpMVBillboard(const std::wstring& collectionid, const std::wstring& mvid);

private:
    base::Thread workerThread_;
    std::function<void(std::wstring)> notify_ = nullptr;
    std::unique_ptr<UserController> userController_ = nullptr;
    std::unique_ptr<RoomController> roomController_ = nullptr;
    std::vector<IpProxy> ipProxys_;
};

