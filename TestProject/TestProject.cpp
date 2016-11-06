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
#include "third_party/zlib/zlib.h"

bool MakePostdata(const std::map<std::string, std::string>& postmap,
    std::vector<uint8>* postdata)
{
    if (postmap.empty())
        return false;

    std::string temp;
    bool first = true;
    for (const auto& param : postmap)
    {
        if (first)
            first = false;
        else
            temp += "&";

        temp += param.first + "=" + UrlEncode(param.second);
    }
    postdata->assign(temp.begin(), temp.end());
    return true;
}

void DoRobotTest()
{
    CurlWrapper curlWrapper;
    HttpRequest httpRequest;
    httpRequest.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_POST;
    httpRequest.url = "http://www.tuling123.com//openapi/api";
    std::map<std::string, std::string> postmap;
    postmap["key"] = "21ef7a39254642f721736081e8e2226d";
    postmap["info"] = "chat info";
    postmap["userid"] = "123";
    bool result = MakePostdata(postmap, &httpRequest.postdata);

    HttpResponse httpResponse;
    result = curlWrapper.Execute(httpRequest, &httpResponse);

}

int ZLibTest()
{
    char text[] = "zlib compress and uncompress test\nturingo@163.com\n2012-11-05\n";
    uLong tlen = strlen(text) + 1;	/* 需要把字符串的结束符'\0'也一并处理 */
    char* buf = NULL;
    uLong blen;

    /* 计算缓冲区大小，并为其分配内存 */
    blen = compressBound(tlen);	/* 压缩后的长度是不会超过blen的 */
    if ((buf = (char*)malloc(sizeof(char) * blen)) == NULL)
    {
        printf("no enough memory!\n");
        return -1;
    }

    /* 压缩 */
    if (compress((unsigned char*)buf, &blen, (unsigned char*)text, tlen) != Z_OK)
    {
        printf("compress failed!\n");
        return -1;
    }

    /* 解压缩 */
    if (uncompress((unsigned char*)text, &tlen, (unsigned char*)buf, blen) != Z_OK)
    {
        printf("uncompress failed!\n");
        return -1;
    }

    /* 打印结果，并释放内存 */
    printf("%s", text);
    if (buf != NULL)
    {
        free(buf);
        buf = NULL;
    }
    return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{   
    base::AtExitManager atExitManager;

    NetworkInitialize();

    ZLibTest();

    NetworkFainalize();
	return 0;
}

