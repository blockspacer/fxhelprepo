#include <Windows.h>
#include <stdio.h>
#include <tchar.h>

#ifdef UNDER_CE
    #include <storemgr.h>
    #include "fathlp.h"
    extern FATINFO g_sfatInfo;
    void MemWatch();
#endif


#ifndef __LEGACY_H__
#define __LEGACY_H__

#define DEF_CLUSTER_SIZE        4096

#define IS_UNC(path)  ((path[0] == _T('\\')) && (path[1] == _T('\\')))

#define INVALID_HANDLE(X)   (INVALID_HANDLE_VALUE == X || NULL == X)
#define VALID_HANDLE(X)     (INVALID_HANDLE_VALUE != X && NULL != X)

#define ERR(x)  _CCprintf(\
    _T("ERROR in %s at line %d: %s; error == 0x%08X\n"), \
    _T(__FILE__), __LINE__, _T(x), GetLastError() )
#define ERR2(x)  _CCprintf(\
    _T("ERROR in %s at line %d: %s; error == 0x%08X\n"), \
    _T(__FILE__), __LINE__, x, GetLastError() )

extern FILE* fl_Logout;
extern BOOL g_fSkipFirstDir;



//BOOL CompleteCreateDirectory(PTCHAR pszCompletePath);
//BOOL DeleteTreeRecursive(PTCHAR szPath, BOOL fDontDeleteRoot);
LPTSTR FormatValue(__int64 dwValue, LPTSTR szValue);

int _CCprintf(const PTCHAR str, ... );

//BOOL FSDT_FileExists(PTCHAR pszFileName);
//BOOL FSDT_CreateFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, DWORD dwWriteLen, __int64 i64FileLen);
//BOOL FSDT_StressCreateFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, DWORD dwWriteLen, __int64 i64FileLen);
//BOOL FSDT_VerifFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64cpFileLen);
//BOOL FSDT_StressVerifFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64cpFileLen);
//BOOL FSDT_CompareFiles(PTCHAR szSrcFileSpec, PTCHAR szDstFileSpec);
//BOOL FSDT_CopyDirectory(PTCHAR szSourcePath,PTCHAR szDestPath, BOOL bOverLay);
//BOOL FSDT_MoveDirectory(PTCHAR szSourcePath,PTCHAR szDestPath);
//BOOL FSDT_CompareDirectory(PTCHAR szSourcePath,PTCHAR szDestPath);
//BOOL FSDT_DeleteTree(PTCHAR szPath, BOOL fDontDeleteRoot);
//BOOL FSDT_SeekTest(HANDLE hFile, PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64SeekPos, DWORD OperatorLen, DWORD dwMoveMethod);
//BOOL FSDT_SeekReadTest(HANDLE hFile, PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64SeekPos, DWORD OperatorLen, DWORD dwMoveMethod);
//__int64 FSDT_FileSize(PTCHAR szFileName );
//BOOL FSDT_GetFATInfo(PTCHAR szPath);
//BOOL FSDT_FillDiskMinusXBytesWithMapping(PTCHAR szPath, PTCHAR szFileNameTmp, DWORD dwFreeSpace, DWORD *pdwFileCnt);
//__int64 FSDT_GetDiskFreeSpace(PTCHAR szPath);
//BOOL FSDT_CutFile(HANDLE hFile, __int64 i64SeekPos, DWORD dwMoveMethod);
//BOOL FSDT_ReadFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize);

//*************************************************
//      GpioTest.cpp
//*************************************************
#ifdef UNDER_CE 
typedef struct {
    volatile BOOL bTerminated;
    UINT32* ptrPins;
    UINT32  pinCnt;
    DWORD dwWaitTime;
    DWORD dwLockTime;
} GPIOTHREAD_PARAM,*LPGPIOTHREAD_PARAM;

DWORD WINAPI gpioTestThread(LPVOID lp) ;
#endif

#define DEFAULT_ERROR( TST, ACT );  \
    if( TST ) { \
    _CCprintf(  TEXT("FAIL: CONDITION( %s ) ACTION( %s )" ), \
    TEXT( #TST ), TEXT( #ACT ) ); \
    ACT; \
    }


//*************************************************
//       for bad block test
//*************************************************
#ifdef UNDER_CE 

typedef enum  _REGION_TYPE { XIP, READONLY_FILESYS, FILESYS } REGION_TYPE;

typedef struct _FlashRegion
{
    REGION_TYPE regionType;
    DWORD       dwStartPhysBlock;
    DWORD       dwNumPhysBlocks;
    DWORD       dwNumLogicalBlocks;
    DWORD       dwSectorsPerBlock;
    DWORD       dwBytesPerBlock;
    DWORD       dwCompactBlocks;

}FlashRegion, *PFlashRegion;


#define IOCTL_DISK_GET_FAL_INFO  \
    CTL_CODE(IOCTL_DISK_BASE, 0x721, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_FMD_TEST_GET_NANDINFO  \
    CTL_CODE(FILE_DEVICE_HAL, 2311, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_HAL_FMD_TEST_SET_BADLIST  \
    CTL_CODE(FILE_DEVICE_HAL, 2310, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_HAL_FMD_TEST_SET_BADTEST  \
    CTL_CODE(FILE_DEVICE_HAL, 2312, METHOD_BUFFERED, FILE_ANY_ACCESS)


#define IOCTL_SD_TEST_INITCOUNT 4
#define IOCTL_SD_TEST_PRINTCOUNT 5

#define NFC_SUPPORT_CHIPNUM     2

typedef struct _T_NAND_INFO
{
    //these are using for nand
    DWORD   Bytes;
    DWORD   Pages;
    DWORD   Blocks;
    DWORD   Planes;
    DWORD   BytesPerBlock;
    //these are using for FMD
    WORD    FalSectorsPerBlock;
    WORD    FalBytesPerSector;
    //these are using for converting fmd sector address to nand page address
    DWORD   FalSectorsPerChip;
    WORD    CvtRowAddrShift;
    BYTE      ChipCnt;                     // NandFlash count
    ULONG   ChipID[NFC_SUPPORT_CHIPNUM];
}T_NAND_INFO, *T_PNAND_INFO;

typedef enum
{
    E_NORMAL = 0,
    E_WRITE_FAILD,
    E_READ_FAILD,
    E_BAD_SPAREENABLE,
    E_LOAD_FAILD,
    E_WRITE_COUNT,
    E_READ_COUNT
}eBadBlock_type;

struct BlockStatusTable{
    unsigned short bad_type:4;
    unsigned short count:12;
} ;


BOOL GetFalInfo(PFlashRegion ptrInfo);
BOOL GetNandInfo(T_PNAND_INFO nand_info);
BOOL SetBadList(LPVOID buff, DWORD size);
BOOL SetBadTest(BOOL fBadTestOn);
BOOL Test_DeactivateDevice(LPCWSTR lpszDevKey);
BOOL Test_ActivateDevice(LPCWSTR lpszDevKey);


BOOL SDPrintCountRst(LPCWSTR lpszDevKey);
BOOL SDInitCounter(LPCWSTR lpszDevKey);

#endif

#endif