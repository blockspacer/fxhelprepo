#pragma once

#include "AuthorityClientBussiness.h"
#include "third_party/chromium/base/callback.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/threading/thread.h"
#include <vector>
#include <map>
#include "event2/bufferevent.h"

class AuthorityController
{
public:
    AuthorityController();
    ~AuthorityController();

    bool Initialize();
    void Finalize();

    void SetSendDataFunction(
        base::Callback<bool(bufferevent*, const std::vector<uint8>& data)> callback);

    bool Start();
    void Stop();

    bool AddClient(bufferevent *bev, const struct sockaddr& sock);
    bool RemoveClient(bufferevent *bev);

    bool HandleMessage(bufferevent *bev, const std::vector<uint8>& data);

    bool AddUser();
    bool RemoveUser();
    bool QueryUser();
    bool ModifyUser();

private:

    void DoAddClient(bufferevent *bev, const struct sockaddr& sock);
    void DoRemoveClient(bufferevent *bev);

    void DoHandleMessage(bufferevent *bev, const std::vector<uint8>& data);

    bool SendDataToClient(bufferevent* bev, const std::vector<uint8>& data);

    // 数据库对象

    // 网络连接对象

    // 权限监控对象

    // 控制用户对象
    std::map<bufferevent*, scoped_ptr<AuthorityClientBussiness>> client_map_;


    base::Callback<bool(bufferevent *, const std::vector<uint8>& data)> send_data_callback_;

    scoped_ptr<base::Thread> control_thread_;
    scoped_refptr<base::TaskRunner> runner_;
};

