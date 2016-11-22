#include "easy_http_impl.h"
#include <memory>
#include <string>
#include "third_party/chromium/base/strings/utf_string_conversions.h"
#include "third_party/chromium/base/path_service.h"
#include "third_party/chromium/base/files/file_path.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"
#include "third_party/chromium/base/single_thread_task_runner.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/location.h"
#include "third_party/libcurl/curl/curl.h"
//#include "base/thread_pool.h"

namespace
{
const char* useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; Trident/7.0; rv:11.0) like Gecko";
const char* acceptencode = "gzip";//目前都不应该接收压缩数据，免得解压麻烦
const uint32 max_parallel = 3;

}
EasyHandle::EasyHandle(const HttpRequest& httpParams)
    : curlhandle_(nullptr)
    , headers_(nullptr)
{
    httpRequest_ = httpParams;

    base::FilePath exeDir;
    PathService::Get(base::DIR_MODULE, &exeDir);
    crtPath_ = base::WideToUTF8(
        exeDir.Append(L"curl-ca-bundle.crt").value());
}

EasyHandle::~EasyHandle()
{

}

#define EASYHANDLE_SETOPT(opttype, optvalue) \
    result = curl_easy_setopt(curlhandle_, opttype, optvalue);\
    if (result != CURLE_OK)\
    {\
        LOG(ERROR) << L"Initialize curl_easy_setopt failed!";\
        return false;\
    }

bool EasyHandle::Initialize()
{
    curlhandle_ = curl_easy_init();
    if (curlhandle_==nullptr)
    {
        LOG(ERROR) << L"Initialize curl_easy_init failed!";
        return false;
    }

    CURLcode result = CURLE_OK;
    const HttpRequest& request = httpRequest_;

    if (!curlhandle_)
        return false;
    std::string url = request.url;
    if (!request.queries.empty())
    {
        bool first = true;
        for (const auto& it : request.queries)
        {
            url += first ? "?" : "&";
            first = false;
            url += it.first + "=" + it.second;
        }
    }


    EASYHANDLE_SETOPT(CURLOPT_URL, (url.c_str()));

    if (url.find("https") != std::string::npos)
    {
        EASYHANDLE_SETOPT(CURLOPT_SSL_VERIFYHOST, 0L);
        EASYHANDLE_SETOPT(CURLOPT_SSL_VERIFYPEER, 0L);
    }

    EASYHANDLE_SETOPT(CURLOPT_FOLLOWLOCATION, 1L);
    EASYHANDLE_SETOPT(CURLOPT_HEADER, 0L);
    EASYHANDLE_SETOPT(CURLOPT_TCP_KEEPALIVE, 1L);
    struct curl_slist *headers = 0;
    headers = curl_slist_append(headers, "Connection:Keep-Alive");
    headers = curl_slist_append(headers, "Accept-Language:zh-CN");
    headers = curl_slist_append(headers, "Accept-Encoding:gzip,deflate,sdch");
    headers = curl_slist_append(headers, "Accept:*/*");
    for (const auto& it : request.headers)
    {
        std::string header = it.first + ":" + it.second;
        headers = curl_slist_append(headers, header.c_str());
    }
    EASYHANDLE_SETOPT(CURLOPT_HTTPHEADER, headers);
    EASYHANDLE_SETOPT(CURLOPT_AUTOREFERER, 1L);
    EASYHANDLE_SETOPT(CURLOPT_ACCEPT_ENCODING, acceptencode);

    if (!request.useragent.empty())
    {
        EASYHANDLE_SETOPT(CURLOPT_USERAGENT, request.useragent.c_str());
    }
    else
    {
        EASYHANDLE_SETOPT(CURLOPT_USERAGENT, useragent);
    }
        

    if (!request.referer.empty())
    {
        EASYHANDLE_SETOPT(CURLOPT_REFERER, request.referer.c_str());
    }

    if (!request.cookies.empty())
    {
        EASYHANDLE_SETOPT(CURLOPT_COOKIE, request.cookies.c_str());
    }

    EASYHANDLE_SETOPT(CURLOPT_CAINFO, crtPath_.c_str());
    EASYHANDLE_SETOPT(CURLOPT_FOLLOWLOCATION, 1L);
    EASYHANDLE_SETOPT(CURLOPT_WRITEFUNCTION, EasyHandle::curl_write_callback);
    EASYHANDLE_SETOPT(CURLOPT_WRITEDATA, this);
    EASYHANDLE_SETOPT(CURLOPT_VERBOSE, 1L);
    EASYHANDLE_SETOPT(CURLOPT_PRIVATE, this);//回调结果的时候要使用

    struct curl_httppost* post = nullptr;
    struct curl_httppost* last = nullptr;

    switch (request.method)
    {
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_GET:
        EASYHANDLE_SETOPT(CURLOPT_HTTPGET, 1L);
        break;
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_POST:
        assert(!request.postdata.empty());
        EASYHANDLE_SETOPT(CURLOPT_HTTPPOST, 1L);
        EASYHANDLE_SETOPT(CURLOPT_POSTFIELDSIZE, request.postdata.size());
        EASYHANDLE_SETOPT(CURLOPT_POSTFIELDS, &request.postdata[0]);
        break;
    case HttpRequest::HTTP_METHOD::HTTP_METHOD_HTTPPOST:
        assert(!request.postfile.empty());
        if (!request.postfile.empty())
        {
            curl_formadd(&post, &last, CURLFORM_COPYNAME, "file",
                CURLFORM_FILE, request.postfile.c_str(), CURLFORM_END);
            EASYHANDLE_SETOPT(CURLOPT_HTTPPOST, post);
        }
        break;
    default:
        assert(false);
        break;
    }

    // 设置代理
    if (request.ipproxy.GetProxyType() != IpProxy::PROXY_TYPE::PROXY_TYPE_NONE)
    {
        uint32 proxytype = CURLPROXY_HTTP;
        switch (request.ipproxy.GetProxyType())
        {
        case IpProxy::PROXY_TYPE::PROXY_TYPE_HTTP:
            proxytype = CURLPROXY_HTTP;
            break;
        case IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS4:
            proxytype = CURLPROXY_SOCKS4;
            break;
        case IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS4A:
            proxytype = CURLPROXY_SOCKS4A;
            break;
        case IpProxy::PROXY_TYPE::PROXY_TYPE_SOCKS5:
            proxytype = CURLPROXY_SOCKS5;
            break;
        default:
            assert(false && L"参数错误");
            break;
        }
        std::string proxyip = request.ipproxy.GetProxyIp();
        uint16 proxyport = request.ipproxy.GetProxyPort();

        EASYHANDLE_SETOPT(CURLOPT_PROXYTYPE, proxytype);
        EASYHANDLE_SETOPT(CURLOPT_PROXY, proxyip.c_str());
        EASYHANDLE_SETOPT(CURLOPT_PROXYPORT, proxyport);
    }

    // 持续6秒没有数据到达，就算超时
    EASYHANDLE_SETOPT(CURLOPT_LOW_SPEED_LIMIT, 1L);
    EASYHANDLE_SETOPT(CURLOPT_LOW_SPEED_TIME, 60L);
    return true;
}
    
