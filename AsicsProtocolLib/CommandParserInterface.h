#pragma once

#include <string>

class CommandParserInterface
{
public:
    virtual bool HandleBussiness(const std::string& request, std::string* response) = 0;
    virtual bool Make(std::string* json_data) = 0;
};