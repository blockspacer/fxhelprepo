#pragma once
#include "third_party/chromium/base/basictypes.h"
#include <functional>
#include <vector>

typedef uint32 WebsocketHandle;

typedef std::function<void(bool, WebsocketHandle)> AddClientCallback;
typedef std::function<void(bool, const std::vector<uint8>&)> ClientCallback;
typedef std::function<void(bool)> SendDataCallback;