CURL* EasyHandle::GetHandle() const
{
    assert(curlhandle_);
    return curlhandle_;
}

void EasyHandle::Finalize()
{
    curl_slist_free_all(headers_);
    curl_easy_cleanup(curlhandle_);
}

size_t EasyHandle::curl_write_callback(char* buffer, size_t size, size_t nitems,
                                       void* privatedata)
{
    EasyHandle* easyHandle = reinterpret_cast<EasyHandle*>(privatedata);
    std::vector<uint8> data(reinterpret_cast<uint8*>(buffer), reinterpret_cast<uint8*>(buffer)+size*nitems);
    return easyHandle->WriteCallback(data);;
}

int EasyHandle::WriteCallback(const std::vector<uint8>& data)
{
    responseData_.insert(responseData_.end(), data.begin(), data.end());
    return data.size();
}

int EasyHandle::curl_progress_callback(void *clientp, double dltotal,
                                       double dlnow, double ultotal,
                                       double ulnow)
{
    EasyHandle* easyHandle = reinterpret_cast<EasyHandle*>(clientp);
    int idlnow = static_cast<int>(dlnow);
    int idltotal = static_cast<int>(dltotal);
    return easyHandle->ProgressCallback(idlnow, idltotal);
}

int EasyHandle::ProgressCallback(uint32 dlnow, uint32 dltotal)
{
    //if (httpRequest_.progressFunction)
    //    httpRequest_.progressFunction(dlnow, dltotal);
    return 0;
}

void EasyHandle::FinishCallback(int statuscode)
{
    auto callback = httpRequest_.asyncHttpResponseCallback;
    if (callback)
    {
        HttpResponse httpResponse;
        httpResponse.curlcode = 0;
        httpResponse.statuscode = statuscode;
        httpResponse.content.assign(responseData_.begin(), responseData_.end());
        callback(httpResponse);
    }
}

void EasyHandle::GetResponseData(std::vector<uint8>* responsedata)
{
    responsedata->assign(responseData_.begin(), responseData_.end());
}

AsyncHttpResponseCallback EasyHandle::GetHttpReply() const
{
    return httpRequest_.asyncHttpResponseCallback;
}

TaskQueue::TaskQueue()
{

}

TaskQueue::~TaskQueue()
{

}

bool TaskQueue::AddTask(std::shared_ptr<EasyHandle> easyHandle)
{
    base::AutoLock lock(lock_);
    queue_.push_back(easyHandle);
    return true;
}

bool TaskQueue::GetTasks(std::deque<std::shared_ptr<EasyHandle>>* easyHandleQueue)
{
    base::AutoLock lock(lock_);
    easyHandleQueue->swap(queue_);
    return true;
}

