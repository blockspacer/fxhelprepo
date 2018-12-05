#pragma once
#include "stdafx.h"
#include "common.h"

extern const uint32 kPlatform = 6;
const uint32 kVersion = 3910;
std::wstring kUseragent = L"�ṷֱ�� 3.9.1 rv:3.9.1.0 (iPhone; iOS 10.3.3; zh_CN)";
const uint32 kPagesize = 80;

//baiduCode = 257 & cityName = ������&page = 1 & pageSize = 80 & platform = 5 & version = 3302$_fan_xing_$
//
//����sing�ķ���
//cityNameʹ�����ı��룬��Ҫת����urlencode
//1. ���в�����key - value��ʽ�Ž�map��
//2. ��key�������г�key1 = value1&key2 = value2�ĸ�ʽ���ַ���str1
//3. ��str1�������$_fan_xing_$����Ȼ�����md5ֵ
//4. ȡmd5�ĵ�8λ��24λ��һ����16λ���ݡ�
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

