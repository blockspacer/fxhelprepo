#include "stdafx.h"
#include <memory>
#include <string>
#include <map>
#include <set>
#include <fstream>

#include "Network/EncodeHelper.h"
#include "Network/IpProxy.h"
#include "UserRoomManager.h"

#undef max
#undef min
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_util.h"

namespace
{
    const uint32 roomusercount = 200;
}

UserRoomManager::UserRoomManager(TcpManager* tcpManager)
    :userController_(new UserController(tcpManager))
    , roomController_(new RoomController)
    , workerThread_("UserRoomManagerThread")
    , break_request_(false)
{
}

UserRoomManager::~UserRoomManager()
{
}

bool UserRoomManager::Initialize()
{
    workerThread_.Start();
    return false;
}

bool UserRoomManager::Finalize()
{
    if (workerThread_.IsRunning())
        workerThread_.Stop();

    return false;
}

void UserRoomManager::SetNotify(std::function<void(std::wstring)> notify)
{
    notify_ = notify;
}

bool UserRoomManager::LoadUserConfig(GridData* userpwd, uint32* total) const
{
    // 读文件
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    path = path.Append(L"BatchLogin.User.cfg");
    base::File userfile(path, base::File::FLAG_OPEN|base::File::FLAG_READ);
    if (!userfile.IsValid())
    {
        assert(false && L"读取配置文件失败");
        LOG(ERROR) << L"读取配置文件BatchLogin.User.cfg失败";
        return false;
    }

    const uint32 memlen = 1024;
    std::string data;
    char str[memlen] = { 0 };
    userfile.Seek(base::File::FROM_BEGIN, 0);
    int read = userfile.ReadAtCurrentPos(str, memlen);
    DWORD err = GetLastError();
    while (read > 0)
    {
        data.insert(data.end(), str, str + read);
        if (read < memlen)//读完了
            break;
        read = userfile.ReadAtCurrentPos(str, memlen);
    }

    assert(!data.empty());

    std::vector<std::string> userinfos = SplitString(data, "\n");

    *total = userinfos.size();
    for (const auto& it : userinfos)
    {
        std::vector<std::string> userinfo = SplitString(it, "\t");
        if (userinfo.size() < 2) // 用户名和密码, 还有可能有cookie
        {
            assert(false && L"account info error!");
            continue;
        }
        std::string username = userinfo[0];
        std::string password = userinfo[1];
        std::string cookies = "";
        if (userinfo.size() > 2)
        {
            cookies = userinfo[2];
        }
        RemoveSpace(&username);
        RemoveSpace(&password);
        RowData row;
        row.push_back(base::UTF8ToWide(username));
        row.push_back(base::UTF8ToWide(password));
        row.push_back(base::UTF8ToWide(cookies));
        userpwd->push_back(row);
    }
    
    return true;
}

bool UserRoomManager::LoadRoomConfig(GridData* roomgrid, uint32* total) const
{
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    path = path.Append(L"BatchLogin.Room.cfg");
    base::File userfile(path, base::File::FLAG_OPEN|base::File::FLAG_READ);
    if (!userfile.IsValid())
    {
        assert(false && L"读取配置文件失败");
        LOG(ERROR) << L"读取配置文件BatchLogin.Room.cfg失败";
        return false;
    }
    const uint32 memlen = 1024;
    std::string data;
    char str[memlen] = { 0 };
    userfile.Seek(base::File::FROM_BEGIN, 0);
    int read = userfile.ReadAtCurrentPos(str, memlen);
    DWORD err = GetLastError();
    while (read > 0)
    {
        data.insert(data.end(), str, str + read);
        if (read < memlen)//读完了
            break;
        read = userfile.ReadAtCurrentPos(str, memlen);
    }

    assert(!data.empty());

    std::vector<std::string> rooms = SplitString(data, "\r\n");
    *total = rooms.size();
    for (const auto& it : rooms)
    {
        std::string roomstr = it;
        RemoveSpace(&roomstr);
        uint32 roomid = 0;
        if (!base::StringToUint(roomstr, &roomid))
        {
            assert(false && L"房间号转换错误");
            LOG(ERROR) << L"roomid change error! " << it;
            continue;
        }
        RowData row;
        row.push_back(base::UintToString16(roomid));
        roomgrid->push_back(row);
    }
    return true;
}

bool UserRoomManager::SaveUserLoginConfig()
{
    workerThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserRoomManager::DoSaveUserLoginConfig, this));
    return true;
}

