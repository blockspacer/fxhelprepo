#pragma once
#include "stdafx.h"

#include <string>
#include <vector>
#include <list>
#include "MessageDefine.h"
#include "CommandParserInterface.h"

#include "third_party/json/writer.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/time/time.h"

template<CommandId cmdid>
class CommandParser :public CommandParserInterface
{
public:
    CommandParser(uint32 session_id) :session_id_(session_id){};
    virtual ~CommandParser(){};
    virtual bool HandleBussiness(const std::string& request, std::string* response)
    {
        *response = "{\"result\":0}";
        return false;
    };
private:
    uint32 session_id_;
};


