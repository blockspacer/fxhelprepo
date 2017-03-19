#pragma once
class AuthorityController
{
public:
    AuthorityController();
    ~AuthorityController();

    bool Initialize();
    void Finalize();

    bool Start();
    void Stop();

    bool AddUser();
    bool RemoveUser();
    bool QueryUser();
    bool ModifyUser();

private:
    // 数据库对象

    // 网络连接对象

    // 权限监控对象

    // 控制用户对象
};