bool UserRoomManager::LoadIpProxy(GridData* proxyinfo)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"Proxy.txt";
    base::FilePath pathname = dirPath.Append(filename);

    std::ifstream ovrifs;
    ovrifs.open(pathname.value());
    if (!ovrifs)
        return false;

    std::stringstream ss;
    ss << ovrifs.rdbuf();
    if (ss.str().empty())
        return false;

    std::string data = ss.str();
    try
    {
        Json::Reader reader;
        Json::Value root(Json::objectValue);
        if (!Json::Reader().parse(data.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isArray())
        {
            assert(false && L"root is not array");
            return false;
        }

        for (const auto& value : root)//for data
        {
            Json::Value temp;
            uint32 proxytype = GetInt32FromJsonValue(value, "proxytype");
            uint32 proxyport = GetInt32FromJsonValue(value, "port");
            std::string proxyip = value.get("ip", "").asString();
            IpProxy proxy;
            proxy.SetProxyType(static_cast<IpProxy::PROXY_TYPE>(proxytype));
            proxy.SetProxyIp(proxyip);
            proxy.SetProxyPort(proxyport);
            ipProxys_[proxyip] = proxy;

            RowData row;
            row.push_back(base::UintToString16(proxytype));
            row.push_back(base::UTF8ToWide(proxyip));
            row.push_back(base::UintToString16(proxyport));
            proxyinfo->push_back(row);
        }
    }
    catch (...)
    {
        assert(false && L"读取错误");
        return false;
    }
    return true;
}

void UserRoomManager::Notify(const std::wstring& msg)
{
    if (notify_)
        notify_(msg);
}

void UserRoomManager::DoSaveUserLoginConfig()
{
    base::File accountCookieFile;
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"BatchLogin.User.Cookie.cfg";
    base::FilePath pathname = dirPath.Append(filename);
    accountCookieFile.Initialize(pathname,
        base::File::FLAG_OPEN_ALWAYS | base::File::FLAG_APPEND);

    if (!accountCookieFile.IsValid())
        return;

    std::vector<UserLoginInfo> userlogininfo;
    userController_->GetUserLoginInfo(&userlogininfo);
    for (const auto& info : userlogininfo)
    {
        std::string rowstring;
        rowstring += info.accountname + "\t";
        rowstring += info.passwod + "\t";
        rowstring += info.cookies + "\n";
        accountCookieFile.WriteAtCurrentPos((char*)rowstring.data(), rowstring.size());
    }
    accountCookieFile.Close();
}
bool UserRoomManager::BatchLogUsers(
    const std::map<std::wstring, std::wstring>& userAccountPassword)
{
    workerThread_.message_loop_proxy()->PostTask(FROM_HERE, 
        base::Bind(&UserRoomManager::DoBatchLogUsers, this, userAccountPassword));
    return true;
}

void UserRoomManager::DoBatchLogUsers(
    const std::map<std::wstring, std::wstring>& userAccountPassword)
{    
    auto current = ipProxys_.begin();

    for (auto it : userAccountPassword)
    {
        std::string account = base::WideToUTF8(it.first);
        std::string password = base::WideToUTF8(it.second);
        std::string errormsg;
        std::wstring message = base::UTF8ToWide(account) + L" 登录";

        IpProxy ipProxy;
        if (!ipProxys_.empty())
        {
            if (current == ipProxys_.end())
                current = ipProxys_.begin();

            ipProxy = current->second;
            current++;
        }

        if (!userController_->AddUser(account, password, ipProxy, &errormsg))
        {
            message += L"失败," + base::UTF8ToWide(errormsg);
        }
        else
        {
            message += L"成功!";
        }

        Notify(message);
        if (break_request_)
        {
            Notify(L"用户中止操作，登录过程中断");
            break;
        }
    }
}

bool UserRoomManager::BatchLogUsersWithCookie(
    const std::map<std::wstring, std::wstring>& accountCookie)
{
    workerThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserRoomManager::DoBatchLogUsersWithCookie, this, accountCookie));
    return true;
}

void UserRoomManager::DoBatchLogUsersWithCookie(
    const std::map<std::wstring, std::wstring>& accountCookie)
{
    IpProxy ipproxy;
    if (!ipProxys_.empty())
        ipproxy = ipProxys_.begin()->second;

    for (auto it : accountCookie)
    {
        std::string account = base::WideToUTF8(it.first);
        std::string cookie = base::WideToUTF8(it.second);
        std::string errormsg;
        std::wstring message = base::UTF8ToWide(account) + L" cookie登录";
        if (!userController_->AddUserWithCookies(account, cookie, ipproxy, &errormsg))
        {
            message += L"失败," + base::UTF8ToWide(errormsg);
        }
        else
        {
            message += L"成功!";
        }

        Notify(message);
        if (break_request_)
        {
            Notify(L"用户中止操作，登录过程中断");
            break;
        }
    }
}

