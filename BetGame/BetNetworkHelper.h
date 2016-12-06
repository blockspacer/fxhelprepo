#pragma once

#include <string>
#include <vector>

#undef max
#undef min
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/callback.h"
#include "Network/TcpManager.h"
#include "Network/User.h"

class BetGameDatabase;
class BetNetworkHelper
{
public:
    BetNetworkHelper();
    ~BetNetworkHelper();

    bool Initialize();
    void Finalize();

    bool Login(const std::string& account, const std::string& password);
    bool EnterRoom(uint32 room_id);

    void SetTipMessage(const base::Callback<void(const std::wstring&)>& callback);
    void SetBetResultNotify(const base::Callback<void(const  BetResult&)>& callback);
    void SetBetTimeNotify(const base::Callback<void(uint32 time)>& callback);

private:
    void OnBetNotify(const  BetResult& bet_result);

    std::unique_ptr<TcpManager> tcp_manager_;
    std::unique_ptr<User> user_;
    base::Callback<void(const std::wstring&)> tips_callback_;
    base::Callback<void(const  BetResult&)> result_callback_;
    base::Callback<void(uint32)> time_callback_;
    scoped_ptr<base::Thread> worker_thread_;
    std::map<uint32, uint32> result_map_;
    std::unique_ptr<BetGameDatabase> database_;
};

