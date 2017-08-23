#pragma once
#include <string>
class Config
{
public:
    Config();
    ~Config();
    bool GetUserName(std::wstring* username) const;
    bool GetPassword(std::wstring* password) const;
    bool GetCookies(std::wstring* cookies) const;
    bool GetRoomid(std::wstring* roomid) const;

    bool SaveUserInfo(const std::wstring& username, 
        const std::wstring& password,
        const std::wstring& cookies,
        bool remember) const;
    bool SaveRoomId(const std::wstring& roomid) const;
    bool GetRemember() const;

    bool SaveRobot(bool enable, const std::wstring& apikey) const;
    bool GetRobot(bool* enable, std::wstring* apikey) const;

    bool SaveGiftThanks(bool enable, uint32 gift_value) const;
    bool GetGiftThanks(bool* enable, uint32* gift_value) const;

    bool SaveEnterRoomWelcome(bool enable, uint32 rich_level) const;
    bool GetEnterRoomWelcome(bool* enable, uint32* rich_level) const;

    bool SaveRepeatChat(bool enable, const std::wstring& content, 
                        const std::wstring& seconds) const;
    bool GetRepeatChat(bool* enable, std::wstring* content, 
                       std::wstring* seconds) const;

private:
    std::wstring filepath_;
};