bool UserRoomManager::FillRooms(const std::vector<std::wstring>& roomids)
{
    std::vector<uint32> iroomids;
    for (auto wstroomid : roomids)
    {
        std::string utf8roomids = base::WideToUTF8(wstroomid);
        uint32 roomid = 0;
        base::StringToUint(utf8roomids, &roomid);
        iroomids.push_back(roomid);
    }
    workerThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserRoomManager::DoFillRooms, this, iroomids));
    return true;

}

void UserRoomManager::DoFillRooms(const std::vector<uint32>& roomids)
{
    for (const auto& roomid : roomids)
    {
        FillSingleRoom(roomid);

        if (break_request_)
        {
            Notify(L"用户中止操作，进入房间过程中断");
            break;
        }
    }
}

void UserRoomManager::FillSingleRoom(uint32 roomid)
{
    bool result = userController_->FillRoom(roomid, roomusercount,
        std::bind(&UserRoomManager::Notify, this, std::placeholders::_1));
    assert(result && L"进入房间失败");
    std::wstring message = L"Fill Room ";
    message += result ? L"Success!" : L"Failed!";
    Notify(message);
}

bool UserRoomManager::UpMVBillboard(const std::wstring& collectionid, 
    const std::wstring& mvid)
{
    workerThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserRoomManager::DoUpMVBillboard, this, collectionid,
        mvid));
    return true;
}

void UserRoomManager::DoUpMVBillboard(const std::wstring& collectionid,
                                      const std::wstring& mvid)
{
    // 这个函数年度不需要用,不执行中断操作
    bool result = userController_->UpMVBillboard(
        base::WideToUTF8(collectionid), base::WideToUTF8(mvid),
        std::bind(&UserRoomManager::Notify, this, std::placeholders::_1));
    assert(result && L"打榜失败");
    std::wstring message = L"upgrade mv billboard ";
    message += result ? L"Success!" : L"Failed!";
    Notify(message);
}

bool UserRoomManager::SendGifts(const std::vector<std::wstring>& users,
    const std::wstring& room_id, uint32 gift_id, uint32 gift_count)
{
    uint32 i_room_id = 0;
    if (!base::StringToUint(base::WideToUTF8(room_id), &i_room_id))
        return false;

    assert(i_room_id);
    workerThread_.message_loop_proxy()->PostTask(FROM_HERE,
        base::Bind(&UserRoomManager::DoSendGifts, this, users,
        i_room_id, gift_id, gift_count));
    return true;
}

void UserRoomManager::DoSendGifts(const std::vector<std::wstring>& users, 
    uint32 roomid, uint32 gift_id, uint32 gift_count)
{
    std::vector<std::string> accounts;
    for (const auto& user : users)
    {
        std::string account = base::WideToUTF8(user);
        accounts.push_back(account);
    }
    userController_->SendGifts(accounts, roomid, gift_id, gift_count,
        std::bind(&UserRoomManager::Notify, this, std::placeholders::_1));
}

bool UserRoomManager::RobVotes(const std::vector<std::wstring>& users, const std::wstring& room_id)
{
    uint32 i_room_id = 0;
    if (!base::StringToUint(base::WideToUTF8(room_id), &i_room_id))
        return false;

    assert(i_room_id);
    workerThread_.message_loop_proxy()->PostTask(
        FROM_HERE, base::Bind(&UserRoomManager::DoRobVotes, this, users, 
        i_room_id));

    return true;
}

void UserRoomManager::DoRobVotes(const std::vector<std::wstring>& users, uint32 roomid)
{
    std::vector<std::string> accounts;
    for (const auto& user : users)
    {
        std::string account = base::WideToUTF8(user);
        accounts.push_back(account);
    }
    userController_->RobVotes(accounts, roomid,
                              std::bind(&UserRoomManager::Notify, 
                              this, std::placeholders::_1));
}

bool UserRoomManager::GetUserStorageInfos(const std::vector<std::wstring>& users, 
    std::vector<UserStorageInfo>* user_storage_infos)
{
    std::vector<std::string> accounts;
    for (const auto& user : users)
    {
        std::string account = base::WideToUTF8(user);
        accounts.push_back(account);
    }

    return userController_->GetUserStorageInfos(accounts, user_storage_infos,
        std::bind(&UserRoomManager::Notify,
        this, std::placeholders::_1));
}

bool UserRoomManager::BatchChangeNickname(const const std::vector<std::wstring>& users,
    const std::wstring& nickname_pre)
{
    std::vector<std::string> accounts;
    for (const auto& user : users)
    {
        std::string account = base::WideToUTF8(user);
        accounts.push_back(account);
    }
    std::string str_nickname_pre = base::WideToUTF8(nickname_pre);
    return userController_->BatchChangeNickname(accounts, str_nickname_pre,
        std::bind(&UserRoomManager::Notify,
        this, std::placeholders::_1));
    return false;
}
void UserRoomManager::SetBreakRequest(bool interrupt)
{
    break_request_ = interrupt;
}
