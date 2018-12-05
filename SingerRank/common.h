#pragma once

#include <map>
#include <string>

#include "third_party/chromium/base/basictypes.h"
#include "Network/EncodeHelper.h"

extern const uint32 kPlatform;
extern const uint32 kVersion;
extern std::wstring kUseragent;
extern const uint32 kPagesize;

//baiduCode = 257 & cityName = 广州市&page = 1 & pageSize = 80 & platform = 5 & version = 3302$_fan_xing_$
//
//计算sing的方案
//cityName使用中文编码，不要转换成urlencode
//1. 所有参数以key - value方式放进map中
//2. 按key升序排列出key1 = value1&key2 = value2的格式的字符串str1
//3. 在str1后面紧跟$_fan_xing_$串，然后计算md5值
//4. 取md5的第8位到24位，一共是16位数据。

std::string GetSignFromMap(const std::map<std::string, std::string>& param_map);

