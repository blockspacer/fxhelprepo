#pragma once
#include <map>
#include <vector>
#include <memory>
#include <functional>
#undef max
#undef min
#include "third_party/chromium/base/basictypes.h"

class TcpClientController;
#include "Network/User.h"
class User;
class MVBillboard;
class IpProxy;
// 批量控制用户类
struct UserLoginInfo
{
    std::string accountname;
    std::string passwod;
    std::string cookies;
};

class UserController
{
public:
    UserController(TcpClientController* tcpManager);
    ~UserController();

    bool Initialize(const scoped_refptr<base::TaskRunner>& runner);
    void Finalize();

    // 在已经知道cookie的情况下不需要再做比较费时的登录操作了
    bool AddUserWithCookies(const std::string& username, 
        const std::string& cookies, const IpProxy& ipproxy, 
        std::string* errormsg);

    bool AddUser(const std::string& username, const std::string& password, 
        const IpProxy& ipproxy, std::string* errormsg);

    bool GetUserLoginInfo(std::vector<UserLoginInfo>* userlogininfo);
    bool SendGifts(const std::vector<std::string>& accounts,
        uint32 roomid, uint32 gift_id, uint32 gift_count,
        const std::function<void(const std::wstring& msg)>& callback);

    bool RealSingLike(const std::string& account,
        uint32 roomid, const std::wstring& song_name,
        const std::function<void(const std::wstring& msg)>& callback);

    bool RobVotes(const std::vector<std::string>& users, uint32 room_id,
                  const std::function<void(const std::wstring& msg)>& callback);

    bool GetUserStorageInfos(const std::vector<std::string>& users,
        std::vector<UserStorageInfo>* user_storage_infos,
        const std::function<void(const std::wstring& msg)>& callback);

    bool BatchChangeNickname(const std::vector<std::string>& users,
        const std::string& nickname_pre,
        const std::function<void(const std::wstring& msg)>& callback);

    bool SingleChangeNickname(const std::string& old_nickname,
        const std::string& new_nickname,
        const std::function<void(const std::wstring& msg)>& callback);

    bool BatchChangeLogo(const std::vector<std::string>& users,
        const std::string& logo_path,
        const std::function<void(const std::wstring& msg)>& callback);

    bool BatchSendChat(uint32 roomid, 
        const std::vector<std::string>& users,
        const std::string& message,
        const std::function<void(const std::wstring& msg)>& callback);

    bool FillRoom(uint32 roomid, uint32 count,
        const std::function<void(const std::wstring& msg)>& callback);
    bool UpMVBillboard(const std::string& collectionid, const std::string& mvid,
                       std::function<void(const std::wstring&message)> callback);

    // 执行对应批量用户控制策略,暂时不处理进入房间的通知回调
    void Run();

private:
    // 提供连接状态出错重连的功能
    void ConnectionBreakCallback(const std::string& user_name, uint32 room_id);

    TcpClientController* tcpManager_;
    std::map<std::string, std::shared_ptr<User> > users_;
    std::unique_ptr<MVBillboard> mvBillboard_;

    std::shared_ptr<Room> shared_room;

    scoped_refptr<base::TaskRunner> runner_;
};

