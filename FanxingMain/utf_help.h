//#include <windows.h>
#include <assert.h>
char ReadByte(char ** src)
{
    char b = **src++;
    return b;
}
unsigned short Read2Byte(char ** src)
{
    unsigned short *sp = (unsigned short*)(*src);
    *src += 2;
    return *sp;
}
unsigned int ANSI2USC(char ** src)
{
    char b1 = ReadByte(src);
    if (b1 <= 0x7F)
    {
        return b1;
    }

    //最多只有两个字节表示
    char b2 = ReadByte(src);
    return b2 << 2 | b1;
}
unsigned int UTF162UCS(char ** src)
{
    // UTF16使用1-2个16bit数据表示UCS
    // 判断是使用1个16bit还是2个16bit;
    unsigned short lead = Read2Byte(src);
    if (lead >= 0xD800 && lead <= 0xDFFF)// 2个16bit表示数据
    {
        unsigned short trail = Read2Byte(src);
        if (trail >= 0xDC00 && trail <= 0xDFFF)
        {
            lead -= 0xD800;
            trail -= 0xDC00;
            return (lead << 16) | (trail);
        }
        else
        {
            // 不在0xDC00-0xDFFF范围内的后尾代理被看作是不合法的数据
            // 如何处理参考RFC再决定
            assert(0);
            return 0;
        }

    }
    else// 1个16bit表示数据,
    {
        // 不需要过多的转换,直接就是UCS了
        return lead;
    }
}
unsigned int UTF82UCS(char ** src)
{
    char b1 = ReadByte(src);
    int ucs = 0;
    if (b1 >> 7 == 0x0)//0000 0000-0000 007F | 0xxxxxxx
    {
        return b1;
    }
    else if (b1 >> 5 == 0x6)//0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    {
        char b2 = ReadByte(src);
        if (b2 >> 6 != 0x2)// 第二字节不是10xxxxxx形式则认为是编码错误
        {
            return 0;
        }
        return b1 << 4 | b2;
    }
    else if (b1 >> 4 == 0xE)//0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    {
        char b2 = ReadByte(src);
        if (b2 >> 6 != 0x2)// 第二字节不是10xxxxxx形式则认为是编码错误
        {
            return 0;
        }
        char b3 = ReadByte(src);
        if (b3 >> 6 != 0x2)// 第三字节不是10xxxxxx形式则认为是编码错误
        {
            return 0;
        }
        return b1 << 8 | b2 << 4 | b3;
    }
    else if (b1 >> 3 == 0x1E)//0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    {
        char b2 = ReadByte(src);
        if (b2 >> 6 != 0x2)// 第二字节不是10xxxxxx形式则认为是编码错误
        {
            return 0;
        }
        char b3 = ReadByte(src);
        if (b3 >> 6 != 0x2)// 第三字节不是10xxxxxx形式则认为是编码错误
        {
            return 0;
        }
        char b4 = ReadByte(src);
        if (b4 >> 6 != 0x2)// 第四字节不是10xxxxxx形式则认为是编码错误
        {
            return 0;
        }
        return b1 << 12 | b2 << 8 | b3 << 4 | b4;
    }
    else
    {
        assert(0);
    }

    return 0;
}

// rfc 3629 
//Char. number range | UTF-8 octet sequence
//(hexadecimal) | (binary)
//--------------------+---------------------------------------------
//0000 0000-0000 007F | 0xxxxxxx
//0000 0080-0000 07FF | 110xxxxx 10xxxxxx
//0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
//0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx

// encode format for new extend data
// 0020 0000-03FF FFFF | 111110xx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
// 0400 0000-7FFF FFFF | 1111110x 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx 10xxxxxx
int UCS2UTF8(unsigned int ucs, char * dest)
{
    char *p = dest;
    // 检测ucs数值, 使用UTF8编码规则表示数据
    if (ucs <= 0x7f)
    {
        if (!p)return 1;
        *p++ = ucs & 0x7f;
    }
    else if (ucs <= 0x7ff)
    {
        if (!p)return 2;
        *p++ = 0xC0 | (ucs >> 6);
        *p++ = 0x08 | (ucs & 0x3f);
    }
    else if (ucs <= 0xFFFF)
    {
        if (!p)return 3;
        *p++ = 0xE0 | (ucs >> 12);
        *p++ = 0x80 | ((ucs >> 6) & 0x3f);
        *p++ = 0x80 | (ucs & 0x3f);
    }
    else if (ucs <= 0x10FFFF)
    {
        if (!p)return 4;
        *p++ = 0xF0 | (ucs >> 18);
        *p++ = 0x80 | ((ucs >> 12) & 0x3f);
        *p++ = 0x80 | ((ucs >> 6) & 0x3f);
        *p++ = 0x80 | (ucs & 0x3f);
    }
    else if (ucs <= 0x3ffffff)//这些值不在RFC3629里定义,但是我觉得也应该处理
    {
        if (!p)return 5;
        *p++ = 0xF8 | (ucs >> 24);
        *p++ = 0x80 | ((ucs >> 18) & 0x3f);
        *p++ = 0x80 | ((ucs >> 12) & 0x3f);
        *p++ = 0x80 | ((ucs >> 6) & 0x3f);
        *p++ = 0x80 | (ucs & 0x3f);
    }
    else if (ucs <= 0x7fffffff)//这些值不在RFC3629里定义,但是我觉得也应该处理
    {
        if (!p)return 6;
        *p++ = 0xF8 | (ucs >> 30);
        *p++ = 0x80 | ((ucs >> 24) & 0x3f);
        *p++ = 0x80 | ((ucs >> 18) & 0x3f);
        *p++ = 0x80 | ((ucs >> 12) & 0x3f);
        *p++ = 0x80 | ((ucs >> 6) & 0x3f);
        *p++ = 0x80 | (ucs & 0x3f);
    }
    else//不在0-0x7fffffff范围内的数值,认为不是合法的ucs
    {
        // 默认处理为0还是忽略呢？参考RFC再作决定
        assert(0);
    }
    return p - dest;
}

