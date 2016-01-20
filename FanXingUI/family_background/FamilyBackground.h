#pragma once
#include <string>
#include <memory>

// 本类负责提供所有后台操作的接口
// 包括：登录

class FamilyDaily;
class FamilyBackground
{
public:
    FamilyBackground();
    ~FamilyBackground();

    bool Init();
    bool Login(const std::string& username, const std::string& password);

private:
    std::unique_ptr<FamilyDaily> familyDaily_;
};

