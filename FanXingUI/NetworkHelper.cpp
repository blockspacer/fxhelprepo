#include "stdafx.h"
#include "NetworkHelper.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "Network/GiftNotifyManager.h"
#include "GiftInfoHelper.h"
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

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
    :curlWrapper_(new CurlWrapper)
    , giftNotifyManager_(new GiftNotifyManager)
    , giftInfoHelper_(new GiftInfoHelper)
{
    
}


NetworkHelper::~NetworkHelper()
{
    
}

bool NetworkHelper::Initialize()
{
    CurlWrapper::CurlInit();
    curlWrapper_->Initialize();
    giftNotifyManager_->Initialize();
    return true;
}

void NetworkHelper::Finalize()
{
    giftNotifyManager_->Finalize();
    curlWrapper_->Finalize();
    CurlWrapper::CurlCleanup();
    return;
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

bool NetworkHelper::EnterRoom(const std::wstring& strroomid)
{
    uint32 roomid = 0;
    base::StringToUint(strroomid, &roomid);
    return EnterRoom(roomid);
}

bool NetworkHelper::EnterRoom(uint32 roomid)
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

    return ret;
}

bool NetworkHelper::ConnectToNotifyServer(uint32 roomid, uint32 userid, 
                                          const std::string& nickname, 
                                          uint32 richlevel, uint32 ismaster, 
                                          uint32 staruserid, 
                                          const std::string& key, 
                                          const std::string& ext)
{
    return ConnectToNotifyServer_(roomid, userid, nickname, richlevel, 
                                  ismaster, staruserid, key, ext);
}

bool NetworkHelper::ConnectToNotifyServer_(uint32 roomid, uint32 userid,
                                           const std::string& nickname,
                                           uint32 richlevel, uint32 ismaster,
                                           uint32 staruserid,
                                           const std::string& key,
                                           const std::string& ext)
{
    bool ret = false;
    ret = giftNotifyManager_->Connect843();
    assert(ret);

    giftNotifyManager_->SetNotify201(
        std::bind(&NetworkHelper::NotifyCallback201,
        this, std::placeholders::_1));

    giftNotifyManager_->SetNotify601(
        std::bind(&NetworkHelper::NotifyCallback601,
        this, roomid, staruserid, std::placeholders::_1));

    giftNotifyManager_->SetNormalNotify(
        std::bind(&NetworkHelper::NotifyCallback,
        this, std::placeholders::_1));

    ret = giftNotifyManager_->Connect8080(roomid, userid, nickname, richlevel,
                                          ismaster, staruserid, key, ext);
    return ret;
}

bool NetworkHelper::Login(const std::wstring& username, 
    const std::wstring& password)
{
    return true;
}

// giftNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback(const std::wstring& message)
{
    // 解析数据包
    LOG(INFO) << __FUNCTION__ << L" " << base::SysWideToMultiByte(message, 936);
    if (notify_)
    {
        notify_(message);
    }
}

bool NetworkHelper::KickoutUsers(uint32 singerid, const EnterRoomUserInfo& enterRoomUserInfo)
{
    curlWrapper_->KickoutUser(singerid, KICK_TYPE::KICK_TYPE_HOUR, enterRoomUserInfo);
    return false;
}

bool NetworkHelper::GetGiftList(uint32 roomid)
{
    std::string responsedata;
    if (!curlWrapper_->GetGiftList(roomid, &responsedata))
    {
        return false;
    }

    return giftInfoHelper_->Initialize(responsedata);
}

bool NetworkHelper::RegisterGetVerifyCode(std::vector<uint8>* picture)
{
    curlWrapper_->RegisterGetVerifyCode(picture);
    return false;
}

bool NetworkHelper::RegisterCheckUserExist(const std::wstring& username)
{
    return curlWrapper_->RegisterCheckUserExist(WideToUtf8(username));
}

bool NetworkHelper::RegisterCheckUserInfo(const std::wstring& username, const std::wstring& password)
{
    std::string utf8username = base::WideToUTF8(username);
    std::string utf8password = base::WideToUTF8(password);
    return curlWrapper_->RegisterCheckUserInfo(utf8username, utf8password);
}

bool NetworkHelper::RegisterCheckVerifyCode(const std::wstring& verifycode)
{
    std::string utf8verifycode = base::WideToUTF8(verifycode);
    return curlWrapper_->RegisterCheckVerifyCode(utf8verifycode);
}

bool NetworkHelper::RegisterUser(const std::wstring& username, 
    const std::wstring& password, const std::wstring& verifycode)
{
    std::string utf8username = base::WideToUTF8(username);
    std::string utf8password = base::WideToUTF8(password);
    std::string utf8verifycode = base::WideToUTF8(verifycode);
    return curlWrapper_->RegisterUser(utf8username, utf8password, utf8verifycode);
}

// giftNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback601(uint32 roomid, uint32 singerid, const RoomGiftInfo601& roomgiftinfo601)
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

void NetworkHelper::NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo)
{
    if (!notify201_)
        return;

    enterRoomUserInfoMap_[enterRoomUserInfo.userid] = enterRoomUserInfo;
    RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
    notify201_(rowdata);
}
