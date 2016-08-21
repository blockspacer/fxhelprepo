#include "stdafx.h"
#include <fstream>
#include "NetworkHelper.h"

#include "Network/TcpManager.h"
#include "Network/User.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
#include "Network/TcpClient.h"
#include "Network/MessageNotifyManager.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/path_service.h"

namespace
{
    RowData EnterRoomUserInfoToRowdata(const EnterRoomUserInfo& enterRoomUserInfo)
    {
        RowData rowdata;
        rowdata.push_back(base::SysUTF8ToWide(enterRoomUserInfo.nickname));
        rowdata.push_back(base::UintToString16(enterRoomUserInfo.richlevel));
        rowdata.push_back(base::UintToString16(enterRoomUserInfo.userid));
        base::Time entertime = base::Time::FromDoubleT(enterRoomUserInfo.unixtime);
        std::wstring time = base::SysUTF8ToWide(MakeFormatTimeString(entertime).c_str());
        rowdata.push_back(time);
        rowdata.push_back(base::UintToString16(enterRoomUserInfo.roomid));
        return rowdata;
    }
};

AntiStrategy::AntiStrategy()
{

}

AntiStrategy::~AntiStrategy()
{

}

HANDLE_TYPE AntiStrategy::GetUserHandleType(const std::string& nickname) const
{
    for (const auto& it : vestnames_)
    {
        if (nickname.find(it) != std::string::npos)
            return handletype_;
    }
    return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
}

HANDLE_TYPE AntiStrategy::GetHandleType() const
{
    return handletype_;
}

void AntiStrategy::SetHandleType(HANDLE_TYPE handletype)
{
    handletype_ = handletype;
}

bool AntiStrategy::AddNickname(const std::string& vestname)
{
    if (vestnames_.end() != vestnames_.find(vestname))
    {
        return false;
    }
    vestnames_.insert(vestname);
    return true;
}

bool AntiStrategy::RemoveNickname(const std::string& vestname)
{
    auto it = vestnames_.find(vestname);
    if (it == vestnames_.end())
        return false;

    vestnames_.erase(it);
    return true;  
}

GiftStrategy::GiftStrategy()
{

}

GiftStrategy::~GiftStrategy()
{

}

void GiftStrategy::SetThanksFlag(bool enable)
{
    thanksflag_ = enable;
}

bool GiftStrategy::GetGiftThanks(const RoomGiftInfo601& giftinfo, std::wstring* chatmessage)
{
    if (!thanksflag_)
        return false;

    std::wstring thanks = L"感谢" + base::UTF8ToWide(giftinfo.sendername) + 
        L"送来的" + base::UintToString16(giftinfo.gitfnumber) + 
        L"个" + base::UTF8ToWide(giftinfo.giftname);

    chatmessage->assign(thanks.begin(), thanks.end());

    return true;
}

EnterRoomStrategy::EnterRoomStrategy()
{

}

EnterRoomStrategy::~EnterRoomStrategy()
{

}

void EnterRoomStrategy::SetWelcomeFlag(bool enable)
{
    welcomeflag_ = enable;
}

void EnterRoomStrategy::SetWelcomeContent(
    const std::map<uint32, WelcomeInfo>& special_welcome)
{
    base::AutoLock lock(welcome_lock_);
    welcome_info_map_ = special_welcome;
    SaveWelcomeContent(welcome_info_map_);
}

void EnterRoomStrategy::GetWelcomeContent(
    std::map<uint32, WelcomeInfo>* special_welcome)
{
    if (!welcome_info_map_.empty())
    {
        *special_welcome = welcome_info_map_;
    }
    LoadWelcomeContent(special_welcome);
    welcome_info_map_ = *special_welcome;
}

bool EnterRoomStrategy::SaveWelcomeContent(
    const std::map<uint32, WelcomeInfo>& welcome_info_map) const
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"welcome_content.txt";
    base::FilePath pathname = dirPath.Append(filename);

    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& welcome_info : welcome_info_map)
    {
        Json::Value jvWelcomeInfo(Json::objectValue);
        jvWelcomeInfo["fanxingid"] = welcome_info.second.fanxing_id;
        jvWelcomeInfo["name"] = base::WideToUTF8(welcome_info.second.name);
        jvWelcomeInfo["content"] = base::WideToUTF8(welcome_info.second.content);
        root.append(jvWelcomeInfo);
    }

    std::string writestring = writer.write(root);

    std::ofstream ofs(pathname.value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << writestring;
    ofs.flush();
    ofs.close();

    return true;
}

