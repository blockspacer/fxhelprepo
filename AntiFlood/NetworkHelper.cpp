#include "stdafx.h"
#include <fstream>
#include "NetworkHelper.h"

#include "PhoneRank.h"
#include "Network/TcpClientController.h"
#include "Network/User.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
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
    const wchar_t* filename = L"AntiVestSensitive.txt";

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
    SaveAntiSetting();
}

bool AntiStrategy::LoadAntiSetting(std::vector<RowData>* rowdatas)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
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
            std::string vestname = value.get("vestname", "vestname").asString();
            std::string sensitive = value.get("sensitive", "sensitive").asString();
            if (!vestname.empty())
                vestnames_.insert(vestname);

            if (!sensitive.empty())
                sensitives_.insert(sensitive);
        }
    }
    catch (...)
    {
        return false;
    }

    for (const auto& vestname : vestnames_)
    {
        RowData rawdata;
        rawdata.push_back(L"马甲");
        rawdata.push_back(base::UTF8ToWide(vestname));
        rawdata.push_back(L"");
        rowdatas->push_back(rawdata);
    }

    for (const auto& sensitive : sensitives_)
    {
        RowData rawdata;
        rawdata.push_back(L"敏感词");
        rawdata.push_back(L"");
        rawdata.push_back(base::UTF8ToWide(sensitive));
        rowdatas->push_back(rawdata);
    }

    return true;
}

