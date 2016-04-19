#pragma once
#include <vector>
#include <memory>

class User;
// 批量控制用户类

class UserController
{
public:
    UserController();
    ~UserController();


    // 执行对应批量用户控制策略
    void Run();

private:
    std::vector<std::shared_ptr<User> > users_;
};

