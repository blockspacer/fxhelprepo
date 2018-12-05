#pragma once

#include <vector>
#include <string>
#include <map>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/base/single_thread_task_runner.h"
#include "third_party/chromium/base/memory/scoped_ptr.h"
#include "third_party/chromium/base/bind.h"

#include "RankTypes.h"

namespace base
{
    class SingleThreadTaskRunner;
}

class CitySingersManager
{
public:
    CitySingersManager();
    ~CitySingersManager();

    bool Initialize(base::SingleThreadTaskRunner* runner);
    void Finalize();

    bool FindCity(const std::string& city_name, CityInfo* city_info);

    void ClearCityRankCache();
    bool GetRankSingerListByCity(const CityInfo& city_info, std::vector<RankSingerInfo>* rank_singer_infos) const;
    bool GetSinglePageDataByCity(
        const std::map<std::string, std::string>& query_city_rank_param,
        std::vector<RankSingerInfo>* rank_singer_infos,
        bool* has_next_page, uint32* online_number) const;

private:
    bool GetCityInfos();

    scoped_refptr<base::SingleThreadTaskRunner> runner_;

    std::map<std::string, std::vector<CityInfo>> province_citys_;
    std::map<std::string, std::vector<RankSingerInfo>> city_singers_rank_;

};

