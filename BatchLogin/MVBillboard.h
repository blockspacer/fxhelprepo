#pragma once
#include <memory>

class CurlWrapper;

class MVBillboard
{
public:
    MVBillboard();
    ~MVBillboard();

    bool UpMVBillboard(const std::string& cookie,
        const std::string& collectionid,
        const std::string& mvid);

    bool MVPlayVedio(const std::string& cookie,
        const std::string& collectionid,
        const std::string& mvid);

    bool MVAddPraise(const std::string& cookie,
        const std::string& collectionid,
        const std::string& mvid);

    bool MVCollect(const std::string& cookie,
        const std::string& collectionid,
        const std::string& mvid);

    bool MVShared(const std::string& cookie, 
        const std::string& collectionid,
        const std::string& mvid);

private:
    bool MVAction_(const std::string& url, const std::string& cookie,
        const std::string& collectionid, const std::string& mvid);

    std::unique_ptr<CurlWrapper> curlWrapper_;
};

