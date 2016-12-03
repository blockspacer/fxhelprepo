// VMPTest.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <windows.h>
#include "VMProtect/VMProtectSDK.h"

//int _tmain(int argc, _TCHAR* argv[])
//{
//	return 0;
//}

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

int main(int argc, char **argv)
{
    char *serial = R"(hsxtk/xQRqrCMaupHWLSlgKKuln8es2L12VsQNKwduQ6NZg76TYHT7tPQhFBd8SDtJNHR6m3x9mfb1nsJG1btjpvHHbIWuDXnQzxex5VPwzqKh0bFAAfQOB1ONbI8T6fGTpuVC8N/CK8uAQ+O1O7fCjWEWresJQZrju7bi5u+PnU87oKTdGKwrgz1L0BEn+z4bSpbNc9mgOVTOoyTUaeDwblhS0zi3ZtE3sufzolIEXxEn4vMF9FsbI0hxfqobXN534ij5XW0IbZTS9Ciu878tS+mMoLGhDwZxhQ2MXoHDlt46AXEG3NZ2xfE4wk8p0OuOO7HmhcemvVPVcTYunn5A==)";

    int res = VMProtectSetSerialNumber(serial);
    print_state(res);

    if (res)
    {
        VMProtectSerialNumberData sd = { 0 };
        VMProtectGetSerialNumberData(&sd, sizeof(sd));
        printf("max. build date: y = %d, m = %d, d = %d\n", sd.dtMaxBuild.wYear, sd.dtMaxBuild.bMonth, sd.dtMaxBuild.bDay);
        printf("please register!\n");
        //return 0;
    }

    printf("I'm registered\n");

    int nSize = VMProtectGetCurrentHWID(NULL, 0);
    char *buf = new char[nSize];
    VMProtectGetCurrentHWID(buf, nSize);
    printf("HWID: %s\n", buf);
    delete[] buf;

    return 0;
}

