#include "stdafx.h"
#include "AuthorityClientBussiness.h"


AuthorityClientBussiness::AuthorityClientBussiness()
{
}


AuthorityClientBussiness::~AuthorityClientBussiness()
{
}

void AuthorityClientBussiness::SetSendDataFunction(
    base::Callback<bool(const std::vector<uint8>& data)> callback)
{
    send_data_callback_ = callback;
}

bool AuthorityClientBussiness::HandleMessage(const std::vector<uint8>& data)
{
    std::vector<uint8> new_data = data;
    new_data.insert(new_data.end(), data.begin(), data.end());
    send_data_callback_.Run(new_data);
    return true;
}