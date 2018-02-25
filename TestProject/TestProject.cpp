// TestProject.cpp : �������̨Ӧ�ó������ڵ㡣
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
#include "Network/TcpClientController.h"
#include "third_party/chromium/base/at_exit.h"
#include "third_party/chromium/base/run_loop.h"
#include "third_party/zlib/zlib.h"
#include "third_party/chromium/base/path_service.h"
#include "UserTracker.h"
#include "third_party/chromium/base/files/file_util.h"
#include "third_party/chromium/base/command_line.h"
#include "third_party/chromium/base/at_exit.h"

extern void SingleUserSingleRoomTest(TcpClientController* tcp_manager);
void InitAppLog();
int ZLibTest()
{
    char text[] = "zlib compress and uncompress test\nturingo@163.com\n2012-11-05\n";
    uLong tlen = strlen(text) + 1;	/* ��Ҫ���ַ����Ľ�����'\0'Ҳһ������ */
    char* buf = NULL;
    uLong blen;

    /* ���㻺������С����Ϊ������ڴ� */
    blen = compressBound(tlen);	/* ѹ����ĳ����ǲ��ᳬ��blen�� */
    if ((buf = (char*)malloc(sizeof(char) * blen)) == NULL)
    {
        printf("no enough memory!\n");
        return -1;
    }

    /* ѹ�� */
    if (compress((unsigned char*)buf, &blen, (unsigned char*)text, tlen) != Z_OK)
    {
        printf("compress failed!\n");
        return -1;
    }

    /* ��ѹ�� */
    if (uncompress((unsigned char*)text, &tlen, (unsigned char*)buf, blen) != Z_OK)
    {
        printf("uncompress failed!\n");
        return -1;
    }

    /* ��ӡ��������ͷ��ڴ� */
    printf("%s", text);
    if (buf != NULL)
    {
        free(buf);
        buf = NULL;
    }
    return 0;
}

int websocket_test();

int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;
    InitAppLog();
    NetworkInitialize();

    //std::unique_ptr<TcpClientController> tcp_manager(new TcpClientController);
    //tcp_manager->Initialize();
    //SingleUserSingleRoomTest(tcp_manager.get());
    //tcp_manager->Finalize();
    //ZLibTest();
    //UserTracker tracker;
    //tracker.Test();

    websocket_test();

    NetworkFainalize();
	return 0;
}

void InitAppLog()
{
    CommandLine::Init(0, NULL);
    base::FilePath path;
    PathService::Get(base::DIR_APP_DATA, &path);
    path = path.Append(L"FanXingHelper").Append(L"fanxinghelper.log");
    logging::LoggingSettings setting;
#ifdef _DEBUG
    setting.logging_dest = logging::LOG_TO_ALL;
    setting.lock_log = logging::LOCK_LOG_FILE;
#else
    setting.logging_dest = logging::LOG_NONE;
    setting.lock_log = logging::DONT_LOCK_LOG_FILE;
#endif
    setting.log_file = path.value().c_str();
    setting.delete_old = logging::APPEND_TO_OLD_LOG_FILE;
    logging::InitLogging(setting);
    logging::SetLogItems(false, true, true, true);
}

