#pragma once
#include <memory>
#include <map>

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file.h"
#include "Network/EncodeHelper.h"
//typedef std::vector<std::wstring> RowData;
//typedef std::vector<RowData> GridData;

struct BlackInfo
{
    uint32 userid;
    std::string nickname;
};
// 处理黑名单的存取,合并文件等功能.
class BlacklistHelper
{
public:
    BlacklistHelper();
    ~BlacklistHelper();

    bool Initialize();
    void Finalize();

    bool LoadBlackList(std::vector<RowData>* rowdata);
    bool SaveBlackList(const std::vector<RowData>& rowdata);

    bool LoadFromFile(std::map<uint32, BlackInfo>* blackInfoMap);
private:
    bool SaveToFile(const std::map<uint32, BlackInfo>& blackInfoMap) const;

    std::map<uint32, BlackInfo> blackInfoMap_;
};

