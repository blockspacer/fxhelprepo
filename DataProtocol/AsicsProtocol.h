#pragma once

#include <vector>
#include "third_party/chromium/base/basictypes.h"


struct TLV
{
    uint8 data_type;
    uint32 length;
    std::vector<uint8> value;
};

#pragma pack(1)
struct Header 
{
    uint16 version;
    uint16 header_length;
    uint32 total_length;
    uint32 serial;
    uint32 ack;
    uint32 timestamp;
};
#pragma pack ()

class AsicsProtocol
{
public:
    AsicsProtocol();
    ~AsicsProtocol();

    // 1、正常组包回复
    // 2、非正常包回复(格式错误或无法解密)
    void SetReceiveCallback();

    // 由网络层收到tcp数据扔过来，处理好粘包的问题，组合好数据包后回调
    bool Receive(const std::vector<uint8>& data);

private:

    bool ReadHeader();
    bool ReadBody();
    bool Decrypt(uint8* buffer, uint32 length);

    void Finish();
    void Error();
    

    uint32 offsect_;
    std::vector<uint8> buffer_;

    struct Header header_;
    std::string json_data_;
};