bool EnterRoomStrategy::LoadWelcomeContent(
    std::map<uint32, WelcomeInfo>* special_welcome)
{
    if (!special_welcome)
        return false;

    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = L"welcome_content.txt";
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
            uint32 fanxingid = GetInt32FromJsonValue(value, "fanxingid");
            std::string name = value.get("name", "").asString();
            std::string content = value.get("content", "").asString();
            WelcomeInfo welcome_info = { fanxingid, base::UTF8ToWide(name),base::UTF8ToWide(content) };
            (*special_welcome)[fanxingid] = welcome_info;
        }
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool EnterRoomStrategy::GetEnterWelcome(const EnterRoomUserInfo& enterinfo, 
    std::wstring* chatmessage)
{
    if (!welcomeflag_)
        return false;

    std::wstring msg;
    base::AutoLock lock(welcome_lock_);
    auto find = welcome_info_map_.find(enterinfo.userid);
    if (find != welcome_info_map_.end())
    {
        msg = find->second.content;
    }
    else
    {
        msg = L"欢迎 " + base::UTF8ToWide(enterinfo.nickname) + L" 进入直播间";
    }
     
    chatmessage->assign(msg.begin(), msg.end());
    return true;
}

NetworkHelper::NetworkHelper()
    : authority_(new Authority)
    , tcpmanager_(new TcpManager)
    , workThread_(new base::Thread("NetworkHelper"))
{
}

NetworkHelper::~NetworkHelper()
{
}

bool NetworkHelper::Initialize()
{
    CurlWrapper::CurlInit();
    workThread_->Start();
    user_.reset(new User);
    tcpmanager_->Initialize();
    AuthorityHelper authorityHelper;
    bool result = authorityHelper.Load(authority_.get());
    assert(!authority_->serverip.empty());
    user_->SetRoomServerIp(authority_->serverip);
    user_->SetTcpManager(tcpmanager_.get());
    return result;
}

void NetworkHelper::Finalize()
{
    chatRepeatingTimer_.Stop();
    workThread_->Stop();
    tcpmanager_->Finalize();
    CurlWrapper::CurlCleanup();
    return;
}

void NetworkHelper::SetAntiStrategy(std::shared_ptr<AntiStrategy> antiStrategy)
{
    antiStrategy_ = antiStrategy;
}

void NetworkHelper::SetGiftStrategy(std::shared_ptr<GiftStrategy> giftStrategy)
{
    giftStrategy_ = giftStrategy;
}

void NetworkHelper::SetGiftThanks(bool enable)
{
    giftStrategy_->SetThanksFlag(enable);
}

void NetworkHelper::SetEnterRoomStrategy(std::shared_ptr<EnterRoomStrategy> enterRoomStrategy)
{
    enterRoomStrategy_ = enterRoomStrategy;
}

void NetworkHelper::SetRoomWelcome(bool enable)
{
    enterRoomStrategy_->SetWelcomeFlag(enable);
}

void NetworkHelper::SetRoomRepeatChat(bool enable, const std::wstring& seconds,
                                      const std::wstring& chatmsg)
{
    uint32 sec = 0;
    base::StringToUint(base::WideToUTF8(seconds),&sec);

    if (sec < 30)
    {
        NotifyCallback(L"设置重复说话时间要超过30秒");
        return;
    }
    
    workThread_->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&NetworkHelper::DoSetRoomRepeatChat, this, enable, sec, chatmsg));
}

void NetworkHelper::SetNotify(notifyfn fn)
{
    notify_ = fn;
}

void NetworkHelper::RemoveNotify()
{
    notify_ = nullptr;
}

void NetworkHelper::SetNotify201(notify201 fn)
{
    notify201_ = fn;
}

void NetworkHelper::RemoveNotify201()
{
    notify201_ = nullptr;
}

void NetworkHelper::SetNotify501(notify501 fn)
{
    notify501_ = fn;
}

void NetworkHelper::RemoveNotify501()
{
    notify501_ = nullptr;
}

void NetworkHelper::SetNotify502(notify502 fn)
{
    notify502_ = fn;
}
void NetworkHelper::RemoveNotify502()
{
    notify502_ = nullptr;
}


void NetworkHelper::SetNotify601(notify601 fn)
{
    notify601_ = fn;
}

void NetworkHelper::RemoveNotify601()
{
    notify601_ = nullptr;
}

bool NetworkHelper::Login(const std::wstring& username, 
    const std::wstring& password, const std::wstring& verifycode,
    std::string* errormsg)
{
    std::string strusername = base::WideToUTF8(username);
    std::string strpassword = base::WideToUTF8(password);
    std::string strverifycode = base::WideToUTF8(verifycode);
    return user_->Login(strusername, strpassword, strverifycode, errormsg);
}

