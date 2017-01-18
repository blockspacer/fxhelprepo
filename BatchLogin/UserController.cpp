#include "stdafx.h"
#include "UserController.h"
#include "MVBillboard.h"
#include "Network/User.h"
#include "Network/EncodeHelper.h"
#include "Network/IpProxy.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/callback.h"

namespace
{
    //const char* serverip = "42.62.68.50";
    const char* serverip = "114.54.2.205";
}
UserController::UserController(TcpClientController* tcpManager)
    : mvBillboard_(new MVBillboard)
    , tcpManager_(tcpManager)
{
    assert(tcpManager_);
}

UserController::~UserController()
{
}

bool UserController::Initialize(const scoped_refptr<base::TaskRunner>& runner)
{
    runner_ = runner;
    return true;
}

void UserController::Finalize()
{
}

bool UserController::AddUser(const std::string& username, 
    const std::string& password, const IpProxy& ipproxy, std::string* errormsg)
{
    if (users_.find(username) != users_.end())
    {
        *errormsg = "重复登录";
        return false;
    }
    std::shared_ptr<User> shared_user(new User);
    if (ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        shared_user->SetIpProxy(ipproxy);
    
    shared_user->Initialize(runner_);
    if (!shared_user->Login(username, password, "", errormsg))
    {
        std::wstring werror = base::UTF8ToWide(*errormsg);
        //assert(false && L"登录失败");
        return false;
    }
   
    shared_user->SetTcpManager(tcpManager_);
    shared_user->SetRoomServerIp(serverip);
    users_[username] = shared_user;

    return true;
}

bool UserController::AddUserWithCookies(const std::string& username,
    const std::string& cookies, const IpProxy& ipproxy, std::string* errormsg)
{
    if (users_.find(username)!=users_.end())
    {
        *errormsg = "重复登录";
        return false;
    }
    std::shared_ptr<User> shared_user(new User);
    shared_user->SetUsername(username);
    if (ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
        shared_user->SetIpProxy(ipproxy);

    shared_user->Initialize(runner_);
    if (!shared_user->LoginWithCookies(cookies, errormsg))
    {
        std::wstring werror = base::UTF8ToWide(*errormsg);
        assert(false && L"登录失败");
        return false;
    }

    shared_user->SetTcpManager(tcpManager_);
    shared_user->SetRoomServerIp(serverip);
    users_[username] = shared_user;

    return true;
}

bool UserController::GetUserLoginInfo(std::vector<UserLoginInfo>* userlogininfo)
{
    for (const auto& it : users_)
    {
        UserLoginInfo logininfo;
        logininfo.accountname = it.second->GetUsername();
        logininfo.passwod = it.second->GetPassword();
        logininfo.cookies = it.second->GetCookies();
        userlogininfo->push_back(logininfo);
    }
    return true;
}

bool UserController::SendGifts(const std::vector<std::string>& accounts,
    uint32 roomid, uint32 gift_id, uint32 gift_count,
    const std::function<void(const std::wstring& msg)>& callback)
{
    for (const auto& account : accounts)
    {
        auto result =users_.find(account);
        if (result == users_.end())
        {
            assert(false && L"找不到对应用户");
            continue;
        }
        std::wstring msg = base::UTF8ToWide(result->first) + L" 赠送 [" + 
            base::UintToString16(gift_count) + L"] 个 giftid = " + 
            base::UintToString16(gift_id) + L" 礼物";
        std::string errormsg;
        if (!result->second->SendGift(roomid, gift_id, gift_count, &errormsg))
        {
            msg += L"失败";
            callback(msg);
            callback(base::UTF8ToWide(errormsg));
        }
        else
        {
            msg += L"成功";
            callback(msg);
        }
    }
    return true;
}

bool UserController::RealSingLike(const std::string& account,
    uint32 roomid, const std::wstring& song_name,
    const std::function<void(const std::wstring& msg)>& callback)
{
    auto result = users_.find(account);
    if (result == users_.end())
    {
        assert(false && L"找不到对应用户");
        return false;
    }
    std::wstring msg = base::UTF8ToWide(result->first) + L" 点赞";
    std::string errormsg;
    if (!result->second->RealSingLike(roomid, song_name, &errormsg))
    {
        msg += L"失败";
        callback(msg);
        callback(base::UTF8ToWide(errormsg));
    }
    else
    {
        msg += L"成功";
        callback(msg);
    }

    return true;
}

bool UserController::RobVotes(
    const std::vector<std::string>& users, uint32 room_id,
    const std::function<void(const std::wstring& msg)>& callback)
{
    for (const auto& account : users)
    {
        auto result = users_.find(account);
        if (result == users_.end())
        {
            callback(L"本地数据错误, 找不到对应用户或用户未登录");
            continue;
        }

        uint32 award_count = 0;
        uint32 single_count = 0;
        std::string errormsg;
        std::wstring msg = base::UTF8ToWide(result->first) + L" 开启大奖宝箱 ";
        if (!result->second->RobVotes(room_id, &award_count, &single_count, &errormsg))
        {
            msg += L"失败 " + base::UTF8ToWide(errormsg);
        }
        else
        {
            msg += L"成功 [" + base::UintToString16(award_count) +L"/"
                + base::UintToString16(award_count) + L"] " + base::UTF8ToWide(errormsg);
        }
        callback(msg);
    }
    return true;
}

bool UserController::GetUserStorageInfos(const std::vector<std::string>& users,
    std::vector<UserStorageInfo>* user_storage_infos,
    const std::function<void(const std::wstring& msg)>& callback)
{
    for (const auto& account : users)
    {
        auto result = users_.find(account);
        if (result == users_.end())
        {
            callback(L"本地数据错误, 找不到对应用户或用户未登录");
            continue;
        }

        std::string errormsg;
        UserStorageInfo user_storage_info;
        std::wstring msg = base::UTF8ToWide(result->first) + L" 获取仓库信息 ";
        if (!result->second->GetStorageGift(&user_storage_info, &errormsg))
        {
            msg += L"失败 " + base::UTF8ToWide(errormsg);
            callback(msg);
            continue;
        }
        else
        {
            msg += L"成功 ";
            callback(msg);
        }
        user_storage_infos->push_back(user_storage_info);
    }
    return true;
}

bool UserController::BatchChangeNickname(const std::vector<std::string>& users,
    const std::string& nickname_pre, 
    const std::function<void(const std::wstring& msg)>& callback)
{
    for (const auto& account : users)
    {
        auto result = users_.find(account);
        if (result == users_.end())
        {
            callback(L"本地数据错误, 找不到对应用户或用户未登录");
            continue;
        }

        std::string errormsg;
        std::string timestamp = GetNowTimeString().substr(6, 5);
        std::string nickname = nickname_pre + timestamp;
        std::wstring msg = base::UTF8ToWide(result->first) + L" 改名";
        if (!result->second->ChangeNickname(nickname, &errormsg))
        {
            msg += L"失败 " + base::UTF8ToWide(errormsg);
            callback(msg);
            continue;
        }
        else
        {
            msg += L"成功 ";
            callback(msg);
        }
    }
    return true;
}

bool UserController::SingleChangeNickname(const std::string& old_nickname,
    const std::string& new_nickname,
    const std::function<void(const std::wstring& msg)>& callback)
{
    auto result = users_.find(old_nickname);
    if (result == users_.end())
    {
        callback(L"本地数据错误, 找不到对应用户或用户未登录");
        return false;
    }

    std::string errormsg;
    std::wstring msg = base::UTF8ToWide(result->first) + L" 改名";
    if (!result->second->ChangeNickname(new_nickname, &errormsg))
    {
        msg += L"失败 " + base::UTF8ToWide(errormsg);
        callback(msg);
        return false;
    }

    msg += L"成功 ";
    callback(msg);
    return true;
}

bool UserController::BatchChangeLogo(const std::vector<std::string>& users,
    const std::string& logo_path,
    const std::function<void(const std::wstring& msg)>& callback)
{
    for (const auto& account : users)
    {
        auto result = users_.find(account);
        if (result == users_.end())
        {
            callback(L"本地数据错误, 找不到对应用户或用户未登录");
            continue;
        }

        std::string errormsg;
        std::wstring msg = base::UTF8ToWide(result->first) + L" 更改头像";
        if (!result->second->ChangeLogo(logo_path, &errormsg))
        {
            msg += L"失败 " + base::UTF8ToWide(errormsg);
            callback(msg);
            continue;
        }
        else
        {
            msg += L"成功 ";
            callback(msg);
        }
    }
    return true;
}

bool UserController::FillRoom(uint32 roomid, uint32 count,
    const std::function<void(const std::wstring& msg)>& callback)
{
    bool first = true;
    for (const auto& it : users_)
    {
        //it.second->EnterRoomFopAlive(roomid);
        // 年度需求, 需要获取到足够信息，但是不需要连接信息
        std::wstring msg = base::UTF8ToWide(it.first) + L" 进入房间";
        std::string nickname;

        if (!it.second->EnterRoomFopAlive(roomid,
            base::Bind(&UserController::ConnectionBreakCallback,
            base::Unretained(this), it.second->GetUsername(),
            roomid)))
        {
            msg += L" 失败 ";
        }
        else
        {
            msg += L" 成功 ";
            it.second->GetRoom(roomid, &shared_room);
        }

        msg += base::UTF8ToUTF16(nickname);
        callback(msg);
    }
    return true;
}

bool UserController::UpMVBillboard(const std::string& collectionid,
                                   const std::string& mvid,
                                   std::function<void(const std::wstring&message)> callback)
{
    uint32 successCount = 0;
    for (const auto& it : users_)
    {
        std::wstring message = base::UTF8ToWide(it.second->GetUsername()) + L"打榜" + base::UTF8ToWide(mvid);
        std::string cookie = it.second->GetCookies();
        
        if (mvBillboard_->UpMVBillboard(cookie, collectionid, mvid))
        {
            message += L"成功";
            successCount++;
        }
        else
        {
            message += L"失败";
        }
        if (callback)
        {
            callback(message);
        }
    }
    assert(users_.size() == successCount);
    return successCount > 0;
}

void UserController::Run()
{
}

// 提供连接状态出错重连的功能
void UserController::ConnectionBreakCallback(const std::string& user_name, 
    uint32 room_id)
{

}
