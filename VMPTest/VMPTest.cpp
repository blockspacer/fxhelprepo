// VMPTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include "VMProtect/VMProtectSDK.h"

#define PRINT_HELPER(state, flag) if (state & flag) printf("%s ", #flag)
void print_state(INT state)
{
    if (state == 0)
    {
        printf("state = 0\n");
        return;
    }

    printf("state = ");
    PRINT_HELPER(state, SERIAL_STATE_FLAG_CORRUPTED);
    PRINT_HELPER(state, SERIAL_STATE_FLAG_INVALID);
    PRINT_HELPER(state, SERIAL_STATE_FLAG_BLACKLISTED);
    PRINT_HELPER(state, SERIAL_STATE_FLAG_DATE_EXPIRED);
    PRINT_HELPER(state, SERIAL_STATE_FLAG_RUNNING_TIME_OVER);
    PRINT_HELPER(state, SERIAL_STATE_FLAG_BAD_HWID);
    PRINT_HELPER(state, SERIAL_STATE_FLAG_MAX_BUILD_EXPIRED);
    printf("\n");
}

char *read_serial(const char *fname)
{
    FILE *f;
    if (0 != fopen_s(&f, fname, "rb")) return NULL;
    fseek(f, 0, SEEK_END);
    int s = ftell(f);
    fseek(f, 0, SEEK_SET);
    char *buf = new char[s + 1];
    fread(buf, s, 1, f);
    buf[s] = 0;
    fclose(f);
    return buf;
}

// The foo() method is very short, but we need it to be an individual function
// so we asked the compiler to not compile it inline
__declspec(noinline) void foo()
{
    printf("I'm foo!\n");
}

int main(int argc, char **argv)
{
    char *serial = read_serial("d:/serial.ini");
    int res = VMProtectSetSerialNumber(serial);
    delete[] serial;
    if (res)
    {
        printf("serial number is bad\n");
        print_state(res);
        return 0;
    }
    printf("serial number is correct, calling foo()\n");
    foo();
    printf("done\n");
    return 0;
}