bool NetworkHelper::LoginGetVerifyCode(std::vector<uint8>* picture)
{
    return user_->LoginGetVerifyCode(picture);
}

bool NetworkHelper::GetCurrentUserDisplay(std::wstring* display)
{
    uint32 clanid = user_->GetClanId();
    uint32 fanxingid = user_->GetFanxingId();
    uint64 servertime = static_cast<uint64>(user_->GetServerTime());
    if (!fanxingid && !clanid)
        return false;

    if (!servertime)
        return false;

    servertime *= 1000000;
    std::wstring loginInfo = L"登录信息: ";
    loginInfo += L"繁星号: " + base::UintToString16(fanxingid);
    loginInfo += L" 公会ID: " + base::UintToString16(clanid);
    *display = loginInfo;
    return true;
}

bool NetworkHelper::EnterRoom(const std::wstring& roomid)
{
    std::string strroomid = base::WideToUTF8(roomid);
    uint32 introomid = 0;
    if (!base::StringToUint(strroomid, &introomid))
    {
        return false;
    }
    return EnterRoom(introomid);
}

bool NetworkHelper::EnterRoom(uint32 roomid)
{
    user_->ExitRooms();
    user_->SetNotify201(std::bind(&NetworkHelper::NotifyCallback201, this,
        std::placeholders::_1));
    user_->SetNotify501(std::bind(&NetworkHelper::NotifyCallback501, this,
        std::placeholders::_1, std::placeholders::_2));

    user_->SetNotify601(
        std::bind(&NetworkHelper::NotifyCallback601,
        this, roomid, std::placeholders::_1));

    user_->SetNormalNotify(std::bind(&NetworkHelper::NotifyCallback, this,
        std::placeholders::_1));
    roomid_ = roomid;
    return user_->EnterRoomFopOperation(roomid);
}

bool NetworkHelper::GetViewerList(uint32 roomid,
    std::vector<RowData>* enterRoomUserInfoRowdata)
{
    std::vector<EnterRoomUserInfo> enterRoomUserInfoList;
    bool result = user_->GetViewerList(roomid, &enterRoomUserInfoList);
    for (const auto& enterRoomUserInfo : enterRoomUserInfoList)
    {
        RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
        enterRoomUserInfoRowdata->push_back(rowdata);
    }
    return result;
}

// messageNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback(const std::wstring& message)
{
    // 解析数据包
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(message, 936);
    if (notify_)
    {
        notify_(message);
    }
}

bool NetworkHelper::KickoutUsers(KICK_TYPE kicktype, uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
	return user_->KickoutUser(kicktype, roomid, enterRoomUserInfo);;
}

bool NetworkHelper::BanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    return user_->BanChat(roomid, enterRoomUserInfo);
}

bool NetworkHelper::UnbanChat(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    return user_->UnbanChat(roomid, enterRoomUserInfo);
}

bool NetworkHelper::Worship(uint32 roomid, uint32 fanxingid)
{
    return user_->Worship(roomid, fanxingid);
}

bool NetworkHelper::SendChatMessage(uint32 roomid, const std::string& message)
{
    return user_->SendChatMessage(roomid, message);
}

void NetworkHelper::SetRobotHandle(bool enable)
{
    robotstate_ = enable;
}

void NetworkHelper::SetRobotApiKey(const std::wstring& apikey)
{
    std::string utf8key = base::WideToUTF8(apikey);
    user_->SetRobotApiKey(utf8key);
}

bool NetworkHelper::SendChatMessageRobot(const RoomChatMessage& roomChatMessage)
{
    return user_->SendChatMessageRobot(roomChatMessage);
}

void NetworkHelper::SetHandleChatUsers(bool handleall501)
{
    handleall501_ = handleall501;
}

bool NetworkHelper::GetActionPrivilege(std::wstring* message)
{
    if (user_->GetFanxingId() != authority_->userid)
    {
        *message = L"当前用户未授权!";
        return false;
    }

    uint32 servertime = user_->GetServerTime();
    uint64 expiretime = authority_->expiretime - base::Time::UnixEpoch().ToInternalValue();
    expiretime /= 1000000;
    if (servertime > expiretime)
    {
        *message = L"当前用户授权已过期!";
        return false;
    }

    if (authority_->clanid && user_->GetClanId() != authority_->clanid)
    {
        *message = L"当前授权仅限指定公会成员使用!";
        return false;
    }


    if (authority_->roomid && (roomid_ != authority_->roomid))
    {
        *message = L"当前授权仅限指定房间使用!";
        return false;
    }

    return true;
}

// messageNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback601(uint32 roomid, const RoomGiftInfo601& roomgiftinfo601)
{
    if (!roomgiftinfo601.token.empty())
    {
        // 原本是抢币的动作，目前不做这类功能
    }

    std::wstring chatmsg;
    if (giftStrategy_->GetGiftThanks(roomgiftinfo601, &chatmsg))
    {
        user_->SendChatMessage(roomid_, base::WideToUTF8(chatmsg));
    }
}

void NetworkHelper::NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo)
{
    TryHandleUser(enterRoomUserInfo);

    std::wstring chatmsg;
    if (enterRoomStrategy_->GetEnterWelcome(enterRoomUserInfo, &chatmsg))
    {
        user_->SendChatMessage(roomid_, base::WideToUTF8(chatmsg));
    }

    if (!notify201_)
        return;

    enterRoomUserInfoMap_[enterRoomUserInfo.userid] = enterRoomUserInfo;
    RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
    notify201_(rowdata);
}

void NetworkHelper::NotifyCallback501(const EnterRoomUserInfo& enterRoomUserInfo,
    const RoomChatMessage& roomChatMessage)
{
    TryHandleUser(enterRoomUserInfo);
    RobotHandleChatMessage(enterRoomUserInfo, roomChatMessage);
    TryHandle501Msg(enterRoomUserInfo);
    if (!notify501_)
        return;

    enterRoomUserInfoMap_[enterRoomUserInfo.userid] = enterRoomUserInfo;
    RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
    notify501_(rowdata);
}

void NetworkHelper::TryHandleUser(const EnterRoomUserInfo& enterRoomUserInfo)
{
    HANDLE_TYPE handletype = antiStrategy_->GetUserHandleType(enterRoomUserInfo.nickname);
    bool result = true;
    switch (handletype)
    {
    case HANDLE_TYPE::HANDLE_TYPE_BANCHAT:
        result = BanChat(roomid_, enterRoomUserInfo);
        break;
    case HANDLE_TYPE::HANDLE_TYPE_KICKOUT:
        result = KickoutUsers(KICK_TYPE::KICK_TYPE_HOUR, roomid_, enterRoomUserInfo);
        break;
    default:
        break;
    }
    assert(result);
}

void NetworkHelper::TryHandle501Msg(const EnterRoomUserInfo& enterRoomUserInfo)
{
    if (!handleall501_)
        return;

    HANDLE_TYPE handletype = antiStrategy_->GetHandleType();
    bool result = true;
    switch (handletype)
    {
    case HANDLE_TYPE::HANDLE_TYPE_BANCHAT:
        result = BanChat(roomid_, enterRoomUserInfo);
        break;
    case HANDLE_TYPE::HANDLE_TYPE_KICKOUT:
        result = KickoutUsers(KICK_TYPE::KICK_TYPE_HOUR, roomid_, enterRoomUserInfo);
        break;
    default:
        break;
    }
    assert(result);
}

void NetworkHelper::RobotHandleEnterRoom(const EnterRoomUserInfo& enterRoomUserInfo)
{

}

void NetworkHelper::RobotHandleChatMessage(
    const EnterRoomUserInfo& enterRoomUserInfo, 
    const RoomChatMessage& roomChatMessage)
{
    if (!robotstate_)
        return;

    // 不是对这个用户说的消息不要处理
    if (user_->GetFanxingId() != roomChatMessage.receiverid)
        return;

    // 从接口获取回复信息
    std::string response;
    if (!user_->RequestRobot(roomChatMessage.senderid,
        roomChatMessage. chatmessage, &response))
    {
        return;
    }
    
    RoomChatMessage responseChat;
    responseChat.roomid = roomChatMessage.roomid;
    responseChat.senderid = roomChatMessage.receiverid;
    responseChat.sendername = roomChatMessage.receivername;
    responseChat.richlevel = roomChatMessage.richlevel;
    responseChat.receiverid = roomChatMessage.senderid;
    responseChat.receivername = roomChatMessage.sendername;
    responseChat.issecrect = roomChatMessage.issecrect;
    responseChat.chatmessage = response;

    SendChatMessageRobot(responseChat);
}

void NetworkHelper::DoSetRoomRepeatChat(bool enable, uint32 seconds,
                                        const std::wstring& chatmsg)
{
    chatRepeatingTimer_.Stop();
    if (!enable)
        return;
    
    DoChatRepeat(chatmsg);
    chatRepeatingTimer_.Start(
        FROM_HERE, base::TimeDelta::FromSeconds(static_cast<uint64>(seconds)),
        base::Bind(&NetworkHelper::DoChatRepeat, this, chatmsg));
}

void NetworkHelper::DoChatRepeat(const std::wstring& chatmsg)
{
    user_->SendChatMessage(roomid_, base::WideToUTF8(chatmsg));
}

