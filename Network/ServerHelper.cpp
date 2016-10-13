#include "ServerHelper.h"

#include "assert.h"

#include <winsock2.h>
#include <WS2tcpip.h>

ServerHelper::ServerHelper()
{
}


ServerHelper::~ServerHelper()
{
}

bool ServerHelper::GetChatServerIp(std::vector<std::string>* hostips)
{
    char   *ptr, **pptr;
    struct hostent *hptr;
    char   str[32];
    ptr = "chat1.fanxing.kugou.com";

    if ((hptr = gethostbyname(ptr)) == NULL)
    {
        return false;
    }

    switch (hptr->h_addrtype)
    {
    case AF_INET:
    case AF_INET6:
        pptr = hptr->h_addr_list;
        for (; *pptr != NULL; pptr++)
        {
            hostips->push_back(std::string(inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str))));
        }
        break;
    default:
        assert(false && L"can not resolve server");
        return false;
    }

    return true;
}