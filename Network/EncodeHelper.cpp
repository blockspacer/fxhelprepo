#include "stdafx.h"
#include "encodehelper.h"
#include <windows.h>
#include <wchar.h>
#include <vector>
#include <assert.h>
#include "third_party/libcurl/curl/curl.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"


static const wchar_t HEX[] = L"0123456789ABCDEF";
std::wstring BinToHex(const void* bin, int len)
{
    const unsigned char* b = static_cast<const unsigned char*>(bin);
    std::vector<wchar_t> hex(len * 2);
    std::vector<wchar_t>::iterator buf = hex.begin();
    for (int i = 0; i < len; ++i)
    {
        *buf++ = HEX[b[i] >> 4];
        *buf++ = HEX[b[i] & 0x0F];
    }
    return std::wstring(hex.begin(), hex.end());
}

std::string BinToAnsiHex(const void* bin, int len)
{
    const unsigned char* b = static_cast<const unsigned char*>(bin);
    std::vector<char> hex(len * 2);
    std::vector<char>::iterator buf = hex.begin();
    for (int i = 0; i < len; ++i)
    {
        *buf++ = static_cast<char>(HEX[b[i] >> 4]);
        *buf++ = static_cast<char>(HEX[b[i] & 0x0F]);
    }
    return std::string(hex.begin(), hex.end());
}

bool HexToBin(const wchar_t* hex, int hexLen, void* bin, int* binLen)
{
    assert(hexLen / 2 <= *binLen);
    char* pout = static_cast<char*>(bin);
    int outLen = 0;

    wchar_t hexCharBuf[3] = { L'\0' };
    const wchar_t* hexEnd = hex + hexLen / 2 * 2;
    while ((hex != hexEnd) && (outLen < *binLen))
    {
        int i = 0;
        while ((i < 2) && (hex != hexEnd))
        {
            while ((hex != hexEnd) && !iswxdigit(*hex))
                ++hex;
            if (hex != hexEnd)
                hexCharBuf[i++] = *hex++;
            else
                break;
        }
        if (i == 2)
        {
            int ch = 0;
            int n = swscanf(hexCharBuf, L"%2x", &ch);
            assert(n == 1);
            *pout++ = ch;
            ++outLen;
        }
        else
        {
            break;
        }
    }
    *binLen = outLen;
    return hexLen / 2 == *binLen;
}

bool WideToMBS(const std::wstring& ws, std::string* s)
{
    const wchar_t *wcharstring = ws.c_str();
    int len = ws.length();
    char* charstring = new char[len + 1];
    WideCharToMultiByte(CP_ACP, NULL, wcharstring, -1, charstring, len + 1, NULL, NULL);
    s->assign(charstring, charstring + len);
    return true;
}

std::wstring MbsToWide(const std::string& str)
{
    auto len = str.length();
    const char* p = str.c_str();
    wchar_t* wp = new wchar_t[len+1];
    MultiByteToWideChar(CP_ACP, NULL, p, len, wp,len);
    wp[len] = 0;
    std::wstring ws(wp);
    delete[] wp;
    return std::move(ws);
}

std::string UrlEncode(const std::string& str)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        char *output = curl_easy_escape(curl, str.c_str(), str.length());
        if (output)
        {
            std::string result(output);
            printf("Encoded: %sn", output);
            curl_free(output);
            return std::move(result);
        }
    }
    return "";
}

std::string UrlDecode(const std::string& str)
{
    CURL *curl = curl_easy_init();
    if (curl)
    {
        int outlen = 0;
        char *output = curl_easy_unescape(curl, str.c_str(), str.length(), &outlen);
        if (output)
        {
            std::string result;
            result.assign(output, output + outlen);
            curl_free(output);
            return std::move(result);
        }
    }
    return "";
}

std::string WideToGBK(const std::wstring& text)
{
    return base::SysWideToMultiByte(text, 936);
}

std::wstring GBKToWide(const std::string& text)
{
    return base::SysMultiByteToWide(text, 936);
}

