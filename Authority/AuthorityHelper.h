#pragma once
#include <string>
#include "third_party/chromium/base/basictypes.h"

struct Authority
{
    uint32 userid = 0;
    uint32 roomid = 0;
    uint32 clanid = 0;
    uint32 kickout = 0;
    uint32 banchat = 0;
    uint32 antiadvance = 0;
    uint64 expiretime = 0;
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

