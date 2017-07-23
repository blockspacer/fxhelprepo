#pragma once
#include <string>
class Config
{
public:
    Config();
    ~Config();
    bool GetUserName(std::wstring* username) const;
    bool GetPassword(std::wstring* password) const;
    bool Save(const std::wstring& username, const std::wstring& password,
        bool remember) const;
    bool GetRemember() const;
private:
    std::wstring filepath_;
};

