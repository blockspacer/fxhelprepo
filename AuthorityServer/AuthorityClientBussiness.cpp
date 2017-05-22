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
    DCHECK(CalledOnValidThread());

    // 这是测试echo的回复
    //std::vector<uint8> new_data = data;
    //new_data.insert(new_data.end(), data.begin(), data.end());
    //send_data_callback_.Run(new_data);

    bool result = buffer_parser_.HandleBuffer(data);
    DCHECK(result);

    std::vector<std::vector<uint8>> repsonses;
    if (buffer_parser_.GetResponses(&repsonses))
    {
        for (const auto response : repsonses)
        {
            send_data_callback_.Run(response);
        }
    }

    return result;
}