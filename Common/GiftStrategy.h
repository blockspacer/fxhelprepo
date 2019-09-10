#pragma once

#include <memory>
#include <string>
#include "Network/common.h"
#include "Common/CommonTypedef.h"

class GiftStrategy
    :public std::enable_shared_from_this < GiftStrategy >
{
public:
    GiftStrategy();
    ~GiftStrategy();

    bool Initialize(const std::string& content);
    void SetThanksFlag(bool enable);
    void SetThanksGiftValue(uint32 gift_value);
    bool GetGiftThanks(const RoomGiftInfo601& giftinfo, std::wstring* chatmessage);

    // 设置自己给对自己送礼物的人限制显示其礼物一段时间
    void SetBanGiftSecondAndValue(uint32 gift_value, uint32 seconds);
    bool GetBanDisplaySeconds(const RoomGiftInfo601& giftinfo, uint32* seconds,
        uint32* ban_gift_value);
    void SetSendToSelfHandle(bool handle);
    bool GetSendToSelfHandle() const;

private:

    struct GiftInfo
    {
        uint32 giftid = 0;
        std::string giftname = "";
        uint32 price = 0;
        double exchange = false;
        uint32 category = 0;
        std::string categoryname = "";
    };

    std::map<uint32, GiftInfo> giftmap_;
    std::map<uint32, std::string> category_;
    std::vector<uint32> ban_gift_setting_values_;

    uint32 thank_gift_value_ = 0;
    bool thanksflag_ = false;
    uint32 ban_gift_value_ = 0;
    uint32 ban_gift_seconds_ = 0;

    bool handle_send_gift_to_self = true;
};

