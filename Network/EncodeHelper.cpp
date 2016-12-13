#include "stdafx.h"
#include "encodehelper.h"
#include <windows.h>
#include <wchar.h>
#include <vector>
#include <assert.h>
#include "third_party/libcurl/curl/curl.h"
#include "third_party/chromium/base/strings/sys_string_conversions.h"
#include "third_party/chromium/base/md5.h"
#include "third_party/chromium/base/strings/string_piece.h"
#include "third_party/chromium/base/strings/string_number_conversions.h"

#include "third_party/cryptopp/cryptlib.h"
#include "third_party/cryptopp/rsa.h"
#include "third_party/cryptopp/aes.h"
#include "third_party/cryptopp/modes.h"
#include "third_party/cryptopp/filters.h"
#include "third_party/cryptopp/files.h"
#include "third_party/cryptopp/hex.h"
#include "third_party/cryptopp/randpool.h"

namespace
{
    static CryptoPP::OFB_Mode<CryptoPP::AES>::Encryption s_globalRNG;

    CryptoPP::RandomNumberGenerator & GlobalRNG()
    {
        return s_globalRNG;
    }

    std::string RSADecryptString_(std::istream* privFilename, const std::string& ciphertext)
    {
        using namespace CryptoPP;
        FileSource privFile(*privFilename, true, new HexDecoder);
        RSAES_OAEP_SHA_Decryptor priv(privFile);

        std::string result;
        StringSource(ciphertext.c_str(), true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
        return result;
    }


    std::string RSAEncryptString_(std::istream* pubFilename, const std::string& seed, const std::string& message)
    {
        using namespace CryptoPP;
        FileSource pubFile(*pubFilename, true, new HexDecoder);
        RSAES_OAEP_SHA_Encryptor pub(pubFile);

        RandomPool randPool;
        randPool.IncorporateEntropy((byte *)seed.c_str(), seed.length());

        std::string result;
        StringSource(message, true, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(result))));
        return result;
    }
}
static const wchar_t HEX[] = L"0123456789abcdef";
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
    std::string result;
    if (!curl)
        return result;

    char *output = curl_easy_escape(curl, str.c_str(), str.length());
    if (output)
    {
        result = output;
        printf("Encoded: %sn", output);
        curl_free(output);
    }
    curl_easy_cleanup(curl);

    return std::move(result);
}

std::string UrlDecode(const std::string& str)
{
    CURL *curl = curl_easy_init();
    std::string result;
    if (!curl)
        return result;

    int outlen = 0;
    char *output = curl_easy_unescape(curl, str.c_str(), str.length(), &outlen);
    if (output)
    {
        result.assign(output, output + outlen);
        curl_free(output);
    }
    curl_easy_cleanup(curl);

    return std::move(result);
}

std::string WideToGBK(const std::wstring& text)
{
    return base::SysWideToMultiByte(text, 936);
}

std::wstring GBKToWide(const std::string& text)
{
    return base::SysMultiByteToWide(text, 936);
}

std::string WideToUtf8(const std::wstring& text)
{
    return base::SysWideToUTF8(text);
}
std::wstring Utf8ToWide(const std::string& text)
{
    return base::SysUTF8ToWide(text);
}

std::string MakeMd5FromString(const std::string& text)
{
    base::MD5Context contex;
    base::MD5Init(&contex);
    base::MD5Digest md5;
    base::MD5Update(&contex, base::StringPiece(text));
    base::MD5Final(&md5, &contex);
    return BinToAnsiHex(md5.a, sizeof(md5));
}

bool UnicodeToUtf8(const std::string& unicode, std::string* utf8)
{
    std::string temp = unicode;
    auto pos = temp.find("\\u");
    while (pos!=std::string::npos)
    {
        temp.replace(pos, 2, "");
        pos = temp.find("\\u");
    }
    std::wstring wstr = Utf8ToWide(temp);
    int len = wstr.length();
    char* str = new char[len];
    bool result = HexToBin(wstr.c_str(), wstr.length(), str, &len);
    if (!result)
    {
        return false;
    }

    std::wstring w;
    int d = 0;
    while (d < len)
    {
        char wc[2] = { 0 };
        wc[1] = str[d];
        wc[0] = str[d + 1];
        w.push_back(*(wchar_t*)(wc));
        d += 2;
    }
    
    *utf8 = WideToUtf8(w);
    return true;
}

std::string MakeFormatTimeString(const base::Time time)
{
    base::Time::Exploded exploded;
    time.LocalExplode(&exploded);
    std::string hour = base::IntToString(exploded.hour);
    if (hour.length() < 2)
    {
        hour = "0" + hour;
    }
    std::string minute = base::IntToString(exploded.minute);
    if (minute.length() < 2)
    {
        minute = "0" + minute;
    }

    std::string second = base::IntToString(exploded.second);
    if (second.length() < 2)
    {
        second = "0" + second;
    }

    //std::string millisecond = base::IntToString(exploded.millisecond);
    std::string timestring = hour + ":" + minute + ":" + second;

    return std::move(timestring);
}

