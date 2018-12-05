#pragma once

#include <map>
#include <string>

#include "third_party/chromium/base/basictypes.h"
#include "Network/EncodeHelper.h"

extern const uint32 kPlatform;
extern const uint32 kVersion;
extern std::wstring kUseragent;
extern const uint32 kPagesize;

//baiduCode = 257 & cityName = ������&page = 1 & pageSize = 80 & platform = 5 & version = 3302$_fan_xing_$
//
//����sing�ķ���
//cityNameʹ�����ı��룬��Ҫת����urlencode
//1. ���в�����key - value��ʽ�Ž�map��
//2. ��key�������г�key1 = value1&key2 = value2�ĸ�ʽ���ַ���str1
//3. ��str1�������$_fan_xing_$����Ȼ�����md5ֵ
//4. ȡmd5�ĵ�8λ��24λ��һ����16λ���ݡ�

std::string GetSignFromMap(const std::map<std::string, std::string>& param_map);

