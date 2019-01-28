#include "MessageLoop.h"


MessageLoop::MessageLoop()
    :message_loop_proxy_(new TaskRunner())
{
}


MessageLoop::~MessageLoop()
{
}
