#pragma once
#include <windows.h>


typedef DWORD (*run_fn)(LPVOID lpParam);
class Thread
{
public:
    Thread();
    ~Thread();
    void Init(run_fn run, LPVOID lpParam);
    bool Start();
    void Stop();
private:
    HANDLE handle_;
    DWORD threadid_;
    run_fn run_;
    LPVOID param_;
};

