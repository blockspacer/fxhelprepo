#include "id3lib_ext.h"

#include "third_party/id3lib/config.h"
#include "third_party/id3lib/include/id3/misc_support.h"
#include "third_party/id3lib/include/id3/tag.h"
#include "kugou/base/unicode.h"

using std::string;
using std::wstring;

bool ID3LibExt::GetMetadata(const wstring& fileName, wstring* title,
                            wstring* artist, wstring* album)
{
    if (!title || !artist || !album)
        return false;

    *title = L"";
    *artist = L"";
    *album = L"";

    auto localFileName = kugou::WideToLocal(fileName);
    try
    {
        ID3_Tag id3Root;
        id3Root.Link(localFileName.c_str(), ID3TT_ALL);
        auto iter = id3Root.CreateIterator();
        for (auto frame = iter->GetNext(); frame; frame = iter->GetNext())
        {
            auto id = frame->GetID();
            switch (id)
            {
                case ID3FID_ALBUM:
                {
                    const char* s = ID3_GetString(frame, ID3FN_TEXT);
                    if (s)
                        *album = kugou::LocalToWide(s);

                    break;
                }
                case ID3FID_TITLE:
                {
                    const char* s = ID3_GetString(frame, ID3FN_TEXT);
                    if (s)
                        *title = kugou::LocalToWide(s);

                    break;
                }

                // All artist-related share the same procedure.
                case ID3FID_ORIGARTIST:
                case ID3FID_WWWARTIST:
                case ID3FID_LEADARTIST:
                {
                    const char* s = ID3_GetString(frame, ID3FN_TEXT);
                    if (s && artist->empty())
                        *artist = kugou::LocalToWide(s);

                    break;
                }
            }
        }
        return true;
    }
    catch (...)
    {
        return false;
    }
}