std::string MakeFormatDateString(const base::Time time)
{
    base::Time::Exploded exploded;
    time.LocalExplode(&exploded);
    std::string year = base::IntToString(exploded.year);
    std::string month = base::IntToString(exploded.month);
    if (month.length() < 2)
    {
        month = "0" + month;
    }

    std::string day = base::IntToString(exploded.day_of_month);
    if (day.length() < 2)
    {
        day = "0" + day;
    }

    //std::string millisecond = base::IntToString(exploded.millisecond);
    std::string datestring = year + "_" + month + "_" + day;

    return std::move(datestring);
}

// 获取13位的当前时间字符串
std::string GetNowTimeString()
{
    return base::Uint64ToString(
        static_cast<uint64>(base::Time::Now().ToDoubleT() * 1000));
}

std::vector<std::string> SplitString(std::string str, const std::string& pattern)
{
    std::vector<std::string> result;
    str += pattern;//扩展字符串以方便操作
    int size = str.size();

    for (int i = 0; i < size; i++)
    {
        int pos = str.find(pattern, i);
        if (pos < size)
        {
            std::string s = str.substr(i, pos - i);
            if (!s.empty())
                result.push_back(s);
            
            i = pos + pattern.size() - 1;
        }
    }
    return result;
}

std::string PickJson(const std::string& rawresponse)
{
    auto beginpos = rawresponse.find("(");
    auto endpos = rawresponse.rfind(")");
    if (beginpos == std::string::npos || endpos == std::string::npos)
    {
        return rawresponse;
    }

    std::string responsedata = 
        rawresponse.substr(beginpos + 1, endpos - beginpos - 1);
    return responsedata;
}

void RemoveSpace(std::string* str)
{
    auto pos = str->find(' ');
    while (pos != std::string::npos)
    {
        str->erase(pos, 1);
        pos = str->find(' ');
    }
}

uint32 GetInt32FromJsonValue(const Json::Value& jvalue, const std::string& name)
{
	uint32 ret = 0;
	Json::Value jvdefault(Json::ValueType::objectValue);
	auto getdata = jvalue.get(name, jvdefault);
	if (getdata.isInt())
	{
		ret = getdata.asInt();
	}
	else if (getdata.isString())
	{
		base::StringToUint(getdata.asString(), &ret);
	}

	return ret;
}

double GetDoubleFromJsonValue(const Json::Value& jvalue, const std::string& name)
{
	double ret = 0;
	Json::Value jvdefault(Json::ValueType::objectValue);
	auto getdata = jvalue.get(name, jvdefault);
	if (getdata.isDouble())
	{
		ret = getdata.asDouble();
	}
	else if (getdata.isInt())
	{
		ret = getdata.asDouble();
	}
	else if (getdata.isString())
	{
		base::StringToDouble(getdata.asString(), &ret);
	}

	return ret;
}

std::string RSADecryptString(std::istream* privFilename, const std::string& ciphertext)
{
    std::string seed = CryptoPP::IntToString(time(NULL));
    seed.resize(16);
    s_globalRNG.SetKeyWithIV((byte *)seed.data(), 16, (byte *)seed.data());

    using namespace CryptoPP;
    FileSource privFile(*privFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Decryptor priv(privFile);

    std::string decrypted;
    std::string part = ciphertext;
    auto pos = 0;
    uint32 maxcipher = 256;
    while (part.length() > maxcipher)
    {
        std::string temp = ciphertext.substr(pos, maxcipher);
        std::string result;
        StringSource(temp.c_str(), true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
        decrypted += result;
        pos += maxcipher;
        part = ciphertext.substr(pos);
    }

    if (!part.empty())
    {
        std::string result;
        StringSource(part.c_str(), true, new HexDecoder(new PK_DecryptorFilter(GlobalRNG(), priv, new StringSink(result))));
        decrypted += result;
    }

    return decrypted;
}

std::string RSAEncryptString(std::istream* pubFilename, const std::string& message)
{
    std::string seed = CryptoPP::IntToString(time(NULL));
    seed.resize(16);
    s_globalRNG.SetKeyWithIV((byte *)seed.data(), 16, (byte *)seed.data());


    using namespace CryptoPP;
    FileSource pubFile(*pubFilename, true, new HexDecoder);
    RSAES_OAEP_SHA_Encryptor pub(pubFile);

    RandomPool randPool;
    randPool.IncorporateEntropy((byte *)seed.c_str(), seed.length());

    std::string ciphertext;
    std::string part = message;
    auto pos = 0;
    uint32 maxplain = 86;
    while (part.length() > maxplain)
    {
        std::string temp = message.substr(pos, maxplain);
        std::string result;
        StringSource(temp, true, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(result))));
        ciphertext += result;
        pos += maxplain;
        part = message.substr(pos);
    }

    if (!part.empty())
    {
        std::string result;
        StringSource(part, true, new PK_EncryptorFilter(randPool, pub, new HexEncoder(new StringSink(result))));
        ciphertext += result;
    }
    return ciphertext;
}
