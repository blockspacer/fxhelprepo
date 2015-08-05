#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#define ltstlib_c
#define LUA_LIB

#include "../lua5.2.2_Src/lua.h"

#include "../lua5.2.2_Src/lauxlib.h"
#include "../lua5.2.2_Src/lualib.h"

#include "legacy.h"

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

static int l_myTest (lua_State *L) {
    
    // 简单的遍历表的功能


    int t = -2;
    stackDump(L);
    lua_pushnil(L);  /* 第一个 key */
    stackDump(L);
    while (lua_next(L, t) != 0)
    {
        /* 'key' (索引-2) 和 'value' (索引-1) */
        DWORD key = 0;
        DWORD value = 0;;
        stackDump(L);
        key = lua_tonumber(L, -2);
        value = lua_tonumber(L, -1);
        
        printf("key=%d, value=%d \n", key, value);

        /* 移除 'value' ；保留 'key' 做下一次迭代 */
        lua_pop(L, 1);
        stackDump(L);
    }
    return 0;  /* no results */
}


static int l_SkipRootPath(lua_State *L)
{
    enum ARG
    {
        ARG_SkipRootPath = 1
    };
    isBoolean(L, ARG_SkipRootPath);
    g_fSkipFirstDir = lua_toboolean(L, ARG_SkipRootPath);

    return 0;
}

static int l_Date(lua_State *L)
{
    SYSTEMTIME systime;
    GetLocalTime( &systime );
    _CCprintf(TEXT("%d-%d-%d, %d:%d:%d\n"), systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);
    return 0;   
}

