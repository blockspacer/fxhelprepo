#pragma once

#include <string>

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
