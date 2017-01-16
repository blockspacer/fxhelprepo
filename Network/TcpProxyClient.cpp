#include <assert.h>
#include "IpProxy.h"
#include "TcpProxyClient.h"
//#include "Network/TcpClient.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

SocketWrapper::SocketWrapper()
    :socket_(0)
{

}
SocketWrapper::~SocketWrapper()
{

}

bool SocketWrapper::Create()
{
    int rc = 0;
    socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (socket_ == INVALID_SOCKET)
    {
        assert(false);
        return false;
    }
    return true;
}

SocketHandle SocketWrapper::GetSocketHandle() const
{
    return socket_;
}

bool SocketWrapper::Connect(const std::string& ip, unsigned short port)
{
    int rc = 0;
    SOCKADDR_IN serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = inet_addr(ip.c_str());

    struct linger lig;
    lig.l_onoff = 0;
    lig.l_linger = 0;
    int iLen = sizeof(struct linger);
    setsockopt(socket_, SOL_SOCKET, SO_LINGER, (char *)&lig, iLen);
    int flag = 1;
    int len = sizeof(int);
    setsockopt(socket_, SOL_SOCKET, SO_REUSEADDR, (const char*)&flag, len);
    int keepalivetime = 10;
    setsockopt(socket_, IPPROTO_TCP, SO_KEEPALIVE,
        (const char FAR *)&keepalivetime, sizeof(keepalivetime));
    rc = connect(socket_, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
    if (SOCKET_ERROR == rc)
    {
        int error = WSAGetLastError();
        return false;
    }

    return true;
}

bool SocketWrapper::Send(const std::vector<char>& data)
{
    int len = send(socket_, data.data(), data.size(), 0);
    return len == data.size();
}

bool SocketWrapper::Recv(std::vector<char>* buffer)
{
    std::vector<char> temp(4096);
    int len = 0;
    len = recv(socket_, &temp[0], temp.size(), 0);
    if (SOCKET_ERROR == len)
    {
        return false;
    }
    else if (len > 0)
    {
        buffer->assign(temp.begin(), temp.begin() + len);
        return true;
    }
    return true;
}

void SocketWrapper::Close()
{
    if (socket_)
    {
        closesocket(socket_);
        socket_ = 0;
    }
}

TcpProxyClient::TcpProxyClient()
{
}

TcpProxyClient::~TcpProxyClient()
{
    Close();
}

void TcpProxyClient::SetProxy(const IpProxy& ipproxy)
{
    ipproxy_ = ipproxy;
}

bool TcpProxyClient::Connect(const std::string& ip, uint16 port)
{
    if (ipproxy_.GetProxyType() == IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS4)
    {
        return ConnectToSocks4Proxy(ip, port);
    }

    if (ipproxy_.GetProxyType() == IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS5)
    {
        return ConnectToSocks5Proxy(ip, port);
    }

    return ConnectToServer(ip, port);
}

SocketHandle TcpProxyClient::GetSocketHandle() const
{
    return socketWrapper_.GetSocketHandle();
}

bool TcpProxyClient::ConnectToSocks4Proxy(const std::string& ip, uint16 port)
{
    if (!socketWrapper_.Create())
    {
        return false;
    }
    
    if (!socketWrapper_.Connect(ipproxy_.GetProxyIp(), ipproxy_.GetProxyPort()))
    {
        return false;
    }

    uint32 u32Ip = inet_addr(ip.c_str());
    char ip0 = u32Ip >> 24;
    char ip1 = u32Ip << 8 >> 24;
    char ip2 = u32Ip << 16 >> 24;
    char ip3 = u32Ip & 0xFF;
    char port0 = port >> 8;
    char port1 = port & 0xFF;
    // socks4协议
        //+---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- + .... + ---- +
        //| VN | CD | DSTPORT | DSTIP | USERID | NULL |
        //+---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- + .... + ---- +
//#of bytes :1    1      2        4      variable  1
    std::vector<char> socks4_version_send_pack;
    //VN is the SOCKS protocol version number and should be 4. CD is the
    //    SOCKS command code and should be 1 for CONNECT request.NULL is a byte
    //    of all zero bits.
    socks4_version_send_pack.push_back(0x04);
    socks4_version_send_pack.push_back(0x01);
    socks4_version_send_pack.push_back(port0);
    socks4_version_send_pack.push_back(port1);
    socks4_version_send_pack.push_back(ip3);
    socks4_version_send_pack.push_back(ip2);
    socks4_version_send_pack.push_back(ip1);
    socks4_version_send_pack.push_back(ip0);
    socks4_version_send_pack.push_back(0x00);

    if (!socketWrapper_.Send(socks4_version_send_pack))
        return false;

    std::vector<char> socks4_version_recv_pack;
    if (!socketWrapper_.Recv(&socks4_version_recv_pack))
        return false;

    //The SOCKS server checks to see whether such a request should be granted
    //based on any combination of source IP address, destination IP address,
    //destination port number, the userid, and information it may obtain by
    //consulting IDENT, cf. RFC 1413.  If the request is granted, the SOCKS
    //server makes a connection to the specified port of the destination host.
    //A reply packet is sent to the client when this connection is established,
    //or when the request is rejected or the operation fails. 
 //           +---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
 //           | VN | CD | DSTPORT | DSTIP |
 //           +---- + ---- + ---- + ---- + ---- + ---- + ---- + ---- +
 //# of bytes : 1    1      2         4
    if (socks4_version_recv_pack.size() != 8)
        return false;

    //VN is the version of the reply code and should be 0. CD is the result
    //code with one of the following values :
    if (socks4_version_recv_pack[0] != 0x00)
        return false;

    //90 : request granted
    //91 : request rejected or failed
    //92 : request rejected becasue SOCKS server cannot connect to
    //identd on the client
    //93 : request rejected because the client program and identd
    //report different user - ids
    if (socks4_version_recv_pack[1] != 0x5a) // 0x5a == 90
        return false;
    //==========================================================================

    //std::string str = "<policy-file-request/>";
    //std::vector<char> data;
    //data.assign(str.begin(), str.end());
    //data.push_back(0);

    //if (!socketWrapper_.Send(data))
    //    return false;

    //std::vector<char> flashresponse;
    //if (!socketWrapper_.Recv(&flashresponse))
    //    return false;

    return true;
}

bool TcpProxyClient::ConnectToSocks5Proxy(const std::string& ip, uint16 port)
{
    if (!socketWrapper_.Create())
    {
        return false;
    }

    if (!socketWrapper_.Connect(ipproxy_.GetProxyIp(), ipproxy_.GetProxyPort()))
    {
        return false;
    }

    // socks5协议
    std::vector<char> socks5_version_send_pack;
    socks5_version_send_pack.push_back(0x05);
    socks5_version_send_pack.push_back(0x03);// 请求三种认证方式
    socks5_version_send_pack.push_back(0x00);// 0x00 不需要认证
    socks5_version_send_pack.push_back(0x01);// 0x01 GSSAPI
    socks5_version_send_pack.push_back(0x02);// 0x02 用户名、密码认证
    if (!socketWrapper_.Send(socks5_version_send_pack))
        return false;
    
    std::vector<char> socks5_version_recv_pack;
    if (!socketWrapper_.Recv(&socks5_version_recv_pack))
        return false;

    if (socks5_version_recv_pack.size() != 2)
        return false;

    if (socks5_version_recv_pack[0] != 0x05)
        return false;

    uint8 method = socks5_version_recv_pack[1];
    if (method!=0)
    {
        assert(false && L"验证失败");
        return false;
    }

    uint32 u32Ip = inet_addr(ip.c_str());
    
    char ip0 = u32Ip >> 24;
    char ip1 = u32Ip << 8 >> 24;
    char ip2 = u32Ip << 16 >> 24;
    char ip3 = u32Ip & 0xFF;
    char port0 = port >> 8;
    char port1 = port & 0xFF;
    u32Ip = htonl(u32Ip);
    port = htons(port);
    std::vector<char> socks5_request;
    socks5_request.push_back(0x05);
    socks5_request.push_back(0x01);// Connect请求
    socks5_request.push_back(0x00);// RSV
    socks5_request.push_back(0x01);// ipv4 type
    socks5_request.push_back(ip0); // ip
    socks5_request.push_back(ip1); // ip
    socks5_request.push_back(ip2); // ip
    socks5_request.push_back(ip3); // ip
    socks5_request.push_back(port0);
    socks5_request.push_back(port1);
    if (!socketWrapper_.Send(socks5_request))
        return false;

    std::vector<char> socks5_response;
    if (!socketWrapper_.Recv(&socks5_response))
        return false;

    if (socks5_response.size() != 10)
        return false;

    if (socks5_response[0] != 0x05)
        return false;

    //0x00表示成功
    //0x01普通SOCKS服务器连接失败
    //0x02现有规则不允许连接
    //0x03网络不可达
    //0x04主机不可达
    //0x05连接被拒
    //0x06 TTL超时
    //0x07不支持的命令
    //0x08不支持的地址类型
    //0x09 - 0xFF未定义
    if (socks5_response[1] != 0x00)
        return false;

    if (socks5_response[2] != 0x00)
        return false;

    return true;
}

bool TcpProxyClient::ConnectToServer(const std::string& ip, uint16 port)
{
    if (!socketWrapper_.Create())
    {
        return false;
    }

    if (!socketWrapper_.Connect(ip, port))
    {
        return false;
    }
    return true;
}

bool TcpProxyClient::Send(const std::vector<uint8>& data)
{
    std::vector<char> chardata(data.begin(), data.end());
    if (!socketWrapper_.Send(chardata))
        return false;

    return true;
}

bool TcpProxyClient::Recv(std::vector<uint8>* buffer)
{
    if (!socketWrapper_.Recv(reinterpret_cast<std::vector<char>*>(buffer)))
        return false;
    
    return true;
}

void TcpProxyClient::Close()
{
    return socketWrapper_.Close();
}