#include "stdafx.h"
#include "Config.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/path_service.h"


Config::Config()
{
    base::FilePath path;
    PathService::Get(base::DIR_EXE, &path);
    filepath_ = path.value();
}

Config::~Config()
{
}

bool Config::Load()
{
    return false;
}
bool Config::GetUserName() const
{
    return false;
}
bool Config::GetPassword() const
{
    return false;
}

bool Config::Save()
{
    return false;
}
