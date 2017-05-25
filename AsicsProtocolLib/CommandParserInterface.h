#pragma once

#include <string>

class CommandParserInterface
{
public:
    virtual bool HandleBussiness(const std::string& request, std::string* response) = 0;
};