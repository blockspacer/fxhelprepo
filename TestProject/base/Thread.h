#pragma once
#include <string>
#include <memory>
#include <thread>

class MessageLoop;
class TaskRunner;

class Thread
{
public:
    explicit Thread(const std::string& name);
    ~Thread();

    bool Start();
    void Stop();
    bool IsRunning() const;

    MessageLoop* message_loop() const{ return message_loop_; }
    std::reference_wrapper<TaskRunner> task_runner();

private:
    static void ThreadFunc(void* params);

    void ThreadMain();
    void Run(MessageLoop* message_loop);

private:
    typedef void* ThreadHandle;
    std::unique_ptr<std::thread> thread_;
    bool running_;
    MessageLoop* message_loop_;
    ThreadHandle handle_;
    std::string name_;
};

