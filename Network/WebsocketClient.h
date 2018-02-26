#pragma once

#include "WebsocketDefine.h"

class WebsocketClient
{
public:
    WebsocketClient();
    ~WebsocketClient();

    WebsocketHandle Connect();

    bool Send();

    void Close();

};

