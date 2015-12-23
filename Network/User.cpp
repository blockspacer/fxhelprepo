#include "User.h"


User::User()
{
}


User::~User()
{
}

// 设置参数
void User::SetUserName(const std::string& username)
{
    username_ = username;
}
std::string User::GetUserName() const
{
    return username_;
}

void User::SetPassword(const std::string& password)
{
    password_ = password;
}

std::string User::GetPassword() const
{
    return password_;
}

void User::SetCookies(const std::vector<std::string> cookies)
{
    cookies_ = cookies;
}

std::vector<std::string> User::GetCookies() const
{
    return cookies_;
}

//设置房间命令消息回调函数,命令的解析和行为处理要在另外的模块处理
void SetRoomNotify()
{

}

// 操作行为
bool User::Login()
{
    return false;
}

bool User::Logout()
{
    return false;
}

bool User::EnterRoom(uint32 roomid)
{
    return false;
}

bool User::ExitRoom()
{
    return false;
}

bool User::Chat(const std::string& message)
{
    return false;
}

bool User::SendStar(uint32 count)
{
    return false;
}

bool User::RetrieveStart()
{
    return false;
}

bool User::SendGift(uint32 giftid)
{
    return false;
}

bool User::KickoutUser(uint32 userid)
{
    return false;
}

bool User::SilencedUser(uint32 userid)
{
    return false;
}
