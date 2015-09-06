#include "thread.h"


Thread::Thread()
    :run_(0)
    , param_(0)
    , handle_(0)
    , threadid_(0)
{
}


Thread::~Thread()
{
}

void Thread::Init(run_fn run, LPVOID lpParam)
{
    run_ = run;
    param_ = lpParam;
    handle_ = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)run_, param_, CREATE_SUSPENDED, &threadid_);
}
bool Thread::Start()
{
    ResumeThread(handle_);
    return (handle_ != INVALID_HANDLE_VALUE);
}

void Thread::Stop()
{
    if (handle_)
    {
        TerminateThread(handle_, 0);
    }
    handle_ = 0;
}
