#ifndef _EASY_HTTP_IMPL_H_
#define  _EASY_HTTP_IMPL_H_
#include <memory>
#include <vector>
#include <queue>
#include "CurlWrapper.h"
#include "third_party/chromium/base/memory/ref_counted.h"
#include "third_party/chromium/base/basictypes.h"
#include "third_party/chromium/base/synchronization/lock.h"
#include "third_party/libcurl/curl/curlbuild.h"
#include "third_party/chromium/base/threading/thread.h"

typedef void CURLM;
typedef void CURL;
struct curl_slist;

namespace base
{
class SequencedTaskRunner;
}

class HttpRequest;
class HttpResponse;

class EasyHandle
{
public:
    EasyHandle(const HttpRequest& httpParams);
    ~EasyHandle();
    bool Initialize();
    CURL* GetHandle() const;
    void Finalize();

    // 回调使用
    static size_t curl_write_callback(char* buffer, size_t size, size_t nitems,
                                      void* privatedata);
    int WriteCallback(const std::vector<uint8>& data);

    static int curl_progress_callback(void *clientp, double dltotal, 
                                      double dlnow, double ultotal,
                                      double ulnow);

    int ProgressCallback(uint32 dlnow, uint32 dltotal);

    void FinishCallback(int statuscode);

    AsyncHttpResponseCallback GetHttpReply() const;
    void GetResponseData(std::vector<uint8>* responsedata);

private:
    CURL* curlhandle_;
    std::string crtPath_;
    struct curl_slist *headers_;
    std::vector<uint8> responseData_;
    HttpRequest httpRequest_;
};

class TaskQueue
{
public:
    TaskQueue();
    ~TaskQueue();

    bool AddTask(std::shared_ptr<EasyHandle> easyHandle);
    bool GetTasks(std::deque<std::shared_ptr<EasyHandle>>* easyHandleQueue);

private:
    std::deque<std::shared_ptr<EasyHandle>> queue_;
    base::Lock lock_;

};

// 转换到EasyHttpImpl接口中
class EasyHttpImpl
{
public:
    EasyHttpImpl();
    ~EasyHttpImpl();

    bool AsyncHttpRequest(const HttpRequest& httpParams);
    void ShutdownService();
private:
    void DoHttpRequest();

    TaskQueue taskQueue_;
    std::unique_ptr<base::Thread> thread_;
    bool shutdownFlag_;
};

#endif // !_EASY_HTTP_IMPL_H_
