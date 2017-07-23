#pragma once
#include <string>
#include <memory>
#include <map>

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/time/time.h"

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;
class FamilyBackground;
class SingerSummaryData;
class FamilyDataController
{
public:
    FamilyDataController();
    virtual ~FamilyDataController();

    bool Login(const std::string& username, const std::string& password);
    bool Login(const std::wstring& wusername, const std::wstring& wpassword);
    bool GetSingerFamilyData(const base::Time& begintime,
                             const base::Time& endtime,
                             GridData* griddata);

    bool GetDailyDataBySingerId(uint32 singerid, const base::Time& begintime,
        const base::Time& endtime, GridData* griddata);

    // ��ȡ����������Ч�����������ݣ�ͨ���ֱ��ÿ����ʽ������������ȥ������
    bool GetFamilyOpenDayCountSummary(const base::Time& begintime,
        const base::Time& endtime, GridData* griddata);

    bool ExportToExcel();

    bool ExportToTxt();
private:

    bool GetNormalSingerIds(std::vector<uint32>* singers);
    base::FilePath exePath_;
    std::unique_ptr<FamilyBackground> familyBackground_;
    std::map<uint32, SingerSummaryData> singer_summary_map_;

    GridData display_griddata_;
};

