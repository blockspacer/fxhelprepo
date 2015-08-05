#include "PubLib.h"

#define PubLib_c        //遵循LUA库写法，无实际意义
#define LUA_LIB         //遵循LUA库写法，无实际意义

#include "../lua5.2.2_Src/lua.h"
#include "../lua5.2.2_Src/lauxlib.h"
#include "../lua5.2.2_Src/lualib.h"

#define D_BUG 0

FILE* fl_Logout = NULL;

int Add(int a ,int b)
{
	return a + b;
}

int _CCprintf( const PTCHAR str, ... )
{
    TCHAR buffer[2048];
    va_list ArgList ;

    va_start (ArgList, str) ;
    wvsprintf(buffer, str, ArgList);

    _tprintf(TEXT("%s"), buffer);

    if (fl_Logout != NULL)
    {
        //fl_Logout = fopen("TestLog.log", "w");
        _ftprintf(fl_Logout, TEXT("%s"), buffer);
        fflush(fl_Logout); 
    }
    va_end (ArgList) ;
    return (0);
}

__int64 PubLib_GetDiskFreeSpace(PTCHAR szPath)
{

    ULARGE_INTEGER uliFreeToCaller, uliTotal, uliTotalFree;

    if(!GetDiskFreeSpaceEx(szPath,
        &uliFreeToCaller, &uliTotal, &uliTotalFree))
    {
        ERR("GetDiskFreeSpaceEx()");
        _CCprintf( _T("Error: GetDiskFreeSpaceEx: %s\n"), szPath);

        return 0;
    }
    return uliFreeToCaller.QuadPart;
}

#define DEFAULT_READ_WRITE_LEN (32 * 1024)
#define isString(L, x) luaL_checktype(L, x, LUA_TSTRING);
#define isNumber(L, x) luaL_checktype(L, x, LUA_TNUMBER);
#define isTable(L, x) luaL_checktype(L, x, LUA_TTABLE);
#define isBoolean(L, x) luaL_checktype(L, x, LUA_TBOOLEAN);

void error (lua_State *L, const char *fmt, ...) { 
    va_list argp; 
    va_start(argp, fmt); 
    vfprintf(fl_Logout,fmt, argp);
    va_end(argp); 
    lua_close(L); 
    exit(EXIT_FAILURE); 
}

PTCHAR InitStringArg(lua_State *L, int idx)
{
    PTCHAR szWcsArg = NULL;

    PCCH szMbsArg = lua_tostring(L,idx);
    
    UINT32 len = strlen(szMbsArg);
    szWcsArg = (PTCHAR)malloc((len+1) * 2);
    mbstowcs(szWcsArg, szMbsArg, len+1);

    return szWcsArg;
}

VOID FreeStringArg(PTCHAR *szWcsArg)
{
    if (*szWcsArg != NULL)
    {
        free(*szWcsArg);
        szWcsArg = NULL;
    }
}


VOID PushStringRst(lua_State *L, PTCHAR rst)
{
	PCHAR szMbsRst = NULL;
    UINT32 len = _tcslen(rst);

    szMbsRst = (PCHAR)malloc(len+1);
    wcstombs(szMbsRst, rst, len+1);

    lua_pushstring(L, szMbsRst);
    free(szMbsRst);
}

static void stackDump (lua_State *L) {
    int i;
    int top = lua_gettop(L);
    for (i = 1; i <= top; i++) {  /* repeat for each level */
        int t = lua_type(L, i);
        switch (t) {

   case LUA_TSTRING:  /* strings */
       printf("`S:%s'", lua_tostring(L, i));
       break;

   case LUA_TBOOLEAN:  /* booleans */
       printf(lua_toboolean(L, i) ? "true" : "false");
       break;

   case LUA_TNUMBER:  /* numbers */
       printf("N:%g", lua_tonumber(L, i));
       break;

   default:  /* other values */
       printf("%s", lua_typename(L, t));
       break;
		}
		printf("  ");  /* put a separator */
	}
	printf("\n");   /* end the listing */
}

LPTSTR FormatValue(__int64 dwValue, LPTSTR szValue) {

    // Loop on each thousand's group.
    DWORD dw = 0, dwGroup[4] = { 0, 0, 0, 0 };
    while (dwValue) {
        dwGroup[dw++] = dwValue % 1000;
        dwValue /= 1000;
    }

    // Format the output with commas.
    switch (dw) {
      case 2:  
          wsprintf(szValue, TEXT("%u,%03u"), dwGroup[1], dwGroup[0]); 
          break;
      case 3:  
          wsprintf(szValue, TEXT("%u,%03u,%03u"), dwGroup[2], dwGroup[1], dwGroup[0]); 
          break;
      case 4:  
          wsprintf(szValue, TEXT("%u,%03u,%03u,%03u"), dwGroup[3], dwGroup[2], dwGroup[1], dwGroup[0]); 
          break;
      default: 
          wsprintf(szValue, TEXT("%u"), dwGroup[0]);
          break;
    }

    return szValue;
}