/*
BOOL Test.CreateFile(
(string) FilePath
(string) FileName,
(table)  buffer,
(int)    write_len,
(int)    file_len,
);

*/
/*
static int l_CreateFile (lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,   //文件所在的路径 参数类型String
        ARG_FileName,       //文件名 参数类型String
        ARG_Buffer,         //需要写入的内容 参数类型Table
        ARG_WriteLen,       //每次写入的长度, 此项为零则使用默认大小 参数类型Int32
        ARG_FileLen,        //文件长度 参数类型Int32
    };
    BOOL        bReturnValue    = TRUE;
    PBYTE       pTestBuffer     = NULL;
    DWORD        dwBlockSize        = 0;
    UINT32        cnt             = 0;
    UINT32        i                = 0;

    PTCHAR    szFilePath        = NULL;
    PTCHAR    szFileName        = NULL;
    DWORD        dwBufferSize    = 0;
    DWORD        dwWriteLen        = 0;
    DWORD        dwFileLen        = 0;

    isString(L, ARG_FilePath);
    isString(L, ARG_FileName);
    isTable(L, ARG_Buffer);
    isNumber(L, ARG_WriteLen);
    isNumber(L, ARG_FileLen);

    szFilePath  = InitStringArg(L, ARG_FilePath);
    szFileName    = InitStringArg(L, ARG_FileName);

    dwBufferSize = luaL_getn(L, ARG_Buffer);
    dwWriteLen    = lua_tonumber(L, ARG_WriteLen);
    dwFileLen    = lua_tonumber(L, ARG_FileLen);

    if (dwWriteLen != 0)
    {
        dwBlockSize = dwWriteLen;
    }
    else
    {
        dwBlockSize = DEFAULT_READ_WRITE_LEN;
    }

    pTestBuffer = (LPBYTE)LocalAlloc(0, dwBlockSize );

    if (pTestBuffer == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    //Initialize test buffer.
    for (cnt=0, i=1; cnt<dwBlockSize; cnt++, i++)
    {
        if(i <= dwBufferSize)
        {
            lua_rawgeti(L, ARG_Buffer, i);
            pTestBuffer[cnt] = lua_tonumber(L,-1);
            lua_pop(L, 1);
        }
        else
        {
            pTestBuffer[cnt] = pTestBuffer[cnt % dwBufferSize];
        }

    }

    bReturnValue = FSDT_CreateFile(szFilePath, szFileName,pTestBuffer, dwBlockSize, dwWriteLen, dwFileLen);

Bail:
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }
    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);

    FreeStringArg(&szFileName);
    return 1;
}
*/
/*
static int l_StressCreateFile (lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,   //文件所在的路径 参数类型String
        ARG_FileName,       //文件名 参数类型String
        ARG_Buffer,         //需要写入的内容 参数类型Table
        ARG_WriteLen,       //每次写入的长度, 此项为零则使用默认大小 参数类型Int32
        ARG_FileLen,        //文件长度 参数类型Int32
    };
    BOOL        bReturnValue    = TRUE;
    PBYTE       pTestBuffer     = NULL;
    DWORD        dwBufferSize        = 0;
    UINT32        cnt             = 0;
    UINT32        i                = 0;

    PTCHAR    szFilePath        = NULL;
    PTCHAR    szFileName        = NULL;
    DWORD        dwFillCnt    = 0;
    DWORD        dwWriteLen        = 0;
    DWORD        dwFileLen        = 0;

    isString(L, ARG_FilePath);
    isString(L, ARG_FileName);
    isTable(L, ARG_Buffer);
    isNumber(L, ARG_WriteLen);
    isNumber(L, ARG_FileLen);

    szFilePath  = InitStringArg(L, ARG_FilePath);
    szFileName    = InitStringArg(L, ARG_FileName);

    dwFillCnt = luaL_getn(L, ARG_Buffer);
    dwWriteLen    = lua_tonumber(L, ARG_WriteLen);
    dwFileLen    = lua_tonumber(L, ARG_FileLen);

    if (dwWriteLen != 0)
    {
        dwBufferSize = dwWriteLen;
    }
    else
    {
        dwBufferSize = DEFAULT_READ_WRITE_LEN;
    }

    pTestBuffer = (LPBYTE)LocalAlloc(0, dwBufferSize );

    if (pTestBuffer == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    //Initialize test buffer.
    for (cnt=0, i=1; cnt<dwBufferSize; cnt++, i++)
    {
        if(i <= dwFillCnt)
        {
            lua_rawgeti(L, ARG_Buffer, i);
            pTestBuffer[cnt] = lua_tonumber(L,-1);
            lua_pop(L, 1);
        }
        else
        {
            pTestBuffer[cnt] = pTestBuffer[cnt % dwFillCnt];
        }
    }
    
    bReturnValue = FSDT_StressCreateFile(szFilePath, szFileName, pTestBuffer, dwBufferSize, dwWriteLen, dwFileLen);

Bail:
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }
    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);

    FreeStringArg(&szFileName);
    return 1;
}

static int l_StressVerifFile (lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,    //文件路径, 类型String
        ARG_FileName,        //文件名,   类型String
        ARG_Buffer,          //内容Buffer, 类型Table
        ARG_ReadLen,         // 每次读写长度,如果为0则使用默认值  类型int
        ARG_FileLen,
    };
    BOOL        bReturnValue    = TRUE;
    PBYTE       pTestBuffer     = NULL;
    DWORD        dwBufferSize        = 0;
    UINT32        cnt             = 0;
    UINT32        i                = 0;

    PTCHAR    szFilePath        = NULL;
    PTCHAR    szFileName        = NULL;
    DWORD        dwFillCnt    = 0;
    DWORD        dwReadLen        = 0;
    DWORD        dwFileLen        = 0;

    isString(L, ARG_FilePath);
    isString(L, ARG_FileName);
    isTable(L, ARG_Buffer);
    isNumber(L, ARG_ReadLen);
    isNumber(L, ARG_ReadLen);
    isNumber(L, ARG_FileLen);


    dwFileLen    = lua_tonumber(L, ARG_FileLen);

    szFilePath  = InitStringArg(L, ARG_FilePath);
    szFileName    = InitStringArg(L, ARG_FileName);

    dwFillCnt = luaL_getn(L, ARG_Buffer);
    dwReadLen    = lua_tonumber(L, ARG_ReadLen);

    if (dwReadLen != 0)
    {
        dwBufferSize = dwReadLen;
    }
    else
    {
        dwBufferSize = DEFAULT_READ_WRITE_LEN;
    }

    pTestBuffer = (LPBYTE)LocalAlloc(0, dwBufferSize );

    if (pTestBuffer == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    //Initialize test buffer.
    for (cnt=0, i=1; cnt<dwBufferSize; cnt++, i++)
    {
        if(i <= dwFillCnt)
        {
            lua_rawgeti(L, ARG_Buffer, i);
            pTestBuffer[cnt] = lua_tonumber(L,-1);
            lua_pop(L, 1);
        }
        else
        {
            pTestBuffer[cnt] = pTestBuffer[cnt % dwFillCnt];
        }

    }
    bReturnValue = FSDT_StressVerifFile(szFilePath, szFileName,pTestBuffer, dwBufferSize, dwFileLen);

Bail:
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }
    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);
    FreeStringArg(&szFileName);
    return 1;
}

static int l_VerifFile (lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,    //文件路径, 类型String
        ARG_FileName,        //文件名,   类型String
        ARG_Buffer,          //内容Buffer, 类型Table
        ARG_ReadLen,         // 每次读写长度,如果为0则使用默认值  类型int
        ARG_FileLen,
    };
    BOOL        bReturnValue    = TRUE;
    PBYTE       pTestBuffer     = NULL;
    DWORD        dwBlockSize        = 0;
    UINT32        cnt             = 0;
    UINT32        i                = 0;

    PTCHAR    szFilePath        = NULL;
    PTCHAR    szFileName        = NULL;
    DWORD        dwBufferSize    = 0;
    DWORD        dwReadLen        = 0;
    DWORD        dwFileLen        = 0;

    isString(L, ARG_FilePath);
    isString(L, ARG_FileName);
    isTable(L, ARG_Buffer);
    isNumber(L, ARG_ReadLen);
    isNumber(L, ARG_ReadLen);
    isNumber(L, ARG_FileLen);


    dwFileLen    = lua_tonumber(L, ARG_FileLen);

    szFilePath  = InitStringArg(L, ARG_FilePath);
    szFileName    = InitStringArg(L, ARG_FileName);

    dwBufferSize = luaL_getn(L, ARG_Buffer);
    dwReadLen    = lua_tonumber(L, ARG_ReadLen);

    if (dwReadLen != 0)
    {
        dwBlockSize = dwReadLen;
    }
    else
    {
        dwBlockSize = DEFAULT_READ_WRITE_LEN;
    }

    pTestBuffer = (LPBYTE)LocalAlloc(0, dwBlockSize );

    if (pTestBuffer == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    //Initialize test buffer.
    for (cnt=0, i=1; cnt<dwBlockSize; cnt++, i++)
    {
        if(i <= dwBufferSize)
        {
            lua_rawgeti(L, ARG_Buffer, i);
            pTestBuffer[cnt] = lua_tonumber(L,-1);
            lua_pop(L, 1);
        }
        else
        {
            pTestBuffer[cnt] = pTestBuffer[cnt % dwBufferSize];
        }

    }

    bReturnValue = FSDT_VerifFile(szFilePath, szFileName,pTestBuffer, dwBlockSize, dwFileLen);

Bail:
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }
    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);
    FreeStringArg(&szFileName);
    return 1;
}

static int l_ReadFile (lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,    //文件路径     参数类型String
        ARG_FileName,        //文件名       参数类型String
        ARG_ReadLen          //每次读取的长度, 如果为0则使用默认  参数类型Int32
    };
    BOOL        bReturnValue    = TRUE;
    PBYTE       pTestBuffer     = NULL;
    DWORD        dwBlockSize        = 0;
    UINT32        cnt             = 0;
    UINT32        i                = 0;

    PTCHAR    szFilePath        = NULL;
    PTCHAR    szFileName        = NULL;
    DWORD        dwReadLen        = 0;
    DWORD        dwFileLen        = 0;

    isString(L, ARG_FilePath);
    isString(L, ARG_FileName);
    isNumber(L, ARG_ReadLen);

    szFilePath  = InitStringArg(L, ARG_FilePath);
    szFileName    = InitStringArg(L, ARG_FileName);

    dwReadLen    = lua_tonumber(L, ARG_ReadLen);

    if (dwReadLen != 0)
    {
        dwBlockSize = dwReadLen;
    }
    else
    {
        dwBlockSize = DEFAULT_READ_WRITE_LEN;
    }

    pTestBuffer = (LPBYTE)LocalAlloc(0, dwBlockSize );

    if (pTestBuffer == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    bReturnValue = FSDT_ReadFile(szFilePath, szFileName,pTestBuffer, dwBlockSize);

Bail:
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }
    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);
    FreeStringArg(&szFileName);
    return 1;
}
/*

BOOL CompareFiles( TCHAR *szSrcFileSpec, TCHAR *szDstFileSpec )
*/
/*
static int l_CompareFiles (lua_State *L)
{
    enum ARG
    {
        ARG_SrcFileSpec = 1, //第一个文件的路径 参数类型String
        ARG_DstFileSpec      //第二个文件的路径 参数类型String
    };
    BOOL    bReturnValue    = TRUE;


    PTCHAR    szSrcFileSpec;
    PTCHAR    szDstFileSpec;

    isString(L, ARG_SrcFileSpec);
    isString(L, ARG_DstFileSpec);

    szSrcFileSpec    = InitStringArg(L, ARG_SrcFileSpec);
    szDstFileSpec    = InitStringArg(L, ARG_DstFileSpec);

    _CCprintf( TEXT("CompareFiles: \"%s\"...to...\"%s\"\n"), szSrcFileSpec, szDstFileSpec);
    bReturnValue = FSDT_CompareFiles(szSrcFileSpec, szDstFileSpec);



    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szSrcFileSpec);
    FreeStringArg(&szDstFileSpec);
    return 1;
}

/*
BOOL CopyDirectory(TCHAR *szSourcePath, TCHAR *szDestPath, BOOL bOverLay)
*/
/*
static int l_CopyDirectory (lua_State *L)
{
    enum ARG
    {
        ARG_SourcePath = 1, //源文件夹路径  参数类型String
        ARG_DestPath,       //目标文件夹路径  参数类型String
        ARG_OverLay         //是否覆盖 如果此参数为true则在复制过程中, 不覆盖已有文件. 反之
        //则覆盖 参数类型: bool
    };
    BOOL    bReturnValue    = TRUE;


    PTCHAR    szSourcePath;
    PTCHAR    szDestPath; 
    BOOL     bOverLay;

    isString(L, ARG_SourcePath);
    isString(L, ARG_DestPath);
    isBoolean(L, ARG_OverLay);

    szSourcePath   = InitStringArg(L, ARG_SourcePath);
    szDestPath     = InitStringArg(L, ARG_DestPath);
    bOverLay       = lua_toboolean(L, ARG_OverLay);

    _CCprintf( TEXT("CopyDirectory \"%s\"...to...\"%s\"\n"), szSourcePath, szDestPath);

    bReturnValue = FSDT_CopyDirectory(szSourcePath,szDestPath, bOverLay);

    lua_pushboolean(L,bReturnValue);
    FreeStringArg(&szSourcePath);
    FreeStringArg(&szDestPath);
    return 1;
}

static int l_MoveDirectory (lua_State *L)
{
    enum ARG
    {
        ARG_SourcePath = 1,  //源文件夹路径 参数类型String
        ARG_DestPath         //目标文件夹路径 参数类型String
    };
    BOOL    bReturnValue    = TRUE;

    PTCHAR    szSourcePath;
    PTCHAR    szDestPath;  

    isString(L, ARG_SourcePath);
    isString(L, ARG_DestPath);

    szSourcePath    = InitStringArg(L, ARG_SourcePath);
    szDestPath      = InitStringArg(L, ARG_DestPath);

    _CCprintf( TEXT("MoveDirectory \"%s\"...to...\"%s\"\n"), szSourcePath, szDestPath);

    bReturnValue = FSDT_MoveDirectory(szSourcePath,szDestPath);

    lua_pushboolean(L,bReturnValue);
    FreeStringArg(&szSourcePath);
    FreeStringArg(&szDestPath);
    return 1;
}

static int l_CompareDirectory (lua_State *L)
{
    enum ARG
    {
        ARG_SourcePath = 1,     //源文件夹路径  参数类型String
        ARG_DestPath            ////目标文件夹路径参数类型String
    };
    BOOL    bReturnValue    = TRUE;


    PTCHAR   szSourcePath ;
    PTCHAR   szDestPath   ;

    isString(L, ARG_SourcePath);
    isString(L, ARG_DestPath);

    szSourcePath    = InitStringArg(L, ARG_SourcePath);
    szDestPath      = InitStringArg(L, ARG_DestPath);

    _CCprintf( TEXT("CompareDirectory \"%s\"...to...\"%s\"\n"), szSourcePath, szDestPath);

    bReturnValue = FSDT_CompareDirectory(szSourcePath,szDestPath);

    lua_pushboolean(L,bReturnValue);
    FreeStringArg(&szSourcePath);
    FreeStringArg(&szDestPath);
    return 1;
}

static int l_DeleteTree(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,           //要删除的文件夹路径    参数类型String
        ARG_DontDeleteRoot      //是否要保留根目录      参数类型bool
    };
    BOOL bReturnValue  = TRUE;
    PTCHAR szPath;
    BOOL bDontDeleteRoot;

    isString(L, ARG_Path);
    isBoolean(L, ARG_DontDeleteRoot);

    szPath = InitStringArg(L, ARG_Path);
    bDontDeleteRoot = lua_toboolean(L, ARG_DontDeleteRoot);
    bReturnValue = FSDT_DeleteTree(szPath, bDontDeleteRoot);

    lua_pushboolean(L, bReturnValue);
    FreeStringArg(&szPath);

    return 1;
}
/*
static int l_DeleteFile(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,       //文件路径
    };
    BOOL bReturnValue  = TRUE;
    PTCHAR szPath;

    isString(L, ARG_Path);

    szPath = InitStringArg(L, ARG_Path);

    _CCprintf(TEXT("Delete: \"%s\"...\n"), szPath);

    if (!FSDT_FileExists(szPath))
    {
        _CCprintf( _T("ERROR: File Not Exists: \"%s\" \n"), szPath);
        bReturnValue = FALSE;
        goto bail;
    }



    //make sure file att is NOT read only.
    if (GetFileAttributes(szPath) & FILE_ATTRIBUTE_READONLY) {
        if (!SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL)) {
            ERR("SetFileAttributes()");
            _CCprintf( _T("Could not set attributes: \"%s\" Last error == 0x%08X\n"), szPath, GetLastError() );
        }
    }
    bReturnValue = DeleteFile(szPath);

    if (!bReturnValue)
    {
        ERR("DeleteFile()");
        _CCprintf( _T("Could not delete file: \"%s\" Last error == 0x%08X\n"), szPath, GetLastError());
    }

bail:
    lua_pushboolean(L, bReturnValue);
    FreeStringArg(&szPath);

    return 1;
}
*//*
static int l_CopyFile(lua_State *L)
{
    enum ARG
    {
        ARG_SourcePath = 1,         //需要复制的源文件 参数类型String
        ARG_DestPath,               //目标路径         参数类型String
        ARG_OverLay                 //是否覆盖         参数类型bool
    };
    BOOL bReturnValue  = TRUE;

    PTCHAR    szSourcePath ;
    PTCHAR    szDestPath   ;
    BOOL     bOverLay      ;

    isString(L, ARG_SourcePath);
    isString(L, ARG_DestPath);
    isBoolean(L, ARG_OverLay);

    szSourcePath   = InitStringArg(L, ARG_SourcePath);
    szDestPath     = InitStringArg(L, ARG_DestPath);
    bOverLay       = lua_toboolean(L, ARG_OverLay);


    _CCprintf( TEXT("Copy \"%s\"...to...\"%s\"\n"), szSourcePath, szDestPath);


    if(!CopyFile(szSourcePath,szDestPath, bOverLay))
    {
        if (!(TRUE == bOverLay && GetLastError() == ERROR_FILE_EXISTS) )
        {
            ERR("CopyFile()");
            bReturnValue = FALSE;
        }
    }

    lua_pushboolean(L,bReturnValue);
    FreeStringArg(&szSourcePath);
    FreeStringArg(&szDestPath);

    return 1;
}

static int l_MoveFile(lua_State *L)
{
    enum ARG
    {
        ARG_SourcePath = 1,         //源路径   参数类型String
        ARG_DestPath,               //目标路径 参数类型String
    };
    BOOL bReturnValue  = TRUE;

    PTCHAR    szSourcePath ;
    PTCHAR    szDestPath   ;

    isString(L, ARG_SourcePath);
    isString(L, ARG_DestPath);

    szSourcePath   = InitStringArg(L, ARG_SourcePath);
    szDestPath     = InitStringArg(L, ARG_DestPath);


    _CCprintf( TEXT("Move \"%s\"...to...\"%s\"\n"), szSourcePath, szDestPath);


    if(!MoveFile(szSourcePath,szDestPath))
    {
        ERR("MoveFile()");
        bReturnValue = FALSE;
    }

    lua_pushboolean(L,bReturnValue);
    FreeStringArg(&szSourcePath);
    FreeStringArg(&szDestPath);

    return 1;
}

static int l_CreateDirectory(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,                    //创建目录
    };

    BOOL bReturnValue  = TRUE;

    PTCHAR szPath;

    isString(L, ARG_Path);

    szPath = InitStringArg(L, ARG_Path);

    _CCprintf( TEXT("CreateDirectory: \"%s\"...\n"), szPath);

    bReturnValue = CreateDirectory(szPath, NULL);
    if (!bReturnValue)
    {
        ERR("CreateDirectory()");
    }

    lua_pushboolean(L, bReturnValue);
    FreeStringArg(&szPath);
    return 1;
}

static int l_FileSize(lua_State *L)
{
    enum ARG
    {
        ARG_FileName = 1,    //文件路径  参数类型String
    };

    __int64 i64FileSize  = 0;

    PTCHAR szFileName;

    isString(L, ARG_FileName);

    szFileName = InitStringArg(L, ARG_FileName);

    i64FileSize = FSDT_FileSize(szFileName);


    _CCprintf( TEXT("Size of \"%s\": %I64u\n"), szFileName, i64FileSize);


    lua_pushnumber(L, i64FileSize);
    FreeStringArg(&szFileName);
    return 1;
}*/
static int l_GetInfo(lua_State *L)
{
    TCHAR szValue[32];
    MEMORYSTATUS MM;

    GlobalMemoryStatus(&MM);



    _CCprintf( TEXT("\n"));
    _CCprintf( TEXT("Memory Statu Information:\n"));
    _CCprintf( TEXT("  Length:            %12s\n"), FormatValue(MM.dwLength, szValue));
    _CCprintf( TEXT("  MemoryLoad:        %12s\n"), FormatValue(MM.dwMemoryLoad, szValue));
    _CCprintf( TEXT("  TotalPhys:         %12s\n"), FormatValue(MM.dwTotalPhys, szValue));
    _CCprintf( TEXT("  AvailPhys          %12s\n"), FormatValue(MM.dwAvailPhys, szValue));
    _CCprintf( TEXT("  TotalPageFile:     %12s\n"), FormatValue(MM.dwTotalPageFile, szValue));
    _CCprintf( TEXT("  AvailPageFile:     %12s\n"), FormatValue(MM.dwAvailPageFile, szValue));
    _CCprintf( TEXT("  TotalVirtual:      %12s\n"), FormatValue(MM.dwTotalVirtual, szValue));
    _CCprintf( TEXT("  AvailVirtual:      %12s\n"), FormatValue(MM.dwAvailVirtual, szValue));
    _CCprintf( TEXT("Lua Statu Information:\n"));
    _CCprintf( TEXT("  Gc_Count:          %12s\n"), FormatValue(lua_gc(L, LUA_GCCOUNT, 0), szValue));
    _CCprintf( TEXT("\n"));


    return 0;
}

