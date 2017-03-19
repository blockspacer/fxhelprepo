#pragma once

#undef max // 因为微软这个二比在某些头文件定义了max宏
#undef min // 因为微软这个二比在某些头文件定义了min宏

#include <vector>
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/bind.h"

class AuthorityClientBussiness
{
public:
    AuthorityClientBussiness();
    ~AuthorityClientBussiness();

    void SetSendDataFunction(
        base::Callback<bool(const std::vector<uint8>& data)> callback);

    bool HandleMessage(const std::vector<uint8>& data);

private:
    base::Callback<bool(const std::vector<uint8>& data)> send_data_callback_;
};

