#include "stdafx.h"
#include "NetworkHelper.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "../Network/GiftNotifyManager.h"
#include "../Network/CurlWrapper.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

namespace
{
    uint32 userid;
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
    CurlWrapper::CurlInit();
}


NetworkHelper::~NetworkHelper()
{
    CurlWrapper::CurlCleanup();
}


bool NetworkHelper::Initialize()
{
    return false;
}
void NetworkHelper::Finalize()
{
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
    uint32 roomid = 0;
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

// giftNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback(const std::string& data)
{
    // 解析数据包


    if (notify_)
    {
        notify_(data);
    }
}

// giftNotifyManager_ 线程回调
void NetworkHelper::NotifyCallback601(const std::string& data)
{
    bool ret = curlWrapper_->GiftService_GiftService(userid, data);
    return;
}
