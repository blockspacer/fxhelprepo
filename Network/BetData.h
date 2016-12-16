#include "third_party/chromium/base/basictypes.h"

struct BetShowData
{
    uint32 bet_gid;
    uint32 odds;
    //uint32 count;
};

struct BetResult
{
    uint32 time = 0;
    uint32 result = 0;
    uint32 display_result = 0;
    uint32 random = 0;

    bool operator<(const BetResult& other) const
    {
        return this->time < other.time;
    }

    bool operator==(const BetResult& other) const
    {
        return this->time == other.time;
    }
};