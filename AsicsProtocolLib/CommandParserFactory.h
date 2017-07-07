#pragma once

#include "MessageDefine.h"
#include "CommandParserInterface.h"
#include "CommandParser.h"
#include "AsicsProtocolLib/EndPointInfoCommandParser.h"

// 只给服务端使用，
// 因为客户端在发送之前要构建的时候就知道是什么类型，收到回复的时候也通过serialid能找到原来的业务回调
class CommandParserFactory
{
public:
    static CommandParserInterface* CreateCommandParser(CommandId cmd_id, uint32 session_id)
    {
        CommandParserInterface* ptr = nullptr;
        switch (cmd_id)
        {
        case CMD_ENDPOINT_INFO_COMPUTER:
            ptr = new EndPointInfoCommandParser(session_id);
            break;

        default:
            ptr = new CommandParser<CMD_UNDEFINE>(session_id);
        }

        return ptr;
    }
};