static int l_GetTickCount(lua_State *L)
{

    lua_pushnumber(L, GetTickCount());
    return 1;
}
static int l_GetFATInfo(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,          //驱动路径.   参数类型String
    };

    BOOL bReturnValue = TRUE;

    PTCHAR szPath;

    isString(L, ARG_Path);

    szPath = InitStringArg(L, ARG_Path);

    bReturnValue = FSDT_GetFATInfo(szPath);

    lua_pushboolean(L, bReturnValue);

    FreeStringArg(&szPath);
    return 1;
}



static int l_GetClusterSize(lua_State *L)
{

    enum ARG
    {
        ARG_Path = 1,          //驱动路径.   参数类型String
    };

    DWORD dwReturnValue = 0;

    PTCHAR szPath;
    DWORD dwBytesPerSector, dwSectorsPerCluster;
    isString(L, ARG_Path);

    szPath = InitStringArg(L, ARG_Path);

#ifdef UNDER_CE
    {
        DEVPB volpb = {0};
        TCHAR szVol[MAX_PATH];
        TCHAR szTmp[MAX_PATH];
        TCHAR* ptS = NULL;
        HANDLE hVol;
        DWORD cb = 0;
        FREEREQ fr = {0};

        _tcscpy(szTmp, szPath);
        ptS = _tcsstr(szTmp+1, TEXT("\\"));
        if(ptS != NULL)
        {
            *ptS = TEXT('\0');
        }
        _stprintf(szVol, _T("%s\\VOL:"), szTmp);

        hVol = CreateFile(szVol, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

        if(INVALID_HANDLE(hVol))
        {
            ERR("CreateFile()");
            _CCprintf( _T("Unable to open disk: %s Last error == 0x%08X\n"), szVol, GetLastError());
            goto EXIT;
        }

        if(!DeviceIoControl(hVol, IOCTL_DISK_GET_VOLUME_PARAMETERS, NULL, 0, &volpb, 
            sizeof(volpb), &cb, NULL) || cb != sizeof(volpb))
        {
            ERR("DeviceIoControl(IOCTL_DISK_GET_VOLUME_PARAMETERS)");
            goto EXIT;
        }
        else
        {
            dwBytesPerSector    = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_BytesPerSector;
            dwSectorsPerCluster = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_SectorsPerCluster;
            dwReturnValue = dwSectorsPerCluster * dwBytesPerSector;
        }

    }
#else
    {
        GetDiskFreeSpace(szPath, &dwBytesPerSector,&dwSectorsPerCluster, NULL, NULL);
        dwReturnValue = dwBytesPerSector * dwSectorsPerCluster;
    }
#endif

EXIT:
    lua_pushnumber(L, dwReturnValue);

    FreeStringArg(&szPath);
    return 1;
}




static int l_OpenLogFile(lua_State *L)
{
    enum ARG
    {
        ARG_FileName=1,
    };
    luaL_checktype(L, ARG_FileName, LUA_TSTRING);
    fl_Logout = fopen(lua_tostring(L, ARG_FileName), "w");
    return 0;
}

static int l_CloseLogFile(lua_State *L)
{

    if (fl_Logout != NULL)
    {
        fclose(fl_Logout);
    }
    fl_Logout = NULL;
    return 0;
}

static int l_FileExists(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,      //文件路径  参数类型String
    };

    BOOL bReturnValue = TRUE;

    PTCHAR szPath;

    isString(L, ARG_Path);

    szPath = InitStringArg(L, ARG_Path);

    bReturnValue = FSDT_FileExists(szPath);

    lua_pushboolean(L, bReturnValue);

    FreeStringArg(&szPath);
    return 1;
}

