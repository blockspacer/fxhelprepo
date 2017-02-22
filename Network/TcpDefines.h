#include <functional>
#include <vector>
#include "third_party/chromium/base/basictypes.h"

typedef intptr_t SocketHandle;

typedef std::function<void(bool, std::vector<uint8>&)> DataReceiveCallback;
typedef std::function<void(bool, SocketHandle)> ConnectCallback;
typedef std::function<void(bool)> SendDataCallback;