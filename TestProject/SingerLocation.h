#pragma once
#include <memory>

class EasyHttpImpl;
class HttpResponse;
class SingerLocation
{
public:
    SingerLocation();
    ~SingerLocation();

    void Test();

private:
    void HttpResponseCallback(const HttpResponse& response);

    std::unique_ptr<EasyHttpImpl> easy_http_impl_;
};

