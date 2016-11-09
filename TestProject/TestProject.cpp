// TestProject.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <assert.h>
#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"
#include "Network/Network.h"
#include "third_party/chromium/base/at_exit.h"
#include "third_party/chromium/base/run_loop.h"
#include "UserTracker.h"

int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;

    NetworkInitialize();

    UserTracker tracker;
    tracker.Test();

    NetworkFainalize();
	return 0;
}

