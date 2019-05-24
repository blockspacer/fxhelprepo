#pragma once

#include <set>
#include <memory>
#include <vector>
#include "third_party/chromium/base/basictypes.h"
#include "Common/CommonTypedef.h"

enum class HANDLE_TYPE
{
    HANDLE_TYPE_NOTHANDLE = 0,
    HANDLE_TYPE_KICKOUT = 1,
    HANDLE_TYPE_BANCHAT = 2
};

class AntiStrategy
    : public std::enable_shared_from_this <AntiStrategy>
{
public:
    AntiStrategy();
    ~AntiStrategy();

    bool AntiStrategy::LoadAntiSetting(std::vector<RowData>* rowdatas);
    bool AntiStrategy::SaveAntiSetting() const;

    HANDLE_TYPE GetUserHandleType(uint32 rich_level, const std::string& nickname) const;
    HANDLE_TYPE GetMessageHandleType(uint32 receiveid, uint32 rich_level, const std::string& message) const;
    HANDLE_TYPE GetHandleType(uint32 rich_level) const;
    void SetHandleType(HANDLE_TYPE handletype);
    void SetHandleRichLevel(uint32 rich_level);

    bool AddSensitive(const std::string& sensitive);
    bool RemoveSensitive(const std::string& sensitive);
    bool AddNickname(const std::string& vestname);
    bool RemoveNickname(const std::string& vestname);
    bool AddReceiveId(const std::string& receiveid);
    bool RemoveReceiveId(const std::string& receiveid);

private:
    std::set<std::string> vestnames_;
    std::set<std::string> sensitives_;
    std::set<uint32> receiveids_;
    HANDLE_TYPE handletype_ = HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
    uint32 rich_level_ = 3;
};