#pragma once

#include <vector>
#include <string>
#include <functional>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include "third_party/chromium/base/basictypes.h"

namespace base
{
    class SingleThreadTaskRunner;
}

class SingerRetriver
{
public:
    SingerRetriver();
    ~SingerRetriver();

    bool Initialize(base::SingleThreadTaskRunner* runner);

    void Finalize();

    void BreakRequest();

    bool GetSingerInfoByClan(uint32 clanid, 
        const std::function<void(const std::vector<uint32>&)>& callback);

private:
    bool DoGetSingerInfoPageByClan(const std::string& url, 
        std::vector<uint32>* roomids, int32* all_page);

    base::SingleThreadTaskRunner* runner_;
};

