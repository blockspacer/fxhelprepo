#pragma once
#include <vector>
#include <memory>
#include "third_party/chromium/base/basictypes.h"

class User;
// 批量控制用户类

class UserController
{
public:
    UserController();
    ~UserController();

    bool AddUser(const std::string& username, const std::string& password);
    bool FillRoom(uint32 roomid, uint32 count);

    // 执行对应批量用户控制策略,暂时不处理进入房间的通知回调
    void Run();

private:
    std::vector<std::shared_ptr<User> > users_;
};