bool TaskQueue::GetOneTask(std::shared_ptr<EasyHandle>* easyHandle)
{
    base::AutoLock lock(lock_);
    if (queue_.empty())
        return false;

    *easyHandle = *queue_.begin();
    queue_.erase(queue_.begin());
    return true;
}

EasyHttpImpl::EasyHttpImpl()
    :shutdownFlag_(false)
    , thread_(new base::Thread("EasyHttpImpl"))
{
    curl_global_init(CURL_GLOBAL_ALL);
    thread_->Start();
}

EasyHttpImpl::~EasyHttpImpl()
{
    curl_global_cleanup();
}

bool EasyHttpImpl::AsyncHttpRequest(const HttpRequest& httpParams)
{
    std::shared_ptr<EasyHandle> easyHandle(new EasyHandle(httpParams));
    taskQueue_.AddTask(easyHandle);
    thread_->message_loop()->PostTask(FROM_HERE, base::Bind(&EasyHttpImpl::DoHttpRequest, 
        base::Unretained(this)));

    return true;
}

void EasyHttpImpl::ShutdownService()
{
    shutdownFlag_ = true;

    if (thread_)
        thread_->Stop();
}

void EasyHttpImpl::DoHttpRequest()
{
    auto multi_handle = curl_multi_init();

    std::map<CURL*, std::shared_ptr<EasyHandle>> easyHandleQueue;

    int still_running = 0;
    do
    {
        if (shutdownFlag_)// 只有在ShutdownService的时候强制退出
            break;

        if (still_running < max_parallel)
        {
            std::shared_ptr<EasyHandle> task;
            if (taskQueue_.GetOneTask(&task))
            {
                if (!task->Initialize())
                {
                    task->FinishCallback(0);
                }
                else
                {
                    easyHandleQueue[task->GetHandle()] = task;
                    int result = curl_multi_add_handle(multi_handle, task->GetHandle());
                    if (result != CURLM_OK)
                    {
                        task->FinishCallback(0);
                    }
                }
            }
        }

        struct timeval timeout;
        int rc; /* select() return code */
        CURLMcode mc; /* curl_multi_fdset() return code */

        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;

        long curl_timeo = -1;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to play around with */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        curl_multi_timeout(multi_handle, &curl_timeo);
        if (curl_timeo >= 0)
        {
            timeout.tv_sec = curl_timeo / 1000;
            if (timeout.tv_sec > 1)
                timeout.tv_sec = 1;
            else
                timeout.tv_usec = (curl_timeo % 1000) * 1000;
        }

        /* get file descriptors from the transfers */
        mc = curl_multi_fdset(multi_handle, &fdread, &fdwrite, &fdexcep, &maxfd);

        if (mc != CURLM_OK)
        {
            fprintf(stderr, "curl_multi_fdset() failed, code %d.\n", mc);
            break;
        }

        /* On success the value of maxfd is guaranteed to be >= -1. We call
        select(maxfd + 1, ...); specially in case of (maxfd == -1) there are
        no fds ready yet so we call select(0, ...) --or Sleep() on Windows--
        to sleep 100ms, which is the minimum suggested value in the
        curl_multi_fdset() doc. */

        if (maxfd == -1)
        {
            Sleep(100);
            rc = 0;
        }
        else
        {
            /* Note that on some platforms 'timeout' may be modified by select().
            If you need access to the original value save a copy beforehand. */
            rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }

        switch (rc)
        {
        case -1:
            /* select error */
            break;
        case 0: /* timeout */
        default: /* action */
            curl_multi_perform(multi_handle, &still_running);
            break;
        }

        CURLMsg *msg; /* for picking up messages with the transfer status */
        int msgs_left; /* how many messages are left */

        /* See how the transfers went */
        while (msg = curl_multi_info_read(multi_handle, &msgs_left))
        {
            if (msg->msg != CURLMSG_DONE)
                continue;

            CURL *curlHandle = msg->easy_handle;
            CURLcode curlCode = CURLE_OK;
            int statusCode = 0;
            curlCode = curl_easy_getinfo(curlHandle, CURLINFO_RESPONSE_CODE, 
                                          &statusCode);
            if (CURLE_OK != curlCode)
            {
                assert(false && L"curl_easy_getinfo error!");
                LOG(ERROR) << L"curl_easy_getinfo error!";
            }

            void *extracted = nullptr;
            curlCode = curl_easy_getinfo(curlHandle, CURLINFO_PRIVATE, &extracted);
            if (CURLE_OK != curlCode)
            {
                assert(false && L"curl_easy_getinfo error!");
                LOG(ERROR) << L"curl_easy_getinfo error!";
            }

            if (extracted)
            {
                EasyHandle* tempEasyHandle = reinterpret_cast<EasyHandle*>(extracted);
                tempEasyHandle->FinishCallback(statusCode);
                curl_multi_remove_handle(multi_handle, curlHandle);
                tempEasyHandle->Finalize();
                easyHandleQueue.erase(tempEasyHandle->GetHandle());
            }
        }
    }
    while (still_running);

    curl_multi_cleanup(multi_handle);

    return;
}