/*********************************以下是PubLib库接口*********************************/
static int l_Date(lua_State *L)
{
    SYSTEMTIME systime;
    GetLocalTime( &systime );
	
	_CCprintf( TEXT("\n"));
#if D_BUG
	_CCprintf( TEXT("Call: l_Date()\n"));
	_CCprintf( TEXT("Today is: %d-%d-%d,"), systime.wYear, systime.wMonth, systime.wDay);
	_CCprintf( TEXT("Now Time: %d:%d:%d:%d\n"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
#endif
	_CCprintf( TEXT("The System Time is: %d-%d-%d  %d:%d:%d:%d\n"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	/*
	_CCprintf( TEXT("\n"));
	_CCprintf( TEXT("这里调用了l_Date()\n"));
	_CCprintf( TEXT("今天是%d年%d月%d日,"), systime.wYear, systime.wMonth, systime.wDay);
    _CCprintf( TEXT("星期%d,"), systime.wDayOfWeek);
	_CCprintf( TEXT("当前时间%d:%d:%d:%d\n"), systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	_CCprintf( TEXT("系统当前时间:%d-%d-%d  %d:%d:%d:%d\n"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond, systime.wMilliseconds);
	*/
	return 0;   
}

static int l_GetSysMemInfo(lua_State *L)
{
    TCHAR szValue[32];
    MEMORYSTATUS MM;
    GlobalMemoryStatus(&MM);

#if D_BUG
    _CCprintf( TEXT("\n"));
	_CCprintf( TEXT("Call: l_GetSysMemInfo()\n"));
    _CCprintf( TEXT("System Memory Statu Information:\n"));
#endif

	/*
    _CCprintf( TEXT("  Length:              %12s\t结构长度\n"), FormatValue(MM.dwLength, szValue));
    _CCprintf( TEXT("  MemoryLoad:          %12s\t内存占用率\n"), FormatValue(MM.dwMemoryLoad, szValue));
    _CCprintf( TEXT("  TotalPhys:         %12sMB\t总物理内存\n"), FormatValue(MM.dwTotalPhys/(1024*1024), szValue));
    _CCprintf( TEXT("  AvailPhys          %12sMB\t可用物理内存\n"), FormatValue(MM.dwAvailPhys/(1024*1024), szValue));
    _CCprintf( TEXT("  TotalPageFile:     %12sMB\t总交换文件空间\n"), FormatValue(MM.dwTotalPageFile/(1024*1024), szValue));
    _CCprintf( TEXT("  AvailPageFile:     %12sMB\t空闲交换文件空间\n"), FormatValue(MM.dwAvailPageFile/(1024*1024), szValue));
    _CCprintf( TEXT("  TotalVirtual:      %12sMB\t用户可用的地址空间\n"), FormatValue(MM.dwTotalVirtual/(1024*1024), szValue));
    _CCprintf( TEXT("  AvailVirtual:      %12sMB\t当前空闲的地址空间\n"), FormatValue(MM.dwAvailVirtual/(1024*1024), szValue));
	*/
	_CCprintf( TEXT("  Length:              %12s\n"), FormatValue(MM.dwLength, szValue));
    _CCprintf( TEXT("  MemoryLoad:          %12s\n"), FormatValue(MM.dwMemoryLoad, szValue));
    _CCprintf( TEXT("  TotalPhys:         %12sMB\n"), FormatValue(MM.dwTotalPhys/(1024*1024), szValue));
    _CCprintf( TEXT("  AvailPhys          %12sMB\n"), FormatValue(MM.dwAvailPhys/(1024*1024), szValue));
    _CCprintf( TEXT("  TotalPageFile:     %12sMB\n"), FormatValue(MM.dwTotalPageFile/(1024*1024), szValue));
    _CCprintf( TEXT("  AvailPageFile:     %12sMB\n"), FormatValue(MM.dwAvailPageFile/(1024*1024), szValue));
    _CCprintf( TEXT("  TotalVirtual:      %12sMB\n"), FormatValue(MM.dwTotalVirtual/(1024*1024), szValue));
    _CCprintf( TEXT("  AvailVirtual:      %12sMB\n"), FormatValue(MM.dwAvailVirtual/(1024*1024), szValue));

    _CCprintf( TEXT("\n"));
    return 0;
}

static int l_Sleep (lua_State *L)
{
    enum ARG
    {
		ARG_Time=1, 
    };

    DWORD wTime;
    isNumber(L, ARG_Time);
    wTime = lua_tonumber(L, ARG_Time);
	_CCprintf( TEXT("\n"));

#if D_BUG
	_CCprintf( TEXT("Call: l_Sleep()\n"));
#endif

    _CCprintf( TEXT("Sleep %d \n"), wTime);
    Sleep(wTime);
    return 0;
}

static int l_GetTickCount(lua_State *L)
{
#if D_BUG
	_CCprintf( TEXT("\n"));
    _CCprintf( TEXT("Call: l_GetTickCount\n"));
#endif

    lua_pushnumber(L, GetTickCount());
    return 1;
}

static int l_LogOut(lua_State *L)
{
    enum ARG
    {
        ARG_Str = 1,    //需要输出的string   参数类型:String
    };

    __int64 i64ReturnValue = 0;
    PTCHAR Str;
    isString(L, ARG_Str);
    Str = InitStringArg(L, ARG_Str);
	
	//_CCprintf( TEXT("\n"));
	//_CCprintf( TEXT("Call: l_LogOut\n"));
    _CCprintf( L"%s\n", Str);
	//_CCprintf( TEXT("%s\n"), Str);  //add 2031-8-5:输出到文件

    FreeStringArg(&Str);
    return 0;
}

static int l_ErrOut(lua_State *L)
{
    enum ARG
    {
        ARG_Str = 1,    //需要输出的string   参数类型:String
    };

    __int64 i64ReturnValue = 0;
    PTCHAR Str;
    isString(L, ARG_Str);
    Str = InitStringArg(L, ARG_Str);
	
	//_CCprintf( TEXT("\n"));
	//_CCprintf( TEXT("Call: l_ErrOut()\n"));
    _CCprintf( L"%s\n", Str);
	////_CCprintf( TEXT("%s\n"), Str);  //add 2031-8-5:输出到文件

    FreeStringArg(&Str);
    return 0;
}

static int l_OpenLogFile(lua_State *L)
{
    enum ARG
    {
        ARG_FileName=1,
    };

    luaL_checktype(L, ARG_FileName, LUA_TSTRING);
    fl_Logout = fopen(lua_tostring(L, ARG_FileName), "w");

#if D_BUG
	_CCprintf( TEXT("\n"));
	_CCprintf( TEXT("Call: l_OpenLogFile()\n"));
#endif

    return 0;
}

static int l_CloseLogFile(lua_State *L)
{

    if (fl_Logout != NULL)
    {
        fclose(fl_Logout);
    }

#if D_BUG
	_CCprintf( TEXT("\n"));
	_CCprintf( TEXT("Call: l_CloseLogFile()\n"));
#endif

    fl_Logout = NULL;
    return 0;
}

static int l_HelloWorld(lua_State* L) 
{ 
     MessageBox(NULL, TEXT("I am an API for LUA Public Lib->HelloWorld()"), TEXT("LUA Information"), MB_OKCANCEL); 

#if D_BUG
	_CCprintf( TEXT("\n"));
	_CCprintf( TEXT("Call: l_HelloWorld()\n"));
#endif

     return 0; 
} 

static int l_AverageSum(lua_State *L) 
{ 
     int n = lua_gettop(L); 
     double Sum = 0; 
     int i; 
      
     for (i = 1; i <= n; i++) 
     { 
         Sum += lua_tonumber(L, i); 
     } 
      
     lua_pushnumber(L, Sum / n);
     lua_pushnumber(L, Sum); 

#if D_BUG
    _CCprintf( TEXT("\n"));
	_CCprintf( TEXT("Call: l_AverageSum()\n"));
#endif

	return 2; 
} 

static int l_TestAdd (lua_State *L)
{
    enum ARG
    {
		ARG_X=1, 
		ARG_Y
    };

    int x,y,Sum;
    isNumber(L, ARG_X);
	isNumber(L, ARG_Y);

    x = lua_tonumber(L, ARG_X);
    y = lua_tonumber(L, ARG_Y);

#if D_BUG
	_CCprintf( TEXT("\n"));
	_CCprintf( TEXT("Call: l_TestAdd()\n"));
#endif

    Sum = Add(x, y);
	lua_pushnumber(L, Sum);

    return 1;
}
//自己封装的LUA库接口数据结构
static const struct luaL_Reg publib [] = {
    {"PrintDate",		    l_Date},
    {"PrintSysMem",			l_GetSysMemInfo},
    {"Sleep",				l_Sleep},
	{"GetTickCount",        l_GetTickCount},
	{"LogOut",              l_LogOut},
    {"ErrOut",			    l_ErrOut},
    {"OpenLogFile",         l_OpenLogFile},
    {"CloseLogFile",        l_CloseLogFile},
	{"HelloWorld",          l_HelloWorld},
	{"AverageSum",          l_AverageSum},
	{"TestAdd",	            l_TestAdd},
    {NULL, NULL}       
};

LUALIB_API int luaopen_publib (lua_State *L) {
    luaL_register(L, LUA_PUBLIBNAME, publib);
    return 1;
}