#pragma once
#include "stdafx.h"
#include "common.h"

extern const uint32 kPlatform = 6;
const uint32 kVersion = 3910;
std::wstring kUseragent = L"酷狗直播 3.9.1 rv:3.9.1.0 (iPhone; iOS 10.3.3; zh_CN)";
const uint32 kPagesize = 80;

//baiduCode = 257 & cityName = 广州市&page = 1 & pageSize = 80 & platform = 5 & version = 3302$_fan_xing_$
//
//计算sing的方案
//cityName使用中文编码，不要转换成urlencode
//1. 所有参数以key - value方式放进map中
//2. 按key升序排列出key1 = value1&key2 = value2的格式的字符串str1
//3. 在str1后面紧跟$_fan_xing_$串，然后计算md5值
//4. 取md5的第8位到24位，一共是16位数据。
std::string GetSignFromMap(const std::map<std::string, std::string>& param_map)
{
    if (param_map.empty())
        return std::string("");

    std::string target;
    for (const auto& it : param_map)
    {
        target += it.first + "=" + it.second + "&";
    }
    target = target.substr(0, target.length() - 1);
    target += "$_fan_xing_$";
    std::string md5 = MakeMd5FromString(target);

    return md5.substr(8, 16);
}

