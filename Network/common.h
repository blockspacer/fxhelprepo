#pragma once
enum class MessageLevel
{
    MESSAGE_LEVEL_DEBUG = 1,
    MESSAGE_LEVEL_ONCE = 1<<1,
    MESSAGE_LEVEL_DISPLAY = 1<<2,
};