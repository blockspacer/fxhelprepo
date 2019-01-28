#include "Thread.h"
#include "MessageLoop.h"

Thread::Thread(const std::string& name)
    :thread_(nullptr)
    , name_(name)
    , running_(false)
{
}


Thread::~Thread()
{
}

bool Thread::Start()
{
    thread_.reset(new std::thread(Thread::ThreadFunc, (void*)(this)));
}

void Thread::Stop()
{

}

bool Thread::IsRunning() const
{
    return running_;
}

void Thread::ThreadFunc(void* params)
{
    Thread* thread = (Thread*)(params);
    thread->ThreadMain();
}

void Thread::ThreadMain()
{
    std::unique_ptr<MessageLoop> message_loop(new MessageLoop());
    message_loop_ = message_loop.get();
    running_ = true;
    Run(message_loop_);
    running_ = false;
}

void Thread::Run(MessageLoop* message_loop)
{

}

std::reference_wrapper<TaskRunner> Thread::task_runner()
{
    return message_loop_->message_loop_proxy();
}
