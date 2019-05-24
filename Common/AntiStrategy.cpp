#include "AntiStrategy.h"

#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/json/value.h"
#include "third_party/json/reader.h"
#include "third_party/json/writer.h"

#include "third_party/chromium/base/strings/sys_string_conversions.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

namespace{
    const wchar_t* filename = L"AntiVestSensitive.txt";

}

AntiStrategy::AntiStrategy()
{

}

AntiStrategy::~AntiStrategy()
{
    SaveAntiSetting();
}

bool AntiStrategy::LoadAntiSetting(std::vector<RowData>* rowdatas)
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname = dirPath.Append(filename);

    std::string data;
    if (!base::ReadFileToString(pathname, &data))
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
            std::string vestname = value.get("vestname", "vestname").asString();
            std::string sensitive = value.get("sensitive", "sensitive").asString();
            if (!vestname.empty())
                vestnames_.insert(vestname);

            if (!sensitive.empty())
                sensitives_.insert(sensitive);
        }
    }
    catch (...)
    {
        return false;
    }

    for (const auto& vestname : vestnames_)
    {
        RowData rawdata;
        rawdata.push_back(L"马甲");
        rawdata.push_back(base::UTF8ToWide(vestname));
        rawdata.push_back(L"");
        rowdatas->push_back(rawdata);
    }

    for (const auto& sensitive : sensitives_)
    {
        RowData rawdata;
        rawdata.push_back(L"敏感词");
        rawdata.push_back(L"");
        rawdata.push_back(base::UTF8ToWide(sensitive));
        rowdatas->push_back(rawdata);
    }

    return true;
}

bool AntiStrategy::SaveAntiSetting() const
{
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    base::FilePath pathname = dirPath.Append(filename);

    Json::FastWriter writer;
    Json::Value root(Json::arrayValue);

    for (const auto& vestname : vestnames_)
    {
        Json::Value vestinfo(Json::objectValue);
        vestinfo["vestname"] = vestname;
        vestinfo["sensitive"] = "";
        root.append(vestinfo);
    }

    for (const auto& sensitive : sensitives_)
    {
        Json::Value vestinfo(Json::objectValue);
        vestinfo["vestname"] = "";
        vestinfo["sensitive"] = sensitive;
        root.append(vestinfo);
    }

    std::string writestring = writer.write(root);

    base::FilePath pathfilename(pathname);
    base::WriteFile(pathfilename, writestring.c_str(), writestring.size());
    return true;
}

HANDLE_TYPE AntiStrategy::GetUserHandleType(uint32 rich_level,
    const std::string& nickname) const
{
    if (rich_level >= rich_level_)// 指定等级以上的不处理
        return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    std::wstring w_nickname = base::UTF8ToWide(nickname);
    for (const auto& it : vestnames_)
    {
        std::wstring target = base::UTF8ToWide(it);
        if (target.size() < 2)
        {
            continue;
        }
        bool pre = true;
        bool post = true;
        if (target.rfind(L"*") == target.size() - 1)
        {
            post = false;
            target = target.substr(0, target.size() - 1);
        }

        if (target.find(L"*") == 0)
        {
            pre = false;
            target = target.substr(1, target.size() - 1);
        }

        if (!pre && !post)
        {
            if (w_nickname.find(target) != std::string::npos)
                return handletype_;
        }

        if (pre)
        {
            if (w_nickname.find(target) == 0)
                return handletype_;
        }

        if (post)
        {
            if (w_nickname.rfind(target) == (w_nickname.size() - target.size()))
                return handletype_;
        }

    }
    return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
}

HANDLE_TYPE AntiStrategy::GetMessageHandleType(uint32 receiveid,
    uint32 rich_level, const std::string& message) const
{
    if (rich_level >= rich_level_)// 指定等级以上的不处理
        return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    for (const auto& it : sensitives_)
    {
        if (message.find(it) != std::string::npos)
            return handletype_;
    }

    uint32 id = receiveid;
    auto it = receiveids_.find(id);
    if (it != receiveids_.end())
        return handletype_;

    return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;
}

HANDLE_TYPE AntiStrategy::GetHandleType(uint32 rich_level) const
{
    if (rich_level >= rich_level_)// 指定等级以上的不处理
        return HANDLE_TYPE::HANDLE_TYPE_NOTHANDLE;

    return handletype_;
}

void AntiStrategy::SetHandleType(HANDLE_TYPE handletype)
{
    handletype_ = handletype;
}

void AntiStrategy::SetHandleRichLevel(uint32 rich_level)
{
    rich_level_ = rich_level;
}

bool AntiStrategy::AddSensitive(const std::string& sensitive)
{
    if (sensitives_.end() != sensitives_.find(sensitive))
    {
        return false;
    }
    sensitives_.insert(sensitive);
    return true;
}

bool AntiStrategy::RemoveSensitive(const std::string& sensitive)
{
    auto it = sensitives_.find(sensitive);
    if (it == sensitives_.end())
        return false;

    sensitives_.erase(it);
    return true;
}

bool AntiStrategy::AddNickname(const std::string& vestname)
{
    if (vestnames_.end() != vestnames_.find(vestname))
    {
        return false;
    }
    vestnames_.insert(vestname);
    return true;
}

bool AntiStrategy::RemoveNickname(const std::string& vestname)
{
    auto it = vestnames_.find(vestname);
    if (it == vestnames_.end())
        return false;

    vestnames_.erase(it);
    return true;
}

bool AntiStrategy::AddReceiveId(const std::string& receiveid)
{
    uint32 id = 0;
    base::StringToUint(receiveid, &id);
    if (receiveids_.end() != receiveids_.find(id))
    {
        return false;
    }
    receiveids_.insert(id);
    return true;

}

bool AntiStrategy::RemoveReceiveId(const std::string& receiveid)
{
    uint32 id = 0;
    base::StringToUint(receiveid, &id);
    auto it = receiveids_.find(id);
    if (it == receiveids_.end())
        return false;

    receiveids_.erase(it);
    return true;
}