#pragma once
#include "TaskRunner.h"

#include <functional>
#include <memory>

class MessageLoop
{
public:
    MessageLoop();
    ~MessageLoop();

    std::reference_wrapper<TaskRunner> message_loop_proxy()
    {
        return message_loop_proxy_;
    }

private:
    std::reference_wrapper<TaskRunner> message_loop_proxy_;

};

class MessageLoopProxy : public TaskRunner
{
public:
    MessageLoopProxy();
    ~MessageLoopProxy();

    bool RunOnCurrentThread() const;
    bool PostTask(const std::function<void()>& task);

};

