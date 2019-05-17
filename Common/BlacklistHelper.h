#pragma once
#include <memory>
#include <map>

#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
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
// ����������Ĵ�ȡ,�ϲ��ļ��ȹ���.
class BlacklistHelper
{
public:
    BlacklistHelper();
    ~BlacklistHelper();

    bool Initialize();
    void Finalize();

    bool LoadBlackList(const std::wstring& path_filename, std::vector<RowData>* rowdata);
    bool SaveBlackList(const std::wstring& path_filename, const std::vector<RowData>& rowdata);
    bool LoadFromFile(
        const std::wstring& path_filename, std::map<uint32, BlackInfo>* blackInfoMap);
private:
    bool SaveToFile(const std::wstring& path_filename, 
        const std::map<uint32, BlackInfo>& blackInfoMap) const;

    std::map<uint32, BlackInfo> blackInfoMap_;
};

