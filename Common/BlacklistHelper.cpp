
#include "BlacklistHelper.h"
#include <fstream>

#include "Network/EncodeHelper.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file.h"
#include "third_party/chromium/base/files/file_util.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/json/json.h"

typedef std::vector<std::wstring> RowData;
namespace
{
    RowData BlackInfoToRowdata(const BlackInfo& enterRoomUserInfo)
    {
        RowData rowdata;
        rowdata.push_back(base::UTF8ToWide(enterRoomUserInfo.nickname));
        rowdata.push_back(base::UintToString16(enterRoomUserInfo.userid));
        return rowdata;
    }
}
BlacklistHelper::BlacklistHelper()
{

}


BlacklistHelper::~BlacklistHelper()
{
}

bool BlacklistHelper::Initialize()
{
    return false;
}

void BlacklistHelper::Finalize()
{
    return;
}

bool BlacklistHelper::LoadBlackList(const std::wstring& path_filename, 
    std::vector<RowData>* rowdatas)
{
    std::map<uint32, BlackInfo> blackInfoMap;
    if (!LoadFromFile(path_filename, &blackInfoMap))
        return false;
    
    for (const auto& blackInfo : blackInfoMap)
    {
        RowData rawdata = BlackInfoToRowdata(blackInfo.second);
        rowdatas->push_back(rawdata);
    }
    return true;
}

bool BlacklistHelper::SaveBlackList(const std::wstring& path_filename, 
    const std::vector<RowData>& rowdatas)
{
    std::map<uint32, BlackInfo> blackInfoMap;
    for (const auto& rowdata : rowdatas)
    {
        if (rowdata.size() !=2)
            return false;
        BlackInfo blackInfo;
        blackInfo.nickname = base::WideToUTF8(rowdata[0]);
        std::string struserid = base::WideToUTF8(rowdata[1]);
        blackInfo.userid = 0;
        if (!base::StringToUint(struserid, &blackInfo.userid))
            return false;
        blackInfoMap[blackInfo.userid] = blackInfo;
    }

    bool result = SaveToFile(path_filename, blackInfoMap);
    return result;
}

bool BlacklistHelper::LoadFromFile(const std::wstring& path_filename, 
    std::map<uint32, BlackInfo>* blackInfoMap)
{
    if (!blackInfoMap)
        return false;

    base::FilePath dirPath(path_filename);

    std::string data;
    if (!base::ReadFileToString(dirPath, &data))
        return false;
    
    try
    {
        Json::Reader reader;
        Json::Value root(Json::objectValue);
        if (!Json::Reader().parse(data.c_str(), root))
        {
            assert(false && L"Json::Reader().parse error");
            return false;
        }

        if (!root.isArray())
        {
            assert(false && L"root is not array");
            return false;
        }

        for (const auto& value : root)//for data
        {
            Json::Value temp;
            uint32 userid = GetInt32FromJsonValue(value, "userid");
            std::string nickname = value.get("nickname", "nick").asString();
            BlackInfo blackInfo = { userid, nickname };
            blackInfoMap_[userid] = blackInfo;
        }
    }
    catch (...)
    {
        return false;
    }
    *blackInfoMap = blackInfoMap_;
    return true;
}

bool BlacklistHelper::SaveToFile(const std::wstring& path_filename,
    const std::map<uint32, BlackInfo>& blackInfoMap) const
{
    base::FilePath pathname(path_filename);

    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& blackInfo : blackInfoMap)
    {
        Json::Value jvBlackInfo(Json::objectValue);
        jvBlackInfo["userid"] = blackInfo.second.userid;
        jvBlackInfo["nickname"] = blackInfo.second.nickname;
        root.append(jvBlackInfo);
    }

    std::string writestring = writer.write(root);

    std::ofstream ofs(pathname.value(), std::ios_base::out);
    if (!ofs)
        return false;

    ofs << writestring;
    ofs.flush();
    ofs.close();

    return true;
}
