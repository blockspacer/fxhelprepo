#include "stdafx.h"
#include <memory>
#include <string>
#include <map>
#include <set>

#include "Network/EncodeHelper.h"
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

UserRoomManager::UserRoomManager()
    :userController_(new UserController)
    , roomController_(new RoomController)
    , workerThread_("UserRoomManagerThread")
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

bool UserRoomManager::LoadUserConfig(GridData* userpwd, uint32* total)
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

    std::vector<std::string> userinfos = SplitString(data, "\r\n");

    *total = userinfos.size();
    for (const auto& it : userinfos)
    {
        std::vector<std::string> userinfo = SplitString(it, "\t");
        if (userinfo.size()!=2) // 用户名和密码
        {
            assert(false && L"account info error!");
            continue;
        }
        std::string username = userinfo[0];
        std::string password = userinfo[1];
        RemoveSpace(&username);
        RemoveSpace(&password);
        //if (!userController_->AddUser(username, password))
        //    continue;
        RowData row;
        row.push_back(base::UTF8ToWide(username));
        row.push_back(base::UTF8ToWide(password));
        userpwd->push_back(row);
    }
    
    return true;
}


bool UserRoomManager::LoadRoomConfig(GridData* roomgrid, uint32* total)
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
        //roomController_->AddRoom(roomid);
        RowData row;
        row.push_back(base::UintToString16(roomid));
        roomgrid->push_back(row);
    }
    return true;
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
    for (auto it : userAccountPassword)
    {
        std::string account = base::WideToUTF8(it.first);
        std::string password = base::WideToUTF8(it.second);
        bool result = userController_->AddUser(account, password);
        if (notify_)
        {
            std::wstring message = base::UTF8ToWide(account) + L" Login ";
            message += result ? L"success!" : L"failed!";
            notify_(message);
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
    //std::vector<uint32> roomids = roomController_->GetRooms();
    for (const auto& roomid : roomids)
    {
        bool result = userController_->FillRoom(roomid, roomusercount);
        assert(result && L"进入房间失败");
        if (notify_)
        {
            std::wstring message = L"Fill Room ";
            message += result ? L"Success!" : L"Failed!";
            notify_(message);
        }
    }
}

void UserRoomManager::FillSingleRoom(uint32 roomid)
{
    bool result = userController_->FillRoom(roomid, roomusercount);
    assert(result && L"进入房间失败");
    if (notify_)
    {
        std::wstring message = L"Fill Room ";
        message += result ? L"Success!" : L"Failed!";
        notify_(message);
    }

}