// return 2 rst
static int l_FillDisk(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,       //需要填充的文件夹.       参数类型String
        ARG_FileNameTmp,    //文件名模版, 程序将在它后面加数字作为文件名 参数类型String
        ARG_FreeSpace       //需要剩余空间的大小.     参数类型Int32
    };

    BOOL bReturnValue = TRUE;

    PTCHAR szPath;
    PTCHAR szFileNameTmp;
    DWORD dwFreeSpace;
    DWORD dwFileCnt =0;

    isString(L, ARG_Path);
    isString(L, ARG_FileNameTmp);
    isNumber(L, ARG_FreeSpace);


    szPath = InitStringArg(L, ARG_Path);
    szFileNameTmp = InitStringArg(L, ARG_FileNameTmp);
    dwFreeSpace = lua_tonumber(L, ARG_FreeSpace);

    bReturnValue = FSDT_FillDiskMinusXBytesWithMapping(szPath, szFileNameTmp, dwFreeSpace, &dwFileCnt);


    lua_pushboolean(L, bReturnValue);
    lua_pushnumber(L, dwFileCnt);


    FreeStringArg(&szPath);
    FreeStringArg(&szFileNameTmp);
    return 2;
}

static int l_GetDiskFreeSpace(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,    //存储器路径   参数类型:String
    };

    __int64 i64ReturnValue = 0;

    PTCHAR szPath;

    isString(L, ARG_Path);

    szPath = InitStringArg(L, ARG_Path);


    i64ReturnValue = FSDT_GetDiskFreeSpace(szPath);

    lua_pushnumber(L, i64ReturnValue);

    FreeStringArg(&szPath);
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
printf("123");

    _CCprintf( L"%s\n", Str);

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

    _CCprintf( L"%s\n", Str);

    FreeStringArg(&Str);
    return 0;
}

