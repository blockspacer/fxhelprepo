#pragma once

#include <list>
#include "third_party/chromium/base/basictypes.h"
#include "Network/EncodeHelper.h"
#include "Network/CurlWrapper.h"
#include "Network/CookiesHelper.h"

struct HouseRecord
{
    std::string house_source_index;
    std::string house_area;
    std::string house_addr;
    std::string house_price;
    std::string house_style; // 室厅
    std::string house_acreage; // 面积
    std::string house_state; // 放盘
    std::string house_intermediary; // 中介商
    std::string house_date; // 放盘时间
};

class HousingRequest
{
public:
    HousingRequest();
    ~HousingRequest();

    // http://housing.gzcc.gov.cn/fyxx/ysz/
    // ysz = 预售证
    bool GetYszResult(std::vector<std::string>* headers,
        std::list<std::vector<std::string>>* record_list);

    bool Test();

private:
    bool GetFirstPageContent(std::string* content);
    bool GetPageContentByNumber(const std::string& page_number, std::string* content);
    bool ParsePageCount(const std::string& data, uint32* page_count) const;
    bool ParseYszResult(const std::string& data, std::vector<std::string>* headers,
        std::list<std::vector<std::string>>* record_list) const;
    bool ParseOneRecode(const std::string& data, std::vector<std::string>* house_record) const;
    bool ParseHeader(const std::string& data, std::vector<std::string>* house_header) const;

    CurlWrapper curl_;
};

