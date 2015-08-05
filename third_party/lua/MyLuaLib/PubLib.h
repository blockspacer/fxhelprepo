#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#ifndef __PUBLIB_H__
#define __PUBLIB_H__

//#define IS_UNC(path)  ((path[0] == _T('\\')) && (path[1] == _T('\\')))
//#define INVALID_HANDLE(X)   (INVALID_HANDLE_VALUE == X || NULL == X)
//#define VALID_HANDLE(X)     (INVALID_HANDLE_VALUE != X && NULL != X)

extern FILE* fl_Logout;

#define ERR(x)  _CCprintf(\
    _T("ERROR in %s at line %d: %s; error == 0x%08X\n"), \
    _T(__FILE__), __LINE__, _T(x), GetLastError() )
#define ERR2(x)  _CCprintf(\
    _T("ERROR in %s at line %d: %s; error == 0x%08X\n"), \
    _T(__FILE__), __LINE__, x, GetLastError() )

LPTSTR FormatValue(__int64 dwValue, LPTSTR szValue);
int _CCprintf(const PTCHAR str, ... );

#endif