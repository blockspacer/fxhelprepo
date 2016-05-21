#include "stdafx.h"
#include "NetworkHelper.h"
#include "Network/User.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "Network/MessageNotifyManager.h"
#include "GiftInfoHelper.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

GiftNetworkHelper::GiftNetworkHelper()
    :curlWrapper_(new CurlWrapper)
    , messageNotifyManager_(new MessageNotifyManager)
    , giftInfoHelper_(new GiftInfoHelper)
{
    
}


GiftNetworkHelper::~GiftNetworkHelper()
{
    
}

bool GiftNetworkHelper::Initialize()
{
    CurlWrapper::CurlInit();
    curlWrapper_->Initialize();
    messageNotifyManager_->Initialize();
    return true;
}

void GiftNetworkHelper::Finalize()
{
    messageNotifyManager_->Finalize();
    curlWrapper_->Finalize();
    CurlWrapper::CurlCleanup();
    return;
}

void GiftNetworkHelper::SetNotify(notifyfn fn)
{
    notify_ = fn;
}

void GiftNetworkHelper::RemoveNotify()
{
    notify_ = nullptr;
}

void GiftNetworkHelper::SetNotify201(notify201 fn)
{
    notify201_ = fn;
}

void GiftNetworkHelper::RemoveNotify201()
{
    notify201_ = nullptr;
}

void GiftNetworkHelper::SetNotify501(notify501 fn)
{
    notify501_ = fn;
}

void GiftNetworkHelper::RemoveNotify501()
{
    notify501_ = nullptr;
}

void GiftNetworkHelper::SetNotify502(notify502 fn)
{
    notify502_ = fn;
}
void GiftNetworkHelper::RemoveNotify502()
{
    notify502_ = nullptr;
}


void GiftNetworkHelper::SetNotify601(notify601 fn)
{
    notify601_ = fn;
}

void GiftNetworkHelper::RemoveNotify601()
{
    notify601_ = nullptr;
}

bool GiftNetworkHelper::EnterRoom(const std::wstring& strroomid, uint32* singerid)
{
    uint32 roomid = 0;
    base::StringToUint(strroomid, &roomid);

    return EnterRoom(roomid, singerid);
}

bool GiftNetworkHelper::EnterRoom(uint32 roomid, uint32* singerid)
{
    uint32 userid = 0;
    std::string nickname = "";
    uint32 richlevel = 0;
    uint32 ismaster = 0;
    uint32 staruserid = 0;
    std::string key = "";
    std::string ext = "";

    bool ret = false;
    ret = curlWrapper_->EnterRoom(roomid, &staruserid);
    assert(ret);
    assert(staruserid);
    if (!ret || !staruserid)
    {
        return false;
    }

    ret = ConnectToNotifyServer_(roomid, userid, nickname, richlevel, ismaster, 
                                 staruserid, key, ext);

    *singerid = staruserid;
    return ret;
}

bool GiftNetworkHelper::ConnectToNotifyServer(uint32 roomid, uint32 userid, 
                                          const std::string& nickname, 
                                          uint32 richlevel, uint32 ismaster, 
                                          uint32 staruserid, 
                                          const std::string& key, 
                                          const std::string& ext)
{
    return ConnectToNotifyServer_(roomid, userid, nickname, richlevel, 
                                  ismaster, staruserid, key, ext);
}

bool GiftNetworkHelper::ConnectToNotifyServer_(uint32 roomid, uint32 userid,
                                           const std::string& nickname,
                                           uint32 richlevel, uint32 ismaster,
                                           uint32 staruserid,
                                           const std::string& key,
                                           const std::string& ext)
{
    bool ret = false;
    ret = messageNotifyManager_->Connect843();
    assert(ret);

    messageNotifyManager_->SetNotify201(
        std::bind(&GiftNetworkHelper::NotifyCallback201,
        this, std::placeholders::_1));

    messageNotifyManager_->SetNotify501(
        std::bind(&GiftNetworkHelper::NotifyCallback501,
        this, std::placeholders::_1));

    messageNotifyManager_->SetNotify601(
        std::bind(&GiftNetworkHelper::NotifyCallback601,
        this, roomid, staruserid, std::placeholders::_1));

    messageNotifyManager_->SetNormalNotify(
        std::bind(&GiftNetworkHelper::NotifyCallback,
        this, std::placeholders::_1));

    ret = messageNotifyManager_->Connect8080_NotLogin(
        roomid, userid, nickname, richlevel, ismaster, staruserid, key, ext);
    return ret;
}

bool GiftNetworkHelper::GetGiftList(uint32 roomid)
{
    std::string responsedata;
    if (!curlWrapper_->GetGiftList(roomid, &responsedata))
    {
        return false;
    }

    return giftInfoHelper_->Initialize(responsedata);
}

// messageNotifyManager_ 线程回调
void GiftNetworkHelper::NotifyCallback(const std::wstring& message)
{
    // 解析数据包
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(message, 936);
    if (notify_)
    {
        notify_(message);
    }
}

// messageNotifyManager_ 线程回调
void GiftNetworkHelper::NotifyCallback601(uint32 roomid, uint32 singerid, const RoomGiftInfo601& roomgiftinfo601)
{
    if (!notify601_)
        return;

    // 如果不是在本房间送给主播的消息，过滤掉不回调
    if ((roomid!=roomgiftinfo601.roomid)
        ||(singerid != roomgiftinfo601.receiverid))
    {
        return;
    }

    if (!roomgiftinfo601.token.empty())
    {
        // 原本是抢币的动作，目前不做这类功能
    }
    GiftInfo giftinfo;
    bool result = giftInfoHelper_->GetGiftInfo(roomgiftinfo601.giftid, &giftinfo);
    notify601_(roomgiftinfo601, giftinfo);
}

void GiftNetworkHelper::NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo)
{
    if (!notify201_)
        return;
}

void GiftNetworkHelper::NotifyCallback501(const EnterRoomUserInfo& enterRoomUserInfo)
{
    if (!notify501_)
        return;
}

