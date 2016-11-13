#pragma once

#include <string>
#include <vector>

#undef max
#undef min
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/callback.h"
#include "Network/TcpManager.h"
#include "Network/User.h"

class BetNetworkHelper
{
public:
    BetNetworkHelper();
    ~BetNetworkHelper();

    bool Initialize();
    void Finalize();

    bool Login(const std::string& account, const std::string& password);
    bool EnterRoom(uint32 room_id);

    void SetBetResultNotify(const base::Callback<void(uint32)>& callback);

private:
    void OnBetNotify(const  BetResult& bet_result);

    std::unique_ptr<TcpManager> tcp_manager_;
    std::unique_ptr<User> user_;
    base::Callback<void(uint32)> result_callback_;
    scoped_ptr<base::Thread> worker_thread_;
};

