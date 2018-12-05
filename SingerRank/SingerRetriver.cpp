#include "stdafx.h"
#include "SingerRetriver.h"

#include "third_party/chromium/base/single_thread_task_runner.h"
#include "third_party/chromium/base/bind.h"
#include "third_party/chromium/base/threading/thread.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

#include "Network/CurlWrapper.h"
#include "Network/EncodeHelper.h"

SingerRetriver::SingerRetriver()
{
}


SingerRetriver::~SingerRetriver()
{
}

bool SingerRetriver::Initialize(base::SingleThreadTaskRunner* runner)
{
    runner_ = runner;
    return true;
}

void SingerRetriver::Finalize()
{

}

void SingerRetriver::BreakRequest()
{

}

bool SingerRetriver::GetSingerInfoByClan(uint32 clan_id,
    const base::Callback<void(const std::vector<uint32>&)>& callback)
{
    if (!runner_->RunsTasksOnCurrentThread())
    {
        runner_->PostTask(FROM_HERE,
            base::Bind(base::IgnoreResult(&SingerRetriver::GetSingerInfoByClan),
                base::Unretained(this), clan_id, callback));
        return true;
    }

    DCHECK(runner_->RunsTasksOnCurrentThread());

    std::string base_url = "http://visitor.fanxing.kugou.com/VServices/Clan.ClanServices.getClanStarListPaging/";
    base_url += base::UintToString(clan_id) + "-";
    std::string url = base_url + "1-12/";
    std::string refer = "http://fanxing.kugou.com/index.php?action=clan&id=" + base::UintToString(clan_id);

    std::vector<uint32> roomids;
    int32 all_page = 0;
    if (!DoGetSingerInfoPageByClan(url, refer, &roomids, &all_page))
        return false;

    for (int page_num = 2; page_num < all_page; page_num++)
    {
        std::vector<uint32> temp;
        url = base_url + base::IntToString(page_num) + "-12/";
        DoGetSingerInfoPageByClan(url, refer, &temp, nullptr);
        if (!temp.empty())
            roomids.insert(roomids.end(), temp.begin(), temp.end());
    }
    callback.Run(roomids);
    return true;
}

bool SingerRetriver::DoGetSingerInfoPageByClan(
    const std::string& url, const std::string& refer,
    std::vector<uint32>* roomids, int32* all_page)
{
    HttpRequest request;
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.url = url;
    request.referer = refer;
    request.useragent = "酷狗直播 3.9.1 rv:3.9.1.0 (iPhone; iOS 10.3.3; zh_CN)";

    // 数据分析
    CurlWrapper curl_wrapper;
    HttpResponse http_reponse;
    if (!curl_wrapper.Execute(request, &http_reponse))
        return false;

    std::string content;
    content.assign(http_reponse.content.begin(),
        http_reponse.content.end());

    content = PickJson(content);
    Json::Reader reader;
    Json::Value root(Json::objectValue);
    if (!reader.parse(content, root, false))
        return false;

    uint32 servertime = GetInt32FromJsonValue(root, "servertime");
    if (servertime == 0)
        return false;

    Json::Value defaultval(Json::objectValue);
    auto data = root.get("data", defaultval);
    if (!data.isObject())
        return false;

    if (all_page)
        *all_page = GetInt32FromJsonValue(data, "totalPage");

    auto singer_list = data.get("list", defaultval);
    if (!singer_list.isArray())
        return false;

    for (auto singer_info_obj : singer_list)
    {
        int32 roomid = GetInt32FromJsonValue(singer_info_obj, "roomId");
        if (roomid)
            roomids->push_back(roomid);
    }
    return true;
}

