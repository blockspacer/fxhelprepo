#pragma once
#include <string>
class Config
{
public:
    Config();
    ~Config();

    bool Load();
    bool GetUserName() const;
    bool GetPassword() const;
    bool Save();

private:
    std::wstring filepath_;
};

