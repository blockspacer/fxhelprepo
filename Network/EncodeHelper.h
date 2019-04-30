#pragma once

#include <vector>
#include <string>
#undef max // ��Ϊ΢�����������ĳЩͷ�ļ�������max��
#undef min // ��Ϊ΢�����������ĳЩͷ�ļ�������min��
#include "third_party/json/json.h"
#include "third_party/chromium/base/time/time.h"

typedef std::vector<std::wstring> RowData;
typedef std::vector<RowData> GridData;

std::wstring BinToHex(const void* bin, int len);
std::string BinToAnsiHex(const void* bin, int len);
bool HexToBin(const wchar_t* hex, int hexLen, void* bin, int* binLen);

bool WideToMBS(const std::wstring& ws, std::string* s);
std::wstring MbsToWide(const std::string& str);

template<typename T>
std::wstring GetWstring(T data)
{
    return std::wstring(L"0x") + BinToHex((void*)&data, sizeof(data));
}

std::string UrlEncode(const std::string& str);
std::string UrlDecode(const std::string& str);

std::string WideToGBK(const std::wstring& text);
std::wstring GBKToWide(const std::string& text);

std::string WideToUtf8(const std::wstring& text);
std::wstring Utf8ToWide(const std::string& text);

std::string MakeMd5FromString(const std::string& text);

////�ṩ��\u��ʽ��ת������ ���磺\u5929\u795e\u8d50\u8d22 תΪ ����Ͳ� ��utf8����
//std::string str = R"(\u5929\u795e\u8d50\u8d22)";
//std::string result;
//UnicodeToUtf8(str, &result);
bool UnicodeToUtf8(const std::string& unicode, std::string* utf8);

std::string MakeFormatTimeStringFromUnixTime(uint32 unix_time);

std::string MakeFormatTimeString(const base::Time time);

std::string MakeFormatDateString(const base::Time time);

// ��ȡ13λ�ĵ�ǰʱ���ַ���
std::string GetNowTimeString();

std::vector<std::string> SplitString(std::string str, const std::string& pattern);

std::string PickJson(const std::string& rawresponse);

void RemoveSpace(std::string* str);

uint32 GetInt32FromJsonValue(const Json::Value& jvalue, const std::string& name);

int64 GetInt64FromJsonValue(const Json::Value& jvalue, const std::string& name);

double GetDoubleFromJsonValue(const Json::Value& jvalue, const std::string& name);

std::string RSADecryptString(std::istream* privFilename, const std::string& ciphertext);

std::string RSAEncryptString(std::istream* pubFilename, const std::string& message);
