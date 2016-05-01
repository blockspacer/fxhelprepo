#pragma once
#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "Network/MessageNotifyManager.h"
#include "Network/CurlWrapper.h"
#include "AuthorityHelper.h"

class CurlWrapper;
class GiftNotifyManager;
class GiftInfoHelper;
class GiftInfo;
class User;

typedef std::function<void(const std::wstring&)> notifyfn;

//typedef std::vector<std::wstring> RowData;
//typedef std::vector<RowData> GridData;
typedef std::function<void(const RowData&)> notify201;
typedef std::function<void(const RowData&)> notify501;
typedef std::function<void(uint32,const std::wstring&)> notify502;
typedef std::function<void(const RoomGiftInfo601&, const GiftInfo&)> notify601;

class NetworkHelper
{
public:
    NetworkHelper();
    ~NetworkHelper();

    bool Initialize();// 启动线程
    void Finalize();// 结束线程

    void SetNotify(notifyfn fn);
    void RemoveNotify();

    void SetNotify201(notify201 fn);
    void RemoveNotify201();

    void SetNotify501(notify501 fn);
    void RemoveNotify501();

    void SetNotify502(notify502 fn);
    void RemoveNotify502();

    void SetNotify601(notify601 fn);
    void RemoveNotify601();

    bool Login(const std::wstring& username, const std::wstring& password);
    bool GetCurrentUserDisplay(std::wstring* display);
    bool EnterRoom(const std::wstring& roomid);
    bool EnterRoom(uint32 roomid);
    bool GetViewerList(uint32 roomid,
        std::vector<RowData>* enterRoomUserInfoRowdata);

    // 判断用户是否有操作权限，暂时实现为只有公会成员才能操作。
    bool GetActionPrivilege(std::wstring* message);

    bool KickoutUsers(KICK_TYPE kicktype, uint32 roomid, 
        const EnterRoomUserInfo& enterRoomUserInfo);
    bool BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);
    bool UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo);

private:
    void NotifyCallback(const std::wstring& message);
    void NotifyCallback601(uint32 roomid, uint32 singerid, const RoomGiftInfo601& roomgiftinfo);
    void NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo);
    void NotifyCallback501(const EnterRoomUserInfo& enterRoomUserInfo);

    std::map<uint32, EnterRoomUserInfo> enterRoomUserInfoMap_;
    notifyfn notify_;
    notify201 notify201_;
    notify201 notify501_;
    notify502 notify502_;
    notify601 notify601_;
    uint32 roomid_ = 0;
    std::unique_ptr<User> user_;
    std::unique_ptr<Authority> authority_;
};

