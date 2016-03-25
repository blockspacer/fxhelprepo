#include "stdafx.h"
#include "RegisterHelper.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file.h"

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/time/time.h"
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

RegisterHelper::RegisterHelper()
{
}


RegisterHelper::~RegisterHelper()
{
}

bool RegisterHelper::SaveVerifyCodeImage(const std::vector<uint8>& image)
{
    if (image.empty())
    {
        return false;
    }

    uint64 time = base::Time::Now().ToInternalValue();
    std::string timestr = base::Uint64ToString(time);
    base::FilePath dirPath;
    bool result = PathService::Get(base::DIR_EXE, &dirPath);
    std::wstring filename = base::UTF8ToWide(timestr + ".png");
    pathname_ = dirPath.Append(filename);
    base::File pngfile(pathname_,
        base::File::FLAG_CREATE_ALWAYS | base::File::FLAG_WRITE);
    pngfile.Write(0, (char*)image.data(), image.size());
    pngfile.Close();
    return true;
}

bool RegisterHelper::GetVerifyCodeImagePath(std::wstring* path) const
{
    if (!path)
        return false;

    *path = pathname_.value();
    return true;
}
