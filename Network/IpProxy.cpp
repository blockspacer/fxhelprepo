#include "IpProxy.h"


IpProxy::IpProxy()
{
}

IpProxy::IpProxy(const IpProxy& ipproxy)
{
    proxytype_ = ipproxy.proxytype_;
    proxyip_ = ipproxy.proxyip_;
    proxyport_ = ipproxy.proxyport_;
}

IpProxy::~IpProxy()
{
}

void IpProxy::SetProxyType(PROXY_TYPE proxytype)
{
    proxytype_ = proxytype;
}

IpProxy::PROXY_TYPE IpProxy::GetProxyType() const
{
    return proxytype_;
}

void IpProxy::SetProxyIp(const std::string& proxyip)
{
    proxyip_ = proxyip;
}

std::string IpProxy::GetProxyIp() const
{
    return proxyip_;
}

void IpProxy::SetProxyPort(uint16 proxyport)
{
    proxyport_ = proxyport;
}

uint16 IpProxy::GetProxyPort() const
{
    return proxyport_;
}