bool AntiStrategy::SaveAntiSetting() const
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname = dirPath.Append(filename);

    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& vestname : vestnames_)
    {
        Json::Value vestinfo(Json::objectValue);
        vestinfo["vestname"] = vestname;
        vestinfo["sensitive"] = "";
        root.append(vestinfo);
    }

    for (const auto& sensitive : sensitives_)
    {
        Json::Value vestinfo(Json::objectValue);
        vestinfo["vestname"] = "";
        vestinfo["sensitive"] = sensitive;
        root.append(vestinfo);
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

HANDLE_TYPE AntiStrategy::GetUserHandleType(uint32 rich_level,
    const std::string& nickname) const
{
    if (rich_level >= rich_level_)// 指定等级以上的不处理
        return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    for (const auto& it : vestnames_)
    {
        if (nickname.find(it) != std::string::npos)
            return handletype_;
    }
    return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
}

HANDLE_TYPE AntiStrategy::GetMessageHandleType(uint32 rich_level,
    const std::string& message) const
{
    if (rich_level >= rich_level_)// 指定等级以上的不处理
        return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    for (const auto& it : sensitives_)
    {
        if (message.find(it) != std::string::npos)
            return handletype_;
    }
    return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
}

HANDLE_TYPE AntiStrategy::GetHandleType(uint32 rich_level) const
{
    if (rich_level >= rich_level_)// 指定等级以上的不处理
        return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    return handletype_;
}

void AntiStrategy::SetHandleType(HANDLE_TYPE handletype)
{
    handletype_ = handletype;
}

void AntiStrategy::SetHandleRichLevel(uint32 rich_level)
{
    rich_level_ = rich_level;
}

bool AntiStrategy::AddSensitive(const std::string& sensitive)
{
    if (sensitives_.end() != sensitives_.find(sensitive))
    {
        return false;
    }
    sensitives_.insert(sensitive);
    return true;
}

bool AntiStrategy::RemoveSensitive(const std::string& sensitive)
{
    auto it = sensitives_.find(sensitive);
    if (it == sensitives_.end())
        return false;

    sensitives_.erase(it);
    return true;
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

bool GiftStrategy::Initialize(const std::string& content)
{
    auto pos = content.find("jsonpcallback_httpvisitorfanxingkugoucomVServicesGiftServiceGiftServicegetGiftList");
    if (pos == std::string::npos)
        return false;

    pos = content.find("(", pos);
    if (pos == std::string::npos)
        return false;

    auto begin = pos + 1;
    pos = content.find(")", begin);
    if (pos == std::string::npos)
        return false;
    auto end = pos;

    std::string root = content.substr(begin, end - begin);
    //解析json数据
    Json::Reader reader;
    Json::Value rootdata(Json::objectValue);
    if (!reader.parse(root, rootdata, false))
    {
        return false;
    }

    // 有必要检测status的值
    uint32 status = rootdata.get(std::string("status"), 0).asInt();
    if (status != 1)
    {
        return false;
    }

    Json::Value dataObject(Json::objectValue);
    dataObject = rootdata.get(std::string("data"), dataObject);
    if (dataObject.empty())
    {
        return false;
    }

    // category_
    Json::Value categoryObject(Json::arrayValue);
    categoryObject = dataObject.get("category", categoryObject);
    if (categoryObject.empty())
    {
        return false;
    }
    for (const auto& it : categoryObject)
    {
        uint32 classid = GetInt32FromJsonValue(it, "classId");
        // 已经自动做了字符转换为utf8, nice
        std::string className = it.get("className", "").asString();
        category_.insert(std::make_pair(classid, className));
    }

    // data
    Json::Value listObject(Json::arrayValue);
    listObject = dataObject.get("list", listObject);
    if (listObject.empty())
    {
        return false;
    }

    for (const auto& it : listObject)
    {
        GiftInfo giftinfo;
        giftinfo.giftid = GetInt32FromJsonValue(it, "id");
        giftinfo.giftname = it.get("name", "").asString();
        giftinfo.price = GetInt32FromJsonValue(it, "price");
        giftinfo.exchange = GetDoubleFromJsonValue(it, "exchange");
        giftinfo.category = GetInt32FromJsonValue(it, "category");
        auto find = category_.find(giftinfo.category);
        if (find != category_.end())
        {
            giftinfo.categoryname = find->second;
        }
        giftmap_.insert(std::make_pair(giftinfo.giftid, giftinfo));
    }

    return true;
}

void GiftStrategy::SetThanksFlag(bool enable)
{
    thanksflag_ = enable;
}

void GiftStrategy::SetGiftValue(uint32 gift_value)
{
    gift_value_ = gift_value;
}

bool GiftStrategy::GetGiftThanks(const RoomGiftInfo601& giftinfo, std::wstring* chatmessage)
{
    if (!thanksflag_)
        return false;

    const auto& it = giftmap_.find(giftinfo.giftid);
    uint32 gift_value = 0;
    if (it != giftmap_.end()) // 如果在礼物列表能找到，就判断价值，如果找不到直接感谢
    {
        gift_value = giftinfo.gitfnumber * it->second.price;
        LOG(INFO) << __FUNCTION__ << L"gift value = [" << base::UintToString16(gift_value) << L" / " << base::UintToString16(gift_value_)<< L"]";
        if (gift_value < gift_value_) // 价值少于设置点的，不发送感谢
            return false;
    }

    std::wstring thanks = L"感谢" + base::UTF8ToWide(giftinfo.sendername) + 
        L"送来的" + base::UintToString16(giftinfo.gitfnumber) + 
        L"个" + base::UTF8ToWide(giftinfo.giftname);

    chatmessage->assign(thanks.begin(), thanks.end());

    return true;
}

NetworkHelper::NetworkHelper()
    : authority_(new AntiFloodAuthority)
    , tcp_client_controller_(new TcpClientController)
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
    tcp_client_controller_->Initialize();
    AuthorityHelper authorityHelper;
    bool result = authorityHelper.LoadAntiFloodAuthority(authority_.get());
    assert(!authority_->serverip.empty());
    user_->SetRoomServerIp(authority_->serverip);
    user_->SetTcpManager(tcp_client_controller_.get());
    user_->Initialize(workThread_->message_loop_proxy());
    return result;
}

void NetworkHelper::Finalize()
{
    chatRepeatingTimer_.Stop();
    workThread_->Stop();
    tcp_client_controller_->Finalize();
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
        NotifyCallback(MessageLevel::MESSAGE_LEVEL_DISPLAY, L"设置重复说话时间要超过30秒");
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

void NetworkHelper::SetRetriveGiftCoin(base::Callback<void(uint32)>& callback)
{
    retrive_gift_coin_callback_ = callback;
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
        std::placeholders::_1, std::placeholders::_2));
    roomid_ = roomid;

    return user_->EnterRoomFopOperation(roomid, &singer_clanid_,
        base::Bind(&NetworkHelper::ConnectionBreakCallback,
        base::Unretained(this)));
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

bool NetworkHelper::GetGiftList(uint32 roomid, std::string* content)
{
    return user_->GetGiftList(roomid, content);
}

// messageNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback(MessageLevel level, const std::wstring& message)
{
    // 解析数据包
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(message, 936);
    if (notify_)
    {
        notify_(level, message);
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

void NetworkHelper::GetCityRankInfos(uint32 roomid)
{
    workThread_->message_loop_proxy()->PostTask(
        FROM_HERE,
        base::Bind(&NetworkHelper::DoGetCityRankInfos, 
        base::Unretained(this), roomid));
}

void NetworkHelper::DoGetCityRankInfos(uint32 roomid)
{
    PhoneRank phone_rank;
    std::wstring display_msg;
    phone_rank.GetCityRankInfos(roomid,
        base::Bind(&NetworkHelper::NotifyCallback,
        base::Unretained(this),
        MessageLevel::MESSAGE_LEVEL_DISPLAY), &display_msg);

    if (display_msg.empty())
        return;

    SendChatMessage(roomid, base::WideToUTF8(display_msg));
}

void NetworkHelper::SetHandleChatUsers(bool handleall501)
{
    handleall501_ = handleall501;
}

bool NetworkHelper::GetActionPrivilege(std::wstring* message)
{
    // 三个值都是0，不正确
    if (!authority_->roomid && !authority_->userid && !authority_->clanid)
    {
        *message = L"授权信息错误!";
        return false;
    }

    // 如果指定了繁星号，则要判断用户权限
    if (authority_->userid && (user_->GetFanxingId() != authority_->userid))
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

    if (authority_->clanid && singer_clanid_ != authority_->clanid)
    {
        *message = L"当前授权仅限在指定公会的主播房间中使用!";
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
    std::wstring message;
    if (!GetActionPrivilege(&message))
    {
        LOG(INFO) << __FUNCTION__ << message.c_str();
        return;
    }

    if (!roomgiftinfo601.token.empty())
    {
        // 原本是抢币的动作
        //std::string error_msg;
        //uint32 coin_count = 0;
        //for (uint32 count = 0; count < 10; count++)
        //{
        //    uint32 coin = 0;
        //    if (!user_->RetrieveHappyFreeCoin(roomid, roomgiftinfo601.token, &coin, &error_msg))
        //        break;

        //    coin_count += coin;
        //}

        //if (coin_count && !retrive_gift_coin_callback_.is_null())
        //    retrive_gift_coin_callback_.Run(coin_count);
    }

    std::wstring chatmsg;
    if (!giftStrategy_->GetGiftThanks(roomgiftinfo601, &chatmsg))
        return;

    user_->SendChatMessage(roomid_, base::WideToUTF8(chatmsg));

    if (notify_)
        notify_(MessageLevel::MESSAGE_LEVEL_DISPLAY, chatmsg);
}

void NetworkHelper::NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo)
{
    if (notify201_)
    {
        enterRoomUserInfoMap_[enterRoomUserInfo.userid] = enterRoomUserInfo;
        RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
        notify201_(rowdata);
    }

    std::wstring message;
    if (!GetActionPrivilege(&message))
    {
        LOG(INFO)<<__FUNCTION__<< message.c_str();
        return;
    }

    TryHandleUser(enterRoomUserInfo);

    std::wstring chatmsg;
    std::wstring private_msg; // 发送给主播的特别通知
    if (!enterRoomStrategy_->GetEnterWelcome(enterRoomUserInfo, &chatmsg, &private_msg))
        return;

    user_->SendChatMessage(roomid_, base::WideToUTF8(chatmsg));

    if (!private_msg.empty())
    {
        user_->SendPrivateMessageToSinger(roomid_, base::WideToUTF8(private_msg));
    }

    if (notify_)
    {
        notify_(MessageLevel::MESSAGE_LEVEL_DISPLAY, chatmsg);
    }
}

void NetworkHelper::NotifyCallback501(const EnterRoomUserInfo& enterRoomUserInfo,
    const RoomChatMessage& roomChatMessage)
{
    if (notify501_)
    {
        enterRoomUserInfoMap_[enterRoomUserInfo.userid] = enterRoomUserInfo;
        RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
        notify501_(rowdata);
    }

    std::wstring message;
    if (!GetActionPrivilege(&message))
    {
        LOG(INFO) << __FUNCTION__ << message.c_str();
        return;
    }
    
    TryHandleUser(enterRoomUserInfo);
    RobotHandleChatMessage(enterRoomUserInfo, roomChatMessage);
    TryHandle501Msg(enterRoomUserInfo, roomChatMessage);
}

void NetworkHelper::ConnectionBreakCallback()
{

}

void NetworkHelper::SetHandleRichLevel(uint32 rich_level)
{
    antiStrategy_->SetHandleRichLevel(rich_level);
}

void NetworkHelper::TryHandleUser(const EnterRoomUserInfo& enterRoomUserInfo)
{
    if (enterRoomUserInfo.isAdmin)// 不处理管理员
        return;

    HANDLE_TYPE handletype = antiStrategy_->GetUserHandleType(
        enterRoomUserInfo.richlevel, enterRoomUserInfo.nickname);
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

void NetworkHelper::TryHandle501Msg(const EnterRoomUserInfo& enterRoomUserInfo,
    const RoomChatMessage& roomChatMessage)
{
    if (enterRoomUserInfo.isAdmin)// 不处理管理员
        return;

    HANDLE_TYPE handletype = HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    if (!handleall501_) // 仅处理关键词
    {
        handletype = antiStrategy_->GetMessageHandleType(
            enterRoomUserInfo.richlevel, roomChatMessage.chatmessage);
    }
    else
    {
        handletype = antiStrategy_->GetHandleType(enterRoomUserInfo.richlevel);
    }

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
    
    // 临时解决方案, 因为如果是勾上自动重复发言再进入房间，此处未得到连接成功的前置条件，所以暂时延时10秒发送第一条消息。
    workThread_->message_loop_proxy()->PostDelayedTask(
        FROM_HERE, base::Bind(&NetworkHelper::DoChatRepeat,
        base::Unretained(this), chatmsg), base::TimeDelta::FromSeconds(10));

    chatRepeatingTimer_.Start(
        FROM_HERE, base::TimeDelta::FromSeconds(static_cast<uint64>(seconds)),
        base::Bind(&NetworkHelper::DoChatRepeat, this, chatmsg));
}

void NetworkHelper::DoChatRepeat(const std::wstring& chatmsg)
{
    user_->SendChatMessage(roomid_, base::WideToUTF8(chatmsg));
}

