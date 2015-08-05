// MediaInfo - All info about media files
// Copyright (C) 2002-2011 MediaArea.net SARL, Info@MediaArea.net
//
// This library is free software: you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this library. If not, see <http://www.gnu.org/licenses/>.
//
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

//---------------------------------------------------------------------------
// Pre-compilation
#include "MediaInfo/PreComp.h"
#ifdef __BORLANDC__
    #pragma hdrstop
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/Setup.h"
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#include "MediaInfo/MediaInfo.h"
#include "MediaInfo/MediaInfo_Internal.h"
#include "MediaInfo_Const.h"
#include <vector>
#include <Windows.h>
#include <Shlwapi.h>
using namespace ZenLib;
//---------------------------------------------------------------------------

    enum aType {
      /*!
       * IS08859-1, or <i>Latin1</i> encoding.  8 bit characters.
       */
      Latin1 = 0,
      /*!
       * UTF16 with a <i>byte order mark</i>.  16 bit characters.
       */
      UTF16 = 1,
      /*!
       * UTF16 <i>big endian</i>.  16 bit characters.  This is the encoding used
       * internally by TagLib.
       */
      UTF16BE = 2,
      /*!
       * UTF8 encoding.  Characters are usually 8 bits but can be up to 32.
       */
      UTF8 = 3,
      /*!
       * UTF16 <i>little endian</i>.  16 bit characters.
       */
      UTF16LE = 4
    };

	std::vector<char> condata( const MediaInfoLib::String& SrcStr, aType t)
{
	std::vector<char> v;

	switch(t)
	{
	case Latin1:
		{
			for(wstring::const_iterator it = SrcStr.begin(); it != SrcStr.end(); it++)
				v.push_back(char(*it));
			break;
		}
	}
	return v;
}
wstring toWString( const MediaInfoLib::String& srcStr )
{
   //return d->data;
	bool bIsMbs = true;
	vector<char> v;

	for(wstring::const_iterator it = srcStr.begin(); it != srcStr.end(); ++it) 
	{
		if( *it & 0xFF00 )
		{
			bIsMbs = false;
			break;
		}
	}

	std::wstring s;
	wchar_t *pw = NULL;
	if( bIsMbs )
	{
		v = condata( srcStr, Latin1 );
		v.push_back( char(0x00) );

		pw = new wchar_t[v.size()];
		MultiByteToWideChar( CP_ACP, 0, (LPCSTR)&v[0], v.size(), pw, v.size() );
	}
	else
	{
		size_t nLen = (srcStr.size() + 1) * sizeof(wchar_t);
		pw = new wchar_t[srcStr.size() + 1];
		memset( pw, 0, nLen );
		memcpy( pw, srcStr.c_str(), nLen );
	}

	StrTrimW( pw, L" " );
	if( NULL != pw )
	{
		s = pw;
	}
	delete []pw;

	return s;
}

