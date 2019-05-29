#include "stdafx.h"
#include <fstream>
#include "NetworkHelper.h"

#include "PhoneRank.h"
//#include "Network/TcpClientController.h"
#include "Network/User.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
#include "third_party/chromium/base/time/time.h"
#include "Network/MessageNotifyManager.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/path_service.h"
#include "Network/WebsocketClientController.h"
#include "Common/CommonTypedef.h"
#include "Common/GiftStrategy.h"
#include "Common/AntiStrategy.h"

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

NetworkHelper::NetworkHelper()
    : authority_(new AntiFloodAuthority)
    //, tcp_client_controller_(new TcpClientController)
    , websocket_client_controller_(new WebsocketClientController)
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
    //tcp_client_controller_->Initialize();
    AuthorityHelper authorityHelper;
    bool result = authorityHelper.LoadAntiFloodAuthority(authority_.get());
    assert(!authority_->serverip.empty());
    user_->Initialize(workThread_->message_loop_proxy());
    //user_->SetRoomServerIp(authority_->serverip);
    user_->SetRoomServerIp("119.146.204.164");
    //user_->SetTcpManager(tcp_client_controller_.get());
    websocket_client_controller_->Initialize();
    user_->SetWebsocketClientController(websocket_client_controller_.get());
    user_->Initialize(workThread_->message_loop_proxy());
    return result;
}

void NetworkHelper::Finalize()
{
    chatRepeatingTimer_.Stop();
    workThread_->Stop();
    //tcp_client_controller_->Finalize();
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
        NotifyCallback(MessageLevel::MESSAGE_LEVEL_DISPLAY, L"�����ظ�˵��ʱ��Ҫ����30��");
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

bool NetworkHelper::LoginWithCookies(const std::string& cookies,
    std::string* errormsg)
{
    return user_->LoginWithCookies(cookies, errormsg);
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
    std::wstring loginInfo = L"��¼��Ϣ: ";
    loginInfo += L"���Ǻ�: " + base::UintToString16(fanxingid);
    loginInfo += L" ����ID: " + base::UintToString16(clanid);
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

// messageNotifyManager_ �̻߳ص�
void NetworkHelper::NotifyCallback(MessageLevel level, const std::wstring& message)
{
    // �������ݰ�
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

bool NetworkHelper::BanEnter(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    return user_->BanEnter(roomid, enterRoomUserInfo);
}

bool NetworkHelper::UnbanEnter(uint32 roomid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    return user_->UnbanEnter(roomid, enterRoomUserInfo);
}

// ���÷����ǵ��Ǳ�������ʾ
bool NetworkHelper::SetRoomGiftNotifyLevel(uint32 roomid, uint32 gift_value)
{
    return user_->SetRoomGiftNotifyLevel(roomid, gift_value);
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
//#ifdef _DEBUG
//    return true;
//#endif

    // ����ֵ����0������ȷ
    if (!authority_->roomid && !authority_->userid && !authority_->clanid)
    {
        *message = L"��Ȩ��Ϣ����!";
        return false;
    }

    // ���ָ���˷��Ǻţ���Ҫ�ж��û�Ȩ��
    if (authority_->userid && (user_->GetFanxingId() != authority_->userid))
    {
        *message = L"��ǰ�û�δ��Ȩ!";
        return false;
    }

    uint32 servertime = user_->GetServerTime();
    uint64 expiretime = authority_->expiretime - base::Time::UnixEpoch().ToInternalValue();
    expiretime /= 1000000;
    if (servertime > expiretime)
    {
        *message = L"��ǰ�û���Ȩ�ѹ���!";
        return false;
    }

    if (authority_->clanid && user_->GetClanId() != authority_->clanid)
    {
        *message = L"��ǰ��Ȩ����ָ�������Աʹ��!";
        return false;
    }

    if (authority_->clanid && singer_clanid_ != authority_->clanid)
    {
        *message = L"��ǰ��Ȩ������ָ�����������������ʹ��!";
        return false;
    }

    if (authority_->roomid && (roomid_ != authority_->roomid))
    {
        *message = L"��ǰ��Ȩ����ָ������ʹ��!";
        return false;
    }

    return true;
}

// messageNotifyManager_ �̻߳ص�
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
        // ԭ�������ҵĶ���
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

    // �����Լ����Լ������������ҵ���
    if (giftStrategy_->GetSendToSelfHandle() &&
        (roomgiftinfo601.senderid == roomgiftinfo601.receiverid))
    {
        uint32 gift_value = 0;
        uint32 ban_second = 0;
            
        if (giftStrategy_->GetBanDisplaySeconds(roomgiftinfo601, &ban_second, &gift_value))
        {
            SetRoomGiftNotifyLevel(roomid_, gift_value);
            workThread_->message_loop_proxy()->PostDelayedTask(
                FROM_HERE, base::Bind(
                base::IgnoreResult(&NetworkHelper::SetRoomGiftNotifyLevel),
                base::Unretained(this), roomid_, 0), base::TimeDelta::FromSeconds(ban_second));
        }
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
    std::wstring private_msg; // ���͸��������ر�֪ͨ
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
    if (enterRoomUserInfo.isAdmin)// ���������Ա
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
    if (enterRoomUserInfo.isAdmin)// ���������Ա
        return;

    HANDLE_TYPE handletype = HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    if (!handleall501_) // ������ؼ���
    {
        handletype = antiStrategy_->GetMessageHandleType(
            roomChatMessage.receiverid, enterRoomUserInfo.richlevel, 
            roomChatMessage.chatmessage);
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

    // ���Ƕ�����û�˵����Ϣ��Ҫ����
    if (user_->GetFanxingId() != roomChatMessage.receiverid)
        return;

    // �ӽӿڻ�ȡ�ظ���Ϣ
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
    
    // ��ʱ�������, ��Ϊ����ǹ����Զ��ظ������ٽ��뷿�䣬�˴�δ�õ����ӳɹ���ǰ��������������ʱ��ʱ10�뷢�͵�һ����Ϣ��
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

bool NetworkHelper::SendGiftByCookie(const std::wstring& cookie, uint32 roomid, 
	uint32 send_value)
{
	//user_->SendGiftById(roomid, )
	return true;
}

