#pragma once
#include <string>
#include <memory>
#include <map>

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/time/time.h"

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;
class FamilyBackground;
class SingerSummaryData;
class FamilyDataAuthority;

class FamilyDataController
{
public:
    FamilyDataController();
    virtual ~FamilyDataController();

    bool LoadAuthority(std::wstring* diaplay_message);

    bool Login(const std::wstring& wusername, const std::wstring& wpassword);
    bool GetSingerFamilyData(const base::Time& begintime,
                             const base::Time& endtime,
                             GridData* griddata);

    bool GetDailyDataBySingerId(uint32 singerid, const base::Time& begintime,
        const base::Time& endtime, GridData* griddata, uint32* onlineminute,
        uint32* effect_day, double* revenue);

    // 获取所有主播有效开播汇总数据，通过分别查每个正式主播的日数据去做汇总
    bool GetFamilyEffectiveDayCountSummary(const base::Time& begintime,
        const base::Time& endtime, GridData* griddata, uint32* effect_count);

    bool ExportToExcel();

    bool ExportToTxt();
private:
    bool InnerLogin(const std::string& username, const std::string& password);
    bool GetNormalSingerIds(std::vector<uint32>* singers);
    base::FilePath exePath_;
    std::unique_ptr<FamilyBackground> familyBackground_;
    std::map<uint32, SingerSummaryData> singer_summary_map_;
    std::unique_ptr<FamilyDataAuthority> authority_;

    GridData display_griddata_;
};

