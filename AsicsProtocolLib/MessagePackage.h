#pragma once
#include <string>
#include <vector>
#include "MessageDefine.h"

class Package
{
public:
    Package(uint32 sequence_number);
    ~Package();
    void SetHeader(const Header& header);
    void SetData(const std::string& data);

    // 服务器处理数据时使用
    bool HandlePackage(std::vector<uint8>* response_package);

    // 客户端构造请求时使用
    bool GetPackage(std::vector<uint8>* request_package);

private:
    bool IsValidCmd(uint32 cmd) const;

    uint32 sequence_number_;
    Header request_header_;
    std::string request_data_;
};

