#include "stdafx.h"
#include "NetworkHelper.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "../Network/GiftNotifyManager.h"
#include "../Network/CurlWrapper.h"
#include "../Network/EncodeHelper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

namespace
{
    uint32 userid;
    uint32 roomid;
    std::string nickname;
    uint32 richlevel;
    uint32 ismaster;
    uint32 staruserid;
    std::string key;
    std::string ext;
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

bool NetworkHelper::EnterRoom(const std::wstring& strroomid)
{
    LOG(INFO) << L"EnterRoom " << strroomid;
    base::StringToUint(strroomid, &roomid);

    bool ret = false;
    ret = curlWrapper_->RoomService_RoomService_enterRoom(
        static_cast<uint32>(roomid));
    assert(ret);
    if (!ret)
    {
        return false;
    }

    ret = curlWrapper_->ExtractUsefulInfo_RoomService_enterRoom(&userid,
        &nickname, &richlevel, &staruserid, &key, &ext);

    if (!ret)
    {
        return false;
    }

    ret = giftNotifyManager_->Connect843();
    assert(ret);
    giftNotifyManager_->Set601Notify(
        std::bind(&NetworkHelper::NotifyCallback601, 
        this, std::placeholders::_1));

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
    return curlWrapper_->LoginRequestWithUsernameAndPassword(
        WideToUtf8(username), WideToUtf8(password));
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

// giftNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback601(const std::string& data)
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
