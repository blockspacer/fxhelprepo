#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "third_party/chromium/base/basictypes.h"

enum class MessageLevel
{
    MESSAGE_LEVEL_DEBUG = 1,
    MESSAGE_LEVEL_ONCE = 1 << 1,
    MESSAGE_LEVEL_DISPLAY = 1 << 2,
};

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;
