#include "GiftStrategy.h"


#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/json/value.h"
#include "third_party/json/reader.h"
#include "third_party/json/writer.h"

#include "third_party/chromium/base/strings/sys_string_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

#include "Network/EncodeHelper.h"


GiftStrategy::GiftStrategy()
    :handle_send_gift_to_self(true)
    , ban_gift_seconds_(60)
    , ban_gift_value_(20)
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

    ban_gift_setting_values_.push_back(5);
    ban_gift_setting_values_.push_back(10);
    ban_gift_setting_values_.push_back(20);
    ban_gift_setting_values_.push_back(50);
    ban_gift_setting_values_.push_back(100);
    ban_gift_setting_values_.push_back(200);

    return true;
}

void GiftStrategy::SetThanksFlag(bool enable)
{
    thanksflag_ = enable;
}

void GiftStrategy::SetThanksGiftValue(uint32 gift_value)
{
    thank_gift_value_ = gift_value;
}

void GiftStrategy::SetBanGiftSecondAndValue(uint32 gift_value, uint32 seconds)
{
    ban_gift_value_ = gift_value;
    ban_gift_seconds_ = seconds;
}


bool GiftStrategy::GetBanDisplaySeconds(const RoomGiftInfo601& giftinfo,
    uint32* seconds, uint32* ban_gift_value)
{
    auto gift_it = giftmap_.find(giftinfo.giftid);
    if (gift_it == giftmap_.end())
        return false;

    if (giftinfo.senderid == giftinfo.receiverid)
    {
        auto gift_values = gift_it->second.price*giftinfo.gitfnumber;
        *seconds = ban_gift_seconds_;

        *ban_gift_value = ban_gift_value_;

        for (auto value : ban_gift_setting_values_)
        {
            if (gift_values < value)
            {
                *ban_gift_value = value;
                return true;
            }
        }
    }
    return false;
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
        LOG(INFO) << __FUNCTION__ << L"gift value = [" << base::UintToString16(gift_value) << L" / " << base::UintToString16(thank_gift_value_) << L"]";
        if (gift_value < thank_gift_value_) // 价值少于设置点的，不发送感谢
            return false;
    }

    std::wstring thanks = L"感谢" + base::UTF8ToWide(giftinfo.sendername) +
        L"送来的" + base::UintToString16(giftinfo.gitfnumber) +
        L"个" + base::UTF8ToWide(giftinfo.giftname);

    chatmessage->assign(thanks.begin(), thanks.end());

    return true;
}


void GiftStrategy::SetSendToSelfHandle(bool handle)
{
    handle_send_gift_to_self = handle;
}

bool GiftStrategy::GetSendToSelfHandle() const
{
    return handle_send_gift_to_self;
}