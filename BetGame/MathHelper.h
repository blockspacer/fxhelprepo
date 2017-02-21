#pragma once
#include <vector>
#include "third_party/chromium/base/basictypes.h"

class MathHelper
{
public:
    MathHelper();
    ~MathHelper();

    static uint32 GetSum(const std::vector<uint32>& data);
    static double GetAvg(const std::vector<uint32>& data);
    static uint32 GetMid(const std::vector<uint32>& data);
    static double GetStandardDeviation(const std::vector<uint32>& data, double avg);
};

