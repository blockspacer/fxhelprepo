#include "HousingRequest.h"

#include "Network/easy_http_impl.h"

namespace
{
static const char* mark_thead_begin = "<thead";
static const char* mark_thead_end = "</thead>";

static const char* mark_table_begin = "<table";
static const char* mark_table_end = "</table>";
static const char* mark_tr_begin = "<tr";
static const char* mark_tr_end = "</tr>";
static const char* mark_td_begin = "<td ";
static const char* mark_td_end = "</td>";
static const char* mark_a_begin = "<a ";
static const char* mark_a_end = "</a>";

bool GetMarkData(const std::string& pagedata, size_t beginpos,
    const std::string& beginmark,
    const std::string& endmark,
    std::string* targetdata,
    size_t *afterendmarkpos)
{
    beginpos = pagedata.find(beginmark, beginpos);
    if (beginpos == std::string::npos)
        return false;
    //beginpos += beginmark.size();

    size_t pos = beginpos;
    int pair_count = 0;
    while (pos < pagedata.size())
    {
        switch (pagedata[pos])
        {
        case '<':
            pair_count++;
            break;
        case '>':
            pair_count--;
            break;
        default:
            break;
        }

        // 完全越过标签了
        if (!pair_count)
            break;
        pos++;
    }

    beginpos = pos+1;

    auto endpos = pagedata.find(endmark, beginpos);
    if (endpos == std::string::npos)
        return false;

    *afterendmarkpos = endpos + endmark.size();
    *targetdata = pagedata.substr(beginpos, endpos - beginpos);
    return true;
}

bool GetTheadData(const std::string& pagedata, std::string* theaddata)
{
    size_t theadendpos = 0;
    return GetMarkData(pagedata, 0, mark_thead_begin, mark_thead_end,
        theaddata, &theadendpos);
}

bool GetTableData(const std::string& pagedata, std::string* tabledata)
{
    size_t tableendpos = 0;
    return GetMarkData(pagedata, 0, mark_table_begin, mark_table_end,
        tabledata, &tableendpos);
}

bool GetTrData(const std::string& pagedata, size_t beginpos,
    size_t* trendpos, std::string* trdata)
{
    return GetMarkData(pagedata, beginpos, mark_tr_begin, mark_tr_end, trdata, trendpos);
}

bool GetTdData(const std::string& pagedata, size_t beginpos, size_t* tdendpos, std::string* tddata)
{
    return GetMarkData(pagedata, beginpos, mark_td_begin, mark_td_end, tddata, tdendpos);
}

bool GetAData(const std::string& pagedata, size_t beginpos,
    std::string* trdata, size_t* trendpos)
{
    return GetMarkData(pagedata, beginpos, mark_a_begin, mark_a_end, trdata, trendpos);
}
}

HousingRequest::HousingRequest()
{
    curl_.Initialize();
}


HousingRequest::~HousingRequest()
{
    curl_.Finalize();
}


bool HousingRequest::GetClfSearch()
{
    HttpRequest request;
    request.url = "http://housing.gzcc.gov.cn/search/clf/clfSearch.jsp";
    //request.url = "http://housing.gzcc.gov.cn//search/clf/clf_detail.jsp?pyID=26426274";
    request.method = HttpRequest::HTTP_METHOD::HTTP_METHOD_GET;
    request.headers["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8";
    request.headers["Accept-Language"] = "zh-CN,zh;q=0.8,en;q=0.6";

    HttpResponse response;
    if (!curl_.Execute(request, &response))
    {
        return false;
    }

    std::string content(response.content.begin(),response.content.end());

    std::list<std::vector<std::string>> record_list;
    if (!ParseClfSearch(content, &record_list))
    {
        return false;
    }
    
    return true;
}

bool HousingRequest::ParseClfSearch(const std::string& data,
    std::list<std::vector<std::string>>* record_list) const
{
    auto pos = data.find("/images/title_info_list_clf.jpg");
    if (pos== std::string::npos)
        return false;

    std::string contain_table = data.substr(pos + 1);

    std::string table_data;
    if (!GetTableData(contain_table, &table_data))
        return false;

    // 从表格获取列头
    //std::string thead_data;
    //if (!GetTheadData(table_data, &thead_data))
    //    return false;

    std::string header_tr_data;
    size_t tr_end;
    if (!GetTrData(table_data, 0, &tr_end, &header_tr_data))
        return false;

    // 从表格获取数据，目前方案是每页15条，多页未处理。
    std::string tr_data;
    for (uint32 index = 0; index < 15; index++)
    {
        if (!GetTrData(table_data, tr_end + 1, &tr_end, &tr_data))
            return false;

        std::vector<std::string> record;
        ParseOneRecode(tr_data, &record);
        record_list->push_back(record);
    }


    return true;
}

bool HousingRequest::Test()
{
    std::string temp = R"(
        <td align="center" class="box_tab_style02_td">1</td>
        <td align="center" class="box_tab_style02_td"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">0170150853</a></td>
        <td align="center" class="box_tab_style02_td padding_left10px"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">番禺区</a></td>
        <td align="left" class="box_tab_style02_td"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">广州市番禺区钟村街祈福新村海晴居1街9B号1楼</a></td>
        <td align="center" class="box_tab_style02_td padding_left10px"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">280.00</a></td>
       <td align="center" class="box_tab_style02_td padding_left10px"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">室厅</a></td>
		<td align="center" class="box_tab_style02_td"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">89.94</a></td>
		<td align="center" class="box_tab_style02_td"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">放盘</a></td>
		<td align="center" class="box_tab_style02_td"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">广州旺一达房地产咨询服务有限公司</a></td>
		<td align="center" class="box_tab_style02_td"><a href="/search/clf/clf_detail.jsp?pyID=26426274" target="_blank">2017-04-09</a></td>)";

    std::vector<std::string> record;
    ParseOneRecode(temp, &record);
    return true;
}

bool HousingRequest::ParseOneRecode(
    const std::string& data, std::vector<std::string>* house_record) const
{
    std::vector<std::string> record_data;
    std::string temp_data = data;
    std::string tr_data;
    size_t last_tr_end = 0;
    size_t current_tr_end;
    if (!GetTdData(temp_data, last_tr_end, &current_tr_end, &tr_data))
        return false;

    last_tr_end = current_tr_end;
    record_data.push_back(tr_data);

    for (int i = 0; i < 9; i++)
    {
        std::string tr_data;
        if (!GetTdData(temp_data, last_tr_end, &current_tr_end, &tr_data))
            return false;

        last_tr_end = current_tr_end;
        size_t a_end;
        std::string a_data;
        if (!GetAData(tr_data, 0, &a_data, &a_end))
            return false;

        record_data.push_back(a_data);
    }
}

bool HousingRequest::GetCountCookie()
{
    return false;
}

bool HousingRequest::GetVerifyCode()
{
    return false;
}

