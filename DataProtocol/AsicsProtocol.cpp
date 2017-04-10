#include "AsicsProtocol.h"
#include "third_party/chromium/base/logging.h"

namespace
{
    static uint32 header_length = sizeof(Header);
}

AsicsProtocol::AsicsProtocol()
{
}


AsicsProtocol::~AsicsProtocol()
{
}

void AsicsProtocol::SetReceiveCallback()
{

}

// 由网络层收到tcp数据扔过来，处理好粘包的问题，组合好数据包后回调
bool AsicsProtocol::Receive(const std::vector<uint8>& data)
{
    if (data.empty())
        return false;

    buffer_.insert(buffer_.end(), data.begin(), data.end());


    if (header_.header_length == 0) // 未读协议头
    {
        if (buffer_.size() < header_length) // 未够协议头长度
            return true;

        if (!ReadHeader())
            return false;
    }

    // 已读协议头
    // 未接收完当前包数据
    if (buffer_.size() < header_.total_length)
        return true;
    
    if (buffer_.size() == header_.total_length)
        return ReadBody();

    // 如果数据超出预计长度，是因为粘包，是正常的；一次读完，通知完成
    return ReadBody();
}

bool AsicsProtocol::ReadHeader()
{

    DCHECK(header_.total_length > header_.header_length);
    return false;
}
bool AsicsProtocol::ReadBody()
{
    return false;
}

void AsicsProtocol::Finish()
{
    return;
}

void AsicsProtocol::Error()
{

}

bool AsicsProtocol::Decrypt(uint8* buffer, uint32 length)
{
    return false;
}
