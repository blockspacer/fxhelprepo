#ifndef _ID3LIB_EXT_H_
#define _ID3LIB_EXT_H_

#include <string>

class ID3LibExt
{
public:
    static bool GetMetadata(const std::wstring& fileName, std::wstring* title,
                            std::wstring* artist, std::wstring* album);

private:
    ID3LibExt();
};

#endif