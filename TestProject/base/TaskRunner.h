#pragma once
class TaskRunner
{
public:
    TaskRunner();
    ~TaskRunner();

    bool RunOnCurrentThread() const;
    bool PostTask(const std::function<void()>& task);

};