int UCS2UTF16(unsigned int ucs, char * dest)
{
    char *p = dest;
    if (ucs <= 0xFFFF)
    {
        if (!p)return 2;
        *p++ = ucs;
        *p++ = 0;
    }
    else if (ucs <= 0x10FFFF)
    {
        if (!p)return 4;
        ucs -= 0x10000;
        unsigned short us = (ucs & 0x3FF) | 0xDC00;//low surrogate
        *p++ = us >> 8;
        *p++ = us & 0xFF;
        us = (ucs >> 10) | 0xD800;//high surrogate
        *p++ = us >> 8;
        *p++ = us & 0xFF;
    }
    else // RFC 2781 UTF16的说明没有描述此范围的处理,我认为应该需要处理
    {
        assert(0);
    }
    return p - dest;
}

int UTF162UTF8(char* src, char * dest)
{
    //read utf16
    int ucs = 0;
    char * utf16 = src;
    if (!dest)
    {
        // 若传入的dest为空,则只为测试变成UTF8后的长度以方便分配内存
        int count = 0;
        while (ucs = UTF162UCS(&utf16))
        {
            count += UCS2UTF8(ucs, 0);
        }
        return count + 1;
    }

    char * utf8 = dest;
    while (ucs = UTF162UCS(&utf16))
    {
        utf8 += UCS2UTF8(ucs, utf8);
    }
    *utf8++ = 0;
    return utf8 - dest;
}

int UTF82UTF16(char* src, char * dest)
{
    //read utf8
    int ucs = 0;
    char * utf8 = src;
    if (!dest)
    {
        // 若传入的dest为空,则只为测试变成UTF8后的长度以方便分配内存
        int count = 0;
        while (ucs = UTF82UCS(&utf8))
        {
            count += UCS2UTF16(ucs, 0);
        }
        return count + 2;
    }

    char * utf16 = dest;
    while (ucs = UTF82UCS(&utf8))
    {
        utf8 += UCS2UTF16(ucs, utf16);
    }
    *utf16++ = 0;
    *utf16++ = 0;
    return utf16 - dest;
}

// 对于需要两个字节的ANSI字符的编译时处理，编译器一般会把其变成两个字节存储；
// 对于需要两个字节的ANSI字符的运行时输入，要看输入端程序的处理方式；可测试scanf
// 对于需要超过两个字节以上表示的编码文字，使用ANSI无法表示，但是编译器会安排成0x3f3f这样的形式
int ANSI2UTF8(char * src, char * dest)
{
    int ucs = 0;
    char * ansi = src;
    if (!dest)
    {
        int count = 0;
        while (ucs = ANSI2USC(&ansi))
        {
            count += UCS2UTF8(ucs, 0);
        }
        return count + 1;
    }

    char * utf8 = dest;
    while (ucs = ANSI2USC(&ansi))
    {
        ansi += UCS2UTF8(ucs, utf8);
    }
    *utf8++ = 0;
    return utf8 - dest;
}

int ANSI2UTF16(char * src, char * dest)
{
    int ucs = 0;
    char * ansi = src;
    if (!dest)
    {
        int count = 0;
        while (ucs = ANSI2USC(&ansi))
        {
            count += UCS2UTF16(ucs, 0);
        }
        return count + 2;
    }

    char * utf16 = dest;
    while (ucs = ANSI2USC(&ansi))
    {
        ansi += UCS2UTF16(ucs, utf16);
    }
    *utf16++ = 0;
    *utf16++ = 0;
    return utf16 - dest;
}