#pragma once
#include <vector>
#include <string>
#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏
#include "third_party/chromium/base/files/file_path.h"

class RegisterHelper
{
public:
    RegisterHelper();
    ~RegisterHelper();

    bool SaveVerifyCodeImage(
        const std::vector<uint8>& image);

    bool GetVerifyCodeImagePath(std::wstring* path) const;

private:
    base::FilePath pathname_;
};

