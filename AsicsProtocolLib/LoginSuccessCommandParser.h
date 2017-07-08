#pragma once
#include "CommandParser.h"
#include "MessageDefine.h"

class LoginSuccessCommandParser :
    public CommandParser<CMD_LOGIN_SUCCESS>
{
public:
    LoginSuccessCommandParser(uint32 session_id);
    ~LoginSuccessCommandParser();

    virtual bool HandleBussiness(const std::string& request, std::string* response);

    virtual bool Make(std::string* json_data);

    void SetUsername(const std::string& username);
    std::string GetUsername() const;

    void SetPassword(const std::string& password);
    std::string GetPassword() const;

    void SetCookie(const std::string& cookie);
    std::string GetCookie() const;

    void SetNickname(const std::string& nickname);
    std::string GetNickname() const;

private:
    std::string username_;
    std::string password_;
    std::string cookie_;
    std::string nickname_;
};

