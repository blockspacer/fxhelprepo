#pragma once
#include <string>
#include <map>
#include <functional>
#include "UserController.h"
#include "RoomController.h"
#include "Network/IpProxy.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/memory/scoped_ptr.h"
#include "third_party/chromium/base/threading/thread.h"

class TcpClientController;
// 带线程控制
class UserRoomManager 
    :public std::enable_shared_from_this<UserRoomManager>
{
public:
    UserRoomManager(TcpClientController* tcpManager_);
    ~UserRoomManager();

    static void AddRef() {}
    static void Release() {}

    bool Initialize();
    bool Finalize();

    void SetNotify(std::function<void(std::wstring)> notify);

    bool LoadUserConfig(GridData* userpwd, uint32* total) const;
    bool LoadRoomConfig(GridData* roomgrid, uint32* total) const;
    bool SaveUserLoginConfig();

    bool LoadIpProxy(GridData* proxyinfo);

    bool BatchLogUsers(const std::map<std::wstring, std::wstring>& userAccountPassword);
    bool BatchLogUsersWithCookie(const std::map<std::wstring, std::wstring>& accountCookie);
    
    bool FillRooms(const std::vector<std::wstring>& roomids);

    bool UpMVBillboard(const std::wstring& collectionid, const std::wstring& mvid);

    bool RealSingLike(const std::vector<std::wstring>& users, const std::wstring& room_id,
        const std::wstring& song_name, const std::wstring& delta);

    bool SendGifts(const std::vector<std::wstring>& users, const std::wstring& room_id,
        uint32 gift_id, uint32 gift_count);

    bool RobVotes(const std::vector<std::wstring>& users, const std::wstring& room_id);

    bool GetUserStorageInfos(const std::vector<std::wstring>& users,
        std::vector<UserStorageInfo>* user_storage_infos);

    bool BatchChangeNickname(const std::vector<std::wstring>& users,
        const std::wstring& nickname_pre);

    bool BatchChangeNicknameList(const std::vector<std::wstring>& users,
        const std::vector<std::wstring>& nickname_list);

    bool BatchChangeLogo(const std::vector<std::wstring>& users,
        const std::wstring& logo_path);

    void SetBreakRequest(bool interrupt);

    bool BatchSendChat(const std::wstring& roomid, const std::vector<std::wstring>& users,
        const std::wstring& message);

protected:
    void Notify(const std::wstring& msg);
    void DoSaveUserLoginConfig();
    void DoBatchLogUsers(const std::map<std::wstring, std::wstring>& userAccountPassword);
    void DoBatchLogUsersWithCookie(const std::map<std::wstring, std::wstring>& accountCookie);
    void DoFillRooms(const std::vector<uint32>& roomids);
    void FillSingleRoom(uint32 roomid);

    void DoUpMVBillboard(const std::wstring& collectionid, const std::wstring& mvid);
    void DoRealSingLike(const std::string& account,
        uint32 room_id, const std::wstring& song_name);

    void DoSendGifts(const std::vector<std::wstring>& users,
        uint32 roomid, uint32 gift_id, uint32 gift_count);
    void DoRobVotes(const std::vector<std::wstring>& users, uint32 roomid);
    void DoBatchChangeNickname(const std::vector<std::wstring>& users,
        const std::wstring& nickname_pre);
    void DoBatchChangeLogo(const std::vector<std::wstring>& users,
        const std::wstring& logo_path);

    void DoBatchSendChat(uint32 roomid, const std::vector<std::string>& users, 
        const std::string& message);

private:
    base::Thread workerThread_;
    scoped_refptr<base::TaskRunner> runner_;
    std::function<void(std::wstring)> notify_ = nullptr;
    std::unique_ptr<UserController> userController_ = nullptr;
    std::unique_ptr<RoomController> roomController_ = nullptr;
    std::map<std::string, IpProxy> ipProxys_;
    bool break_request_ = false;
};