/*
BOOL FSDT_SeekTest(HANDLE hFile, PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64SeekPos, DWORD OperatorLen, DWORD dwMoveMethod)
*//*
static int l_SeekTest(lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,      //文件路径     参数类型String
        ARG_SeekPos,           //Seek的点列表 参数类型Table
        ARG_OperatorLen,       //操作后写入读取的长度. 参数类型Table
        ARG_Buffer,            //写入的Buffer          参数类型Table
        ARG_MoveMethod         //移动方式. FILE_BEGIN  FILE_CURRENT FILE_END   参数类型int32
    };

    BOOL         bReturnValue    = TRUE;
    PBYTE        pTestBuffer     = NULL;
    __int64      *pTestPos       = NULL;
    DWORD        *pLen           = NULL;

    UINT32       cnt             = 0;
    UINT32       i               = 0;
    PTCHAR       szFilePath      = NULL;
    DWORD        dwBufferSize    = 0;
    DWORD        PosTableSize    = 0;
    DWORD        LenTableSize    = 0;
    DWORD        dwMoveMethod    = 0;
    DWORD        dwBlockSize     = 0;
    HANDLE       hFile           = INVALID_HANDLE_VALUE;
    UINT         PosCnt          = 0;
    UINT         LenCnt          = 0;


    isString(L, ARG_FilePath);
    isTable(L, ARG_SeekPos);
    isTable(L, ARG_OperatorLen);
    isTable(L, ARG_Buffer);
    isNumber(L, ARG_MoveMethod);

    szFilePath      = InitStringArg(L, ARG_FilePath);
    PosTableSize    = luaL_getn(L, ARG_SeekPos);
    LenTableSize    = luaL_getn(L, ARG_OperatorLen);
    dwBufferSize    = luaL_getn(L, ARG_Buffer);
    dwMoveMethod    = lua_tonumber(L, ARG_MoveMethod);


    //init pos array
    pTestPos = (__int64 *)LocalAlloc(0, PosTableSize*sizeof(__int64));

    if (pTestPos == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    for (cnt=0; cnt<PosTableSize; cnt++)
    {

        lua_rawgeti(L, ARG_SeekPos, cnt+1);

        pTestPos[cnt] = lua_tonumber(L,-1);
        lua_pop(L, 1);

    }

    //init len array
    pLen = (DWORD *)LocalAlloc(0, LenTableSize*sizeof(DWORD));
    if (pLen == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }
    for (cnt=0; cnt<LenTableSize; cnt++)
    {
        lua_rawgeti(L, ARG_OperatorLen, cnt+1);
        pLen[cnt] = lua_tonumber(L,-1);
        lua_pop(L, 1);
    }

    //open file to test
    hFile = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (INVALID_HANDLE_VALUE == hFile )
    {
        bReturnValue = FALSE;
        ERR("CreateFile()");
        _CCprintf( _T("Unable to open file \"%s\" "), szFilePath);
        _CCprintf( _T("Last error == 0x%08X\n"), GetLastError());
        goto Bail;
    }


    for (LenCnt=0; LenCnt<LenTableSize; LenCnt++)
    {

        if (pLen[LenCnt] <= DEFAULT_READ_WRITE_LEN)
        {
            dwBlockSize = pLen[LenCnt];
        }
        else
        {
            dwBlockSize = DEFAULT_READ_WRITE_LEN;
        }
        //Initialize test buffer.
        pTestBuffer = (PBYTE)LocalAlloc(0, dwBlockSize );

        if (pTestBuffer == NULL )
        {
            ERR("LocalAlloc()");
            bReturnValue = FALSE;
            goto Bail;
        }





        for (cnt=0, i=1; cnt<dwBlockSize; cnt++, i++)
        {
            if(i <= dwBufferSize)
            {
                lua_rawgeti(L, ARG_Buffer, i);
                pTestBuffer[cnt] = lua_tonumber(L,-1);
                lua_pop(L, 1);
            }
            else
            {
                pTestBuffer[cnt] = pTestBuffer[cnt % dwBufferSize];
            }

        }


        for (PosCnt=0; PosCnt<PosTableSize; PosCnt++)
        {
            _CCprintf( TEXT("Seeking @ %d"),pTestPos[PosCnt]);
            _CCprintf( TEXT("======Len: %d\n"),pLen[LenCnt]);
            bReturnValue &= FSDT_SeekTest(hFile, pTestBuffer, dwBlockSize, pTestPos[PosCnt], pLen[LenCnt], dwMoveMethod);
        }

        if (pTestBuffer)
        {
            LocalFree(pTestBuffer );
            pTestBuffer = NULL;
        }

    }

Bail:
    if (VALID_HANDLE(hFile))
    {

        if (!CloseHandle(hFile ) )
        {
            ERR("CloseHandle()");
        }

    }
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }

    if (pTestPos)
    {
        LocalFree(pTestPos );
    }

    if (pLen)
    {
        LocalFree(pLen );
    }

    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);
    return 1;
}
static int l_SeekReadTest(lua_State *L)
{
    enum ARG
    {
        ARG_FilePath = 1,      //文件路径     参数类型String
        ARG_SeekPos,           //Seek的点列表 参数类型Table
        ARG_OperatorLen,       //操作后写入读取的长度. 参数类型Table
        ARG_Buffer,            //写入的Buffer          参数类型Table
        ARG_MoveMethod         //移动方式. FILE_BEGIN  FILE_CURRENT FILE_END   参数类型int32
    };

    BOOL         bReturnValue    = TRUE;
    PBYTE        pTestBuffer     = NULL;
    __int64      *pTestPos       = NULL;
    DWORD        *pLen           = NULL;

    UINT32       cnt             = 0;
    UINT32       i               = 0;
    PTCHAR       szFilePath      = NULL;
    DWORD        dwBufferSize    = 0;
    DWORD        PosTableSize    = 0;
    DWORD        LenTableSize    = 0;
    DWORD        dwMoveMethod    = 0;
    DWORD        dwBlockSize     = 0;
    HANDLE       hFile           = INVALID_HANDLE_VALUE;
    UINT         PosCnt          = 0;
    UINT         LenCnt          = 0;


    isString(L, ARG_FilePath);
    isTable(L, ARG_SeekPos);
    isTable(L, ARG_OperatorLen);
    isTable(L, ARG_Buffer);
    isNumber(L, ARG_MoveMethod);

    szFilePath      = InitStringArg(L, ARG_FilePath);
    PosTableSize    = luaL_getn(L, ARG_SeekPos);
    LenTableSize    = luaL_getn(L, ARG_OperatorLen);
    dwBufferSize    = luaL_getn(L, ARG_Buffer);
    dwMoveMethod    = lua_tonumber(L, ARG_MoveMethod);


    //init pos array
    pTestPos = (__int64 *)LocalAlloc(0, PosTableSize*sizeof(__int64));

    if (pTestPos == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    for (cnt=0; cnt<PosTableSize; cnt++)
    {

        lua_rawgeti(L, ARG_SeekPos, cnt+1);

        pTestPos[cnt] = lua_tonumber(L,-1);
        lua_pop(L, 1);

    }

    //init len array
    pLen = (DWORD *)LocalAlloc(0, LenTableSize*sizeof(DWORD));
    if (pLen == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }
    for (cnt=0; cnt<LenTableSize; cnt++)
    {
        lua_rawgeti(L, ARG_OperatorLen, cnt+1);
        pLen[cnt] = lua_tonumber(L,-1);
        lua_pop(L, 1);
    }

    //open file to test
    hFile = CreateFile(szFilePath, GENERIC_READ | GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


    if (INVALID_HANDLE_VALUE == hFile )
    {
        bReturnValue = FALSE;
        ERR("CreateFile()");
        _CCprintf( _T("Unable to open file \"%s\" "), szFilePath);
        _CCprintf( _T("Last error == 0x%08X\n"), GetLastError());
        goto Bail;
    }


    for (LenCnt=0; LenCnt<LenTableSize; LenCnt++)
    {

        if (pLen[LenCnt] <= DEFAULT_READ_WRITE_LEN)
        {
            dwBlockSize = pLen[LenCnt];
        }
        else
        {
            dwBlockSize = DEFAULT_READ_WRITE_LEN;
        }
        //Initialize test buffer.
        pTestBuffer = (PBYTE)LocalAlloc(0, dwBlockSize );

        if (pTestBuffer == NULL )
        {
            ERR("LocalAlloc()");
            bReturnValue = FALSE;
            goto Bail;
        }





        for (cnt=0, i=1; cnt<dwBlockSize; cnt++, i++)
        {
            if(i <= dwBufferSize)
            {
                lua_rawgeti(L, ARG_Buffer, i);
                pTestBuffer[cnt] = lua_tonumber(L,-1);
                lua_pop(L, 1);
            }
            else
            {
                pTestBuffer[cnt] = pTestBuffer[cnt % dwBufferSize];
            }

        }


        for (PosCnt=0; PosCnt<PosTableSize; PosCnt++)
        {
            _CCprintf( TEXT("Seeking @ %d"),pTestPos[PosCnt]);
            _CCprintf( TEXT("======Len: %d\n"),pLen[LenCnt]);
            bReturnValue &= FSDT_SeekReadTest(hFile, pTestBuffer, dwBlockSize, pTestPos[PosCnt], pLen[LenCnt], dwMoveMethod);
        }

        if (pTestBuffer)
        {
            LocalFree(pTestBuffer );
            pTestBuffer = NULL;
        }

    }

Bail:
    if (VALID_HANDLE(hFile))
    {

        if (!CloseHandle(hFile ) )
        {
            ERR("CloseHandle()");
        }

    }
    if (pTestBuffer)
    {
        LocalFree(pTestBuffer );
    }

    if (pTestPos)
    {
        LocalFree(pTestPos );
    }

    if (pLen)
    {
        LocalFree(pLen );
    }

    lua_pushboolean(L,bReturnValue);

    FreeStringArg(&szFilePath);
    return 1;
}

/*BOOL FSDT_CutFile(HANDLE hFile, __int64 i64SeekPos, DWORD dwMoveMethod)*/
static int l_CutFile(lua_State *L)
{
    enum ARG
    {
        ARG_Path = 1,           //文件路径 参数类型String
        ARG_SeekPos,            //开始切除的点, 参数类型int32
        ARG_MoveMethod          //移动方式  FILE_BEGIN  FILE_CURRENT FILE_END   参数类型int32
    };

    BOOL         bReturnValue    = TRUE;
    HANDLE       hFile           = INVALID_HANDLE_VALUE;

    PTCHAR  szPath      ;
    __int64 i64SeekPos  ;
    DWORD   dwMoveMethod;

    isString(L, ARG_Path);
    isNumber(L, ARG_SeekPos);
    isNumber(L, ARG_MoveMethod);

    szPath       = InitStringArg(L, ARG_Path);
    i64SeekPos   = lua_tonumber(L, ARG_SeekPos);
    dwMoveMethod = lua_tonumber(L, ARG_MoveMethod);


    _CCprintf( TEXT("CutFile: \"%s\"\n"), szPath);


    hFile = CreateFile(szPath, GENERIC_READ | GENERIC_WRITE, 0,
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (INVALID_HANDLE_VALUE == hFile )
    {

        bReturnValue = FALSE;
        ERR("CreateFile()");
        _CCprintf( _T("Unable to open file \"%s\""), szPath);
        _CCprintf( _T("Last error == 0x%08X\n"), GetLastError());
        goto Bail;
    }
    bReturnValue = FSDT_CutFile(hFile, i64SeekPos, dwMoveMethod);

Bail:

    if (VALID_HANDLE(hFile))
    {

        if (!CloseHandle(hFile ) )
        {
            ERR("CloseHandle()");
        }

    }

    lua_pushboolean(L, bReturnValue);

    FreeStringArg(&szPath);
    return 1;
}

static int l_MemWatch(lua_State *L)
{
#ifdef UNDER_CE
    MemWatch();
#endif
    return 0;
}

#ifdef UNDER_CE 
GPIOTHREAD_PARAM gpio_param;
HANDLE      hGpioTestThread      = INVALID_HANDLE_VALUE;
UINT32*     ptrPinGrp = NULL;
#endif

static int l_GpioTest (lua_State *L)
{
    enum ARG
    {

        ARG_Pins=1,   
        ARG_WaitTime,
        ARG_LockTime
    };

    BOOL        bReturnValue    = TRUE;
#ifdef UNDER_CE   
    UINT32        cnt             = 0;
    
    DWORD        dwPinCnt    = 0;
    DWORD dwWaitTime = 0;
    DWORD dwLockTime = 0;
    
    isTable(L, ARG_Pins);
    luaL_checktype(L, ARG_WaitTime, LUA_TNUMBER);
    luaL_checktype(L, ARG_LockTime, LUA_TNUMBER);
    dwWaitTime = lua_tonumber(L, ARG_WaitTime);
    dwLockTime = lua_tonumber(L, ARG_LockTime);
    
    dwPinCnt = luaL_getn(L, ARG_Pins);

    ptrPinGrp = (UINT32*)LocalAlloc(0, dwPinCnt*sizeof(UINT32));

    if (ptrPinGrp == NULL )
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    for (cnt=1; cnt<=dwPinCnt; cnt++)
    {
        lua_rawgeti(L, ARG_Pins, cnt);
        ptrPinGrp[cnt-1] = lua_tonumber(L,-1);
        lua_pop(L, 1);
    }
    
    gpio_param.bTerminated = FALSE;
    gpio_param.pinCnt = dwPinCnt;
    gpio_param.ptrPins = ptrPinGrp;
    gpio_param.dwWaitTime = lua_tonumber(L, ARG_WaitTime);
    gpio_param.dwLockTime = lua_tonumber(L, ARG_LockTime);

    hGpioTestThread = CreateThread(NULL,
        0,
        gpioTestThread,
        (LPVOID)&gpio_param,
        0,
        NULL);

    if (INVALID_HANDLE(hGpioTestThread))
    {
        bReturnValue = FALSE;
        goto Bail;
    }

Bail:

#endif
    lua_pushboolean(L,bReturnValue);
    return 1;
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

    _CCprintf( TEXT("Sleep %d \n"), wTime);

    Sleep(wTime);
    return 0;
}
static int l_GpioStop (lua_State *L)
{
    enum ARG
    {

        ARG_WaitTime=1, 

    };

    DWORD dwReturnValue = 0;
    DWORD wTime = 0;
    isNumber(L, ARG_WaitTime);

    wTime = lua_tonumber(L, ARG_WaitTime);

#ifdef UNDER_CE   
    if (INVALID_HANDLE(hGpioTestThread))
    {
        dwReturnValue = -2;
        goto Bail;
    }

    gpio_param.bTerminated = TRUE;
    if (WAIT_TIMEOUT == WaitForSingleObject(hGpioTestThread,wTime))
    {
        _CCprintf(TEXT("Error:GpioStop: WAIT_TIMEOUT == WaitForSingleObject\n"));
        TerminateThread(hGpioTestThread, -3);
    }
    GetExitCodeThread(hGpioTestThread, &dwReturnValue);
    _CCprintf(TEXT("GpioStop: get the exit code: %d\n"), dwReturnValue);

Bail:
    if (ptrPinGrp != NULL)
    {
        LocalFree(ptrPinGrp);
        ptrPinGrp = NULL;
    }
#endif
    lua_pushnumber(L, dwReturnValue);
    return 1;
}

//***********************************************************
//  bad block test 
//***********************************************************

#ifdef UNDER_CE  
int getfield (lua_State *L, const char *key) {
    int result;
    lua_pushstring(L, key);
    lua_gettable(L, -2);  /* get background[key] */
    if (!lua_isnumber(L, -1))
        error(L, "invalid component in BlockStatusTable");
    result = (int)lua_tonumber(L, -1);
    lua_pop(L, 1);  /* remove number */
    return result;
}

#endif
static int l_initBadTable (lua_State *L) {
    enum ARG
    {
        ARG_StatusTable = 1,
    };
#ifdef UNDER_CE  
    DWORD dwItemCnt = 0;
    DWORD cnt = 0;

    FlashRegion Info;
    T_NAND_INFO NandInfo;
    struct BlockStatusTable* ptrStatus = NULL;
    struct BlockStatusTable* ptrStatusTable = NULL;

    // get the infomation from fal
    if (!GetFalInfo(&Info))
    {
        error(L, "Error @  l_initBadTable GetFalInfo");
    }
    if (!GetNandInfo(&NandInfo))
    {
        error(L, "Error @  l_initBadTable GetNandInfo");
    }

  
    luaL_checktype(L, ARG_StatusTable, LUA_TTABLE);

    dwItemCnt = luaL_getn(L, ARG_StatusTable);


    ptrStatusTable = LocalAlloc(LPTR, sizeof(struct BlockStatusTable)*(NandInfo.Blocks*NandInfo.FalSectorsPerBlock));

    if (ptrStatusTable == NULL)
    {
        error(L, "Error @  l_initBadTable LocalAlloc");
    }


    for (cnt=1; cnt<=dwItemCnt; ++cnt)
    {
        lua_rawgeti(L, ARG_StatusTable, cnt);
        ptrStatus = &ptrStatusTable[
            (Info.dwStartPhysBlock+getfield(L, "block")) * (NandInfo.FalSectorsPerBlock)
                + getfield(L, "sector")%(NandInfo.FalSectorsPerBlock)];
        ptrStatus->bad_type = getfield(L, "bad_type");
        ptrStatus->count = getfield(L, "count");
    }
    // todo: IO CTRL

    if(!SetBadList(ptrStatusTable, sizeof(struct BlockStatusTable)*(NandInfo.Blocks*NandInfo.FalSectorsPerBlock)))
    {
        error(L, "Error @  l_initBadTable SetBadList");
    }

    LocalFree(ptrStatusTable);
#endif
    return 0;  /* no results */
}
static int l_BadTest (lua_State *L) {
    enum ARG
    {
        ARG_Status = 1,
    };
#ifdef UNDER_CE  
    luaL_checktype(L, ARG_Status, LUA_TBOOLEAN);
    if (!SetBadTest(lua_toboolean(L, ARG_Status)))
    {
        error(L, "Error @  l_BadTest SetBadTest");
    }
#endif
    return 0;
}

DWORD m_dwSectorSize = 0;
static int l_GetSectorSize (lua_State *L) {
    
#ifdef UNDER_CE
    if (m_dwSectorSize == 0)
    {
        T_NAND_INFO NandInfo;
        if (!GetNandInfo(&NandInfo))
        {
            error(L, "Error @  l_initBadTable GetNandInfo");
        }
        m_dwSectorSize = NandInfo.FalSectorsPerBlock;
    }
#endif
    lua_pushnumber(L, m_dwSectorSize);
    return 1;
}

static int l_ActivateDevice (lua_State *L) {
    enum ARG
    {
        ARG_DeviceKey = 1,          
    };
    
    PTCHAR szPath;
    BOOL fRet = FALSE;
#ifdef UNDER_CE
    szPath = InitStringArg(L, ARG_DeviceKey);
    fRet = Test_ActivateDevice(szPath);
    FreeStringArg(&szPath);
    lua_pushboolean(L, fRet);
    return 1;
#endif
    lua_pushboolean(L, FALSE);
    return 1;
}

static int l_DeactivateDevice (lua_State *L) {
    enum ARG
    {
        ARG_DeviceKey = 1,          
    };

    PTCHAR szPath;
    BOOL fRet = FALSE;
#ifdef UNDER_CE
    szPath = InitStringArg(L, ARG_DeviceKey);
    fRet = Test_DeactivateDevice(szPath);
    FreeStringArg(&szPath);
    lua_pushboolean(L, fRet);
    return 1;
#endif
    lua_pushboolean(L, FALSE);
    return 1;
}

static int l_SDInitCounter (lua_State *L) {

    enum ARG
    {
        ARG_DeviceKey = 1,          
    };

    PTCHAR szPath;
    BOOL fRet = FALSE;

#ifdef UNDER_CE
    szPath = InitStringArg(L, ARG_DeviceKey);
    fRet = SDInitCounter(szPath);
    lua_pushboolean(L, fRet);
    FreeStringArg(&szPath);
    return 1;
#endif
    lua_pushboolean(L, FALSE);
    return 1;
}

static int l_SDPrintCountRst (lua_State *L) {
    enum ARG
    {
        ARG_DeviceKey = 1,          
    };
    BOOL fRet = FALSE;
    PTCHAR szPath;
#ifdef UNDER_CE
    szPath = InitStringArg(L, ARG_DeviceKey);
    fRet = SDPrintCountRst(szPath);
    lua_pushboolean(L, fRet);
    FreeStringArg(&szPath);
    return 1;
#endif
    lua_pushboolean(L, FALSE);
    return 1;
}

static const struct luaL_Reg tstlib [] = {
    {"cc",                 l_myTest},
   // {"CreateFile",         l_CreateFile},
   // {"CompareFiles",       l_CompareFiles},
//{"CopyDirectory",      l_CopyDirectory},
//{"MoveDirectory",      l_MoveDirectory},
  //  {"CompareDirectory",   l_CompareDirectory},
 //   {"VerifFile",          l_VerifFile},
 //   {"DeleteTree",         l_DeleteTree},
 //   {"CreateDirectory",    l_CreateDirectory},
  //  {"FileSize",           l_FileSize},
    {"GetFATInfo",         l_GetFATInfo},
  //  {"GetClusterSize",     l_GetClusterSize},
 //   {"FillDisk",           l_FillDisk},
    {"GetInfo",            l_GetInfo},
//    {"FileExists",         l_FileExists},
    {"GetDiskFreeSpace",   l_GetDiskFreeSpace},
//    {"SeekTest",           l_SeekTest},
//    {"SeekReadTest",           l_SeekReadTest},
    {"CutFile",            l_CutFile},
//    {"DeleteFile",         l_DeleteFile},
//    {"CopyFile",           l_CopyFile},
 //   {"MoveFile",           l_MoveFile},
//{"ReadFile",           l_ReadFile},
    {"LogOut",             l_LogOut},
    {"ErrOut",             l_ErrOut},
    {"GetTickCount",       l_GetTickCount},
    {"OpenLogFile",         l_OpenLogFile},
    {"CloseLogFile",        l_CloseLogFile},
    {"MemWatch",           l_MemWatch},
    //{"SkipRootPath",        l_SkipRootPath},
    {"Date",                l_Date},
   // {"GpioTest",                l_GpioTest},
  //  {"GpioStop",                l_GpioStop},
    {"Sleep",                l_Sleep},
  //{"StressCreateFile",                l_StressCreateFile},
 //   {"StressVerifFile",                l_StressVerifFile},
  //  {"SetBadTable",                l_initBadTable},
  //  {"BadTest",                l_BadTest},
  //  {"GetSectorSize",                l_GetSectorSize},
   // {"ActivateDevice",                l_ActivateDevice},
   // {"DeactivateDevice",                l_DeactivateDevice},
   // {"SDPrintCountRst",                l_SDPrintCountRst},
  //  {"SDInitCounter",                l_SDInitCounter},
    {NULL, NULL}  /* sentinel */
};

LUALIB_API int luaopen_test (lua_State *L) {
    luaL_register(L, LUA_TESTLIBNAME, tstlib);
    return 1;
}