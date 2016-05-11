#pragma once

#include "third_party/chromium/base/basictypes.h"

struct Authority
{
    uint32 userid;
    uint32 roomid;
    uint32 clanid;
    uint32 kickout;
    uint32 banchat;
    uint32 antiadvance;
};

// 提供授权文件明文格式读写功能
class AuthorityHelper
{
public:
    AuthorityHelper();
    ~AuthorityHelper();

    bool Load(Authority* authority);
    bool Save(const Authority& authority);

};

