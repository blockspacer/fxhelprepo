#include "stdafx.h"
#include "NetworkHelper.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "../Network/GiftNotifyManager.h"
#include "../Network/CurlWrapper.h"
#include "../Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"

namespace
{
    std::string MakeFormatTimeString(const base::Time time)
    {
        base::Time::Exploded exploded;
        time.LocalExplode(&exploded);
        std::string hour = base::IntToString(exploded.hour);
        if (hour.length() < 2)
        {
            hour = "0" + hour;
        }
        std::string minute = base::IntToString(exploded.minute);
        if (minute.length() < 2)
        {
            minute = "0" + minute;
        }

        std::string second = base::IntToString(exploded.second);
        if (second.length() < 2)
        {
            second = "0" + second;
        }

        std::string millisecond = base::IntToString(exploded.millisecond);
        std::string timestring = hour + ":" + minute + ":" + second+ "." + millisecond;

        return std::move(timestring);
    }

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
    //if (!curlWrapper_->Servies_Uservice_UserService_getCurrentUserInfo(
    //    roomid, &userid, &nickname, &richlevel))
    //{
    //    return false;
    //}

    //ret = curlWrapper_->RoomService_RoomService_enterRoom(
    //    static_cast<uint32>(roomid));
    //assert(ret);
    //if (!ret)
    //{
    //    return false;
    //}

    //ret = curlWrapper_->ExtractStarfulInfo_RoomService_enterRoom(
    //    &staruserid, &key, &ext);

    //if (!ret)
    //{
    //    return false;
    //}

    ret = curlWrapper_->EnterRoom(roomid, &staruserid);
    assert(ret);
    if (!ret)
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
        this, std::placeholders::_1, std::placeholders::_2));

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
    if (!curlWrapper_->LoginRequestWithUsernameAndPassword(WideToUtf8(username), WideToUtf8(password)))
    {
        return false;
    }

    // 获取cookie部分内容
    if (!curlWrapper_->Services_UserService_UserService_getMyUserDataInfo())
    {
        return false;
    }

    // 获取cookie部分内容
    if (!curlWrapper_->Services_IndexService_IndexService_getUserCenter())
    {
        return false;
    }
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

// giftNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback601(uint32 roomid, const std::string& data)
{
    std::wstring responsedata;
    for (int i = 0; i < 20; i++)
    {
        bool ret = curlWrapper_->GiftService_GiftService(
            roomid, data, &responsedata);
        if (notify_)
        {
            notify_(responsedata);
        }
    }
    
    return;
}

void NetworkHelper::NotifyCallback201(const EnterRoomUserInfo& enterRoomUserInfo)
{
    enterRoomUserInfoMap_[enterRoomUserInfo.userid] = enterRoomUserInfo;
    RowData rowdata = EnterRoomUserInfoToRowdata(enterRoomUserInfo);
    notify201_(rowdata);
}
