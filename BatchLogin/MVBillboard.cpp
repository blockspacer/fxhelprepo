#include "stdafx.h"
#include <string>
#undef max
#undef min
#include "Network/CurlWrapper.h"
#include "Network/easy_http_impl.h"
#include "MVBillboard.h"
#include "Network/EncodeHelper.h"

MVBillboard::MVBillboard()
    :curlWrapper_(new CurlWrapper)
    , easyHttpImpl_(new EasyHttpImpl)
{
    
}

MVBillboard::~MVBillboard()
{
    easyHttpImpl_->ShutdownService();
}

bool MVBillboard::UpMVBillboard(const std::string& cookie,
    const std::string& collectionid,
    const std::string& mvid)
{
    bool result = true;
    result &= MVPlayVedio(cookie, collectionid, mvid);
    result &= MVAddPraise(cookie, collectionid, mvid);
    result &= MVCollect(cookie, collectionid, mvid);
    result &= MVShared(cookie, collectionid, mvid);
    return result;
}

bool MVBillboard::MVPlayVedio(const std::string& cookie,
    const std::string& collectionid,
    const std::string& mvid)
{
    std::string action = "playVideo";
    std::string url = "http://fanxing.kugou.com/NServices/Video/OfflineVideoService/" + action;
    return MVAction2_(url, cookie, collectionid, mvid);
}

bool MVBillboard::MVAddPraise(const std::string& cookie,
    const std::string& collectionid,
    const std::string& mvid)
{
    std::string action = "addPraise";
    std::string url = "http://fanxing.kugou.com/NServices/Video/OfflineVideoService/" + action;
    return MVAction2_(url, cookie, collectionid, mvid);
}

bool MVBillboard::MVCollect(const std::string& cookie,
    const std::string& collectionid,
    const std::string& mvid)
{
    std::string action = "mvCollect";
    std::string url = "http://fanxing.kugou.com/NServices/Video/OfflineVideoPlayService/" + action;
    return MVAction2_(url, cookie, collectionid, mvid);
}

bool MVBillboard::MVShared(const std::string& cookie, 
    const std::string& collectionid,
    const std::string& mvid)
{
    std::string action = "mvShare";
    std::string url = "http://fanxing.kugou.com/NServices/Video/OfflineVideoPlayService/" + action;
    return MVAction2_(url, cookie, collectionid, mvid);
}

//bool MVBillboard::MVAction_(const std::string& url, const std::string& cookie,
//    const std::string& collectionid, const std::string& mvid)
//{
//    HttpRequest request;
//    request.url = url;
//    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
//    request.queries["args"] = "[%22" + mvid + "%22,%22web%22]";
//    request.queries["_"] = GetNowTimeString();
//    request.referer = "http://fanxing.kugou.com/mvplay/" + collectionid;
//    request.cookies = cookie;
//    HttpResponse response;
//    if (!curlWrapper_->Execute(request, &response))
//        return false;
//
//    std::string rootdata(response.content.begin(), response.content.end());
//    //½âÎöjsonÊý¾Ý
//    Json::Reader reader;
//    Json::Value root(Json::objectValue);
//    if (!reader.parse(rootdata, root, false))
//    {
//        return false;
//    }
//
//    uint32 status = GetInt32FromJsonValue(root, "status");
//    if (status != 1)
//        return false;
//
//    bool result = root.get("data", false).asBool();
//    std::string errorcode = root.get("errorcode", false).asString();
//    return result;
//}
bool MVBillboard::MVAction2_(const std::string& url, const std::string& cookie,
    const std::string& collectionid, const std::string& mvid)
{
    HttpRequest request;
    request.url = url;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.queries["args"] = "[" + mvid + ",%22web%22]";
    request.queries["_"] = GetNowTimeString();
    request.referer = "http://fanxing.kugou.com/mvplay/" + collectionid;
    request.cookies = cookie;

    easyHttpImpl_->AsyncHttpRequest(request);
    return true;
}