namespace MediaInfoLib
{

//---------------------------------------------------------------------------
//To clarify the code
namespace MediaInfo_Debug_MediaInfo
{

#if defined (MEDIAINFO_DEBUG_CONFIG) || defined (MEDIAINFO_DEBUG_BUFFER) || defined (MEDIAINFO_DEBUG_OUTPUT)
    #ifdef WINDOWS
        const Char* MediaInfo_Debug_Name=_T("MediaInfo_Debug");
    #else
        const Char* MediaInfo_Debug_Name=_T("/tmp/MediaInfo_Debug");
    #endif
#endif

#ifdef MEDIAINFO_DEBUG_CONFIG
    #define MEDIAINFO_DEBUG_STATIC(_TOAPPEND) \
        { \
            File F(Ztring(MediaInfo_Debug_Name)+_T(".Config.static.txt"), File::Access_Write_Append); \
            Ztring Debug; \
            _TOAPPEND; \
            Debug+=_T("\r\n"); \
            F.Write(Debug); \
            F.Close(); \
        }
#else // MEDIAINFO_DEBUG_CONFIG
    #define MEDIAINFO_DEBUG_STATIC(_TOAPPEND)
#endif // MEDIAINFO_DEBUG_CONFIG

}
using namespace MediaInfo_Debug_MediaInfo;

//***************************************************************************
// Constructor/destructor
//***************************************************************************

//---------------------------------------------------------------------------
MediaInfo::MediaInfo()
{
    Internal=new MediaInfo_Internal();
}

//---------------------------------------------------------------------------
MediaInfo::~MediaInfo()
{
    delete (MediaInfo_Internal*)Internal; //Internal=NULL;
}

//***************************************************************************
// Files
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfo::Open(const String &File_Name_)
{
    return ((MediaInfo_Internal*)Internal)->Open(File_Name_);
}

//---------------------------------------------------------------------------
size_t MediaInfo::Open (const int8u* Begin_, size_t Begin_Size_, const int8u*, size_t, int64u)
{
    return ((MediaInfo_Internal*)Internal)->Open(Begin_, Begin_Size_);
}

//---------------------------------------------------------------------------
size_t MediaInfo::Open_Buffer_Init (int64u File_Size, int64u File_Offset)
{
    return ((MediaInfo_Internal*)Internal)->Open_Buffer_Init(File_Size, File_Offset);
}

//---------------------------------------------------------------------------
size_t MediaInfo::Open_Buffer_Continue (const int8u* ToAdd, size_t ToAdd_Size)
{
    return ((MediaInfo_Internal*)Internal)->Open_Buffer_Continue(ToAdd, ToAdd_Size).to_ulong();
}

//---------------------------------------------------------------------------
int64u MediaInfo::Open_Buffer_Continue_GoTo_Get ()
{
    return ((MediaInfo_Internal*)Internal)->Open_Buffer_Continue_GoTo_Get();
}

//---------------------------------------------------------------------------
size_t MediaInfo::Open_Buffer_Finalize ()
{
    return ((MediaInfo_Internal*)Internal)->Open_Buffer_Finalize();
}

//---------------------------------------------------------------------------
size_t MediaInfo::Open_NextPacket ()
{
    return ((MediaInfo_Internal*)Internal)->Open_NextPacket().to_ulong();;
}

//---------------------------------------------------------------------------
size_t MediaInfo::Save()
{
    return 0; //Not yet implemented
}

//---------------------------------------------------------------------------
void MediaInfo::Close()
{
    return ((MediaInfo_Internal*)Internal)->Close();
}

//***************************************************************************
// Get File info
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfo::Inform(size_t)
{
    return ((MediaInfo_Internal*)Internal)->Inform();
}

//---------------------------------------------------------------------------
String MediaInfo::Get(stream_t StreamKind, size_t StreamPos, size_t Parameter, info_t KindOfInfo)
{
    return ((MediaInfo_Internal*)Internal)->Get(StreamKind, StreamPos, Parameter, KindOfInfo);
}

//---------------------------------------------------------------------------
String MediaInfo::Get(stream_t StreamKind, size_t StreamPos, const String &Parameter, info_t KindOfInfo, info_t KindOfSearch)
{
	String tmpstr = ((MediaInfo_Internal*)Internal)->Get(StreamKind, StreamPos, Parameter, KindOfInfo, KindOfSearch);;
	if ( tmpstr.empty() )
		return tmpstr;
	tmpstr = toWString( tmpstr );
    return tmpstr;
}

//***************************************************************************
// Set File info
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfo::Set(const String &, stream_t, size_t, size_t, const String &)
{
    return 0; //Not yet implemented
}

//---------------------------------------------------------------------------
size_t MediaInfo::Set(const String &, stream_t, size_t, const String &, const String &)
{
    return 0; //Not yet implemented
}

//***************************************************************************
// Output buffer
//***************************************************************************

//---------------------------------------------------------------------------
size_t MediaInfo::Output_Buffer_Get (const String &Value)
{
    return ((MediaInfo_Internal*)Internal)->Output_Buffer_Get(Value);
}

//---------------------------------------------------------------------------
size_t MediaInfo::Output_Buffer_Get (size_t Pos)
{
    return ((MediaInfo_Internal*)Internal)->Output_Buffer_Get(Pos);
}

//***************************************************************************
// Information
//***************************************************************************

//---------------------------------------------------------------------------
String MediaInfo::Option (const String &Option, const String &Value)
{
    return ((MediaInfo_Internal*)Internal)->Option(Option, Value);
}

//---------------------------------------------------------------------------
String MediaInfo::Option_Static (const String &Option, const String &Value)
{
    MEDIAINFO_DEBUG_STATIC(Debug+=_T("Option_Static, Option=");Debug+=Ztring(Option);Debug+=_T(", Value=");Debug+=Ztring(Value);)
    MediaInfoLib::Config.Init(); //Initialize Configuration

         if (Option==_T("Info_Capacities"))
    {
        return _T("Option disactivated for this version, will come back soon!");
        //MediaInfo_Internal MI;
        //return MI.Option(Option);
    }
    else if (Option==_T("Info_Version"))
    {
        Ztring ToReturn=MediaInfoLib::Config.Info_Version_Get();
        if (MediaInfo_Internal::LibraryIsModified())
            ToReturn+=_T(" modified");
        return ToReturn;
    }
    else
        return MediaInfoLib::Config.Option(Option, Value);
}

//---------------------------------------------------------------------------
size_t MediaInfo::Count_Get (stream_t StreamKind, size_t StreamPos)
{
    return ((MediaInfo_Internal*)Internal)->Count_Get(StreamKind, StreamPos);

}

//---------------------------------------------------------------------------
size_t MediaInfo::State_Get ()
{
    return ((MediaInfo_Internal*)Internal)->State_Get();
}

} //NameSpace


