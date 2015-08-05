#include "legacy.h"


#ifdef UNDER_CE
    FATINFO g_sfatInfo;
#endif

/*
    Delete Tree
*/
FILE* fl_Logout = NULL;


int _CCprintf( const PTCHAR str, ... )
{
    
    TCHAR buffer[2048];
    va_list ArgList ;

    va_start (ArgList, str) ;
    wvsprintf(buffer, str, ArgList);

//#ifdef UNDER_CE 
//RETAILMSG(TRUE, (TEXT("%s"), buffer));
//#else
    _tprintf(TEXT("%s"), buffer);
//#endif

    if (fl_Logout != NULL)
    {
        //fl_Logout = fopen("testout.log", "w");
        _ftprintf(fl_Logout, TEXT("%s"), buffer);
        fflush(fl_Logout); 
    }
    va_end (ArgList) ;
    return (0);

}
/*
BOOL FSDT_DeleteTree(PTCHAR szPath, BOOL fDontDeleteRoot) {
    TCHAR szBuf[MAX_PATH];
    UINT32 length;


    _CCprintf(TEXT("Cleaning up \"%s\"...\n"), szPath);

    

    // Copy the path to a temp buffer.
    _tcsncpy(szBuf, szPath, MAX_PATH);
    szBuf[MAX_PATH-1]=TEXT('\0');

    // Get the length of the path.
    length = _tcslen(szBuf);

    // Ensure the path ends with a wack.
    if (szBuf[length - 1] != TEXT('\\')) {
        szBuf[length++] = TEXT('\\');
        szBuf[length] = TEXT('\0');
    }

    return DeleteTreeRecursive(szBuf, fDontDeleteRoot);
}
/*++
    Delete Tree by Recursive
    NOTE: If you don't want or you can't delete the root, pls make fDontDeleteRoot = TRUE
    by li_gangping
--*/
/*
BOOL DeleteTreeRecursive(PTCHAR szPath, BOOL fDontDeleteRoot) {

    BOOL fResult = TRUE;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA w32fd;

    // Locate the end of the path.
    PTCHAR szAppend = szPath + _tcslen(szPath);

    // Append our search spec to the path.
    _tcscpy(szAppend, _T("*"));

    // Start the file/subdirectory find.
    
    hFind = FindFirstFile(szPath, &w32fd);

    // Loop on each file/subdirectory in this directory.
    if (hFind != INVALID_HANDLE_VALUE) {
        do {

            // Check to see if the find is a directory.
            if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

                // Make sure the directory is not "." or ".."
                if (_tcscmp(w32fd.cFileName, _T(".")) && _tcscmp(w32fd.cFileName, _T(".."))) {

                    // Append the directory to our path and recurse into it.
                    _tcscat(_tcscpy(szAppend, w32fd.cFileName), _T("\\"));
                    fResult &= DeleteTreeRecursive(szPath, FALSE);
                }

                // Otherwise the find must be a file.
            } else {

                // Append the file to our path.
                _tcscpy(szAppend, w32fd.cFileName);

                // If the file is read-only, change to read/write.
                if (w32fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY) {
                    if (!SetFileAttributes(szPath, FILE_ATTRIBUTE_NORMAL)) {
                        ERR("SetFileAttributes()");
                        _CCprintf(_T("Could not set attributes: \"%s\" Last error == 0x%08X\n"), szPath, GetLastError() );
                    }
                }

                // Delete the file.
                if (!DeleteFile(szPath)) {
                    ERR("DeleteFile()");
                    _CCprintf( _T("Could not delete: \"%s\" Last error == 0x%08X\n"), szPath, GetLastError() );
                    fResult = FALSE;
                }
            }

            // Get next file/directory.
        } while (FindNextFile(hFind, &w32fd));

        // Close our find.
        FindClose(hFind);
    }

    if (!fDontDeleteRoot) {

        // Delete the directory;
        *(szAppend - 1) = _T('\0');
        if (!RemoveDirectory(szPath)) {
            ERR("RemoveDirectory()");
            _CCprintf( _T("Could not remove directory: \"%s\" Last error == 0x%08X\n"), szPath, GetLastError() );
            return FALSE;
        }
    }

    return fResult;
}

/*
    File Exists??? I am not sure.....
    You can use this function to check a directory, and it will run accurate.
    BUT NOTE: It will call FAILS on the desktop if the path == "A:" "C:"..&*(^^&%%^%&^$
    by li_gangping

    2008-05-10: Now, it's a member of FSDT. li_gangping
*/


/*++
    "Completely" create the dir, and all the dirs along its path.

     Since there is no concept of "current dir" in object store, the
     path always starts with a '\'.
     by li_gangping
--*/

BOOL g_fSkipFirstDir = TRUE;
/*
BOOL CompleteCreateDirectory(PTCHAR pszCompletePath)
{
    PTCHAR      pszPath, pszDir, pszBackSlash;
    BOOL        uStatus = TRUE;
    BOOL        fSkipFIrstDir = g_fSkipFirstDir;

    if (!pszCompletePath)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    
    pszPath = (PTCHAR)LocalAlloc(0, (_tcslen(pszCompletePath)+1)*sizeof(TCHAR));
    if (!pszPath)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    _tcscpy(pszPath, pszCompletePath);
    pszDir = pszPath;
    pszDir = _tcschr(pszDir, _T('\\'));
    for(;;)
    {
        pszBackSlash = _tcschr(pszDir+1, _T('\\'));
        if (!pszBackSlash)
            break;
        *pszBackSlash = _T('\0');
        if (fSkipFIrstDir)
        {
            if (FSDT_FileExists(pszPath))
            {
                fSkipFIrstDir=FALSE;
                *pszBackSlash = _T('\\');
                pszDir = pszBackSlash;
                continue;
            }
            else
            {
                SetLastError(ERROR_GEN_FAILURE);
                uStatus = FALSE;
                goto Exit;
            }

        }

        // The WinCE redir returns ACCESS_DENIED if dir already exists (like Win95)
        if (!CreateDirectory(pszPath, NULL) && (GetLastError() != ERROR_ALREADY_EXISTS) &&
            (!IS_UNC(pszPath) || (GetLastError() != ERROR_ACCESS_DENIED)))
        {
            ERR("CreateDirectory()");
            _CCprintf( _T("Could not create directory: \"%s\" Last error == 0x%08X\n"), pszPath, GetLastError());
            goto Exit;
        }

        *pszBackSlash = _T('\\');
        pszDir = pszBackSlash;
    }
    if (!CreateDirectory(pszPath, NULL) && GetLastError() != ERROR_ALREADY_EXISTS)
        uStatus = FALSE;
Exit:
    LocalFree(pszPath);
    return uStatus;
} 

/*++
    File System Driver Test Function
    Create special file use buffer
    NOTE: if dwWriteLen == 0, function will use dwBufferSize to replace it, and WILL NOT fflush every step
    by li_gangping
--*/

/*
BOOL FSDT_CreateFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, DWORD dwWriteLen, __int64 i64FileLen)
{

	BOOL		bReturnValue	= TRUE;
	
	HANDLE      hFile           = INVALID_HANDLE_VALUE;
	TCHAR        szFileSpec[MAX_PATH+20];
	DWORD       dwLastError      = NO_ERROR;
	DWORD       nBytesWritten;

	//Make sure the directory exists. call fails on the desktop if the path == "A:"
	if (!FSDT_FileExists(szFilePath) )
	{
		bReturnValue = (CompleteCreateDirectory(szFilePath ) == TRUE);
#ifdef UNDER_CE
		if (!bReturnValue )
		{
            ERR("CompleteCreateDirectory()");
			_CCprintf( _T("Unable to create directory \"%s\" Last error == 0x%08X\n"), szFilePath, GetLastError() );
			goto Bail;
		}

#else   //The CompleteCreateDirectory() call fails on the desktop if the path == "A:"
		bReturnValue = TRUE;
#endif
	}
    if (szFilePath[_tcslen(szFilePath)-1] == _T('\\'))
    {
        _stprintf(szFileSpec, _T("%s%s"), szFilePath, szFileName );
    }
    else
    {
        _stprintf(szFileSpec, _T("%s\\%s"), szFilePath, szFileName );
    }
	

	hFile = CreateFile(szFileSpec, GENERIC_READ | GENERIC_WRITE, 0, 
		NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	dwLastError = GetLastError();
	if (INVALID_HANDLE_VALUE == hFile )
	{
		bReturnValue = FALSE;
        ERR("CreateFile()");
		_CCprintf( _T("Unable to create file \"%s\" Last error == 0x%08X\n"),szFileSpec, dwLastError);
		goto Bail;
	}


	_CCprintf( _T("Creating file %s of size %I64u\n"), szFileSpec, i64FileLen) ;


	while (i64FileLen )
	{
		if (i64FileLen < dwBufferSize )
		{
			dwBufferSize = (DWORD)i64FileLen;
		}
        nBytesWritten = 0;
		bReturnValue = WriteFile(hFile, pTestBuffer, dwBufferSize, &nBytesWritten, NULL );

        dwLastError = GetLastError();
        if (!bReturnValue)
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("WriteFile()");
            _CCprintf( _T("WriteFile %s return false; Last error == 0x%08X\n"), szFileSpec, dwLastError);
            goto Bail;
        }

        if ((nBytesWritten != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("WriteFile()");
            _CCprintf( _T("Writing file %s error, written len %d != buffer len %d; Last error == 0x%08X\n"), szFileSpec, nBytesWritten, dwBufferSize, dwLastError);
            goto Bail;
        }

        // if write len != 0, flush buffer every step.
        if (dwWriteLen != 0 && !FlushFileBuffers(hFile))
        {
            dwLastError = GetLastError();
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBlockSize)
            ERR("FlushFileBuffers()");
            _CCprintf( _T("Could not flush buffer when writing file %s Last error == 0x%08X\n"), szFileSpec, dwLastError);
            goto Bail;         

        }

		i64FileLen -= dwBufferSize;
	}

Bail:

	if (VALID_HANDLE(hFile))
	{

		if (!CloseHandle(hFile ) )
		{
			ERR("CloseHandle()");
		}

	}
	return bReturnValue;
}


BOOL FSDT_StressCreateFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, DWORD dwWriteLen, __int64 i64FileLen)
{

    BOOL		bReturnValue	= TRUE;

    HANDLE      hFile           = INVALID_HANDLE_VALUE;
    TCHAR        szFileSpec[MAX_PATH+20];
    DWORD       dwLastError      = NO_ERROR;
    DWORD       nBytesWritten;
    DWORD       idCnt = 0;
    PBYTE       pWriterBuffer = NULL;
    PBYTE       pTmpWriterBuffer = NULL;
    __int64     i64AllBuffLen = i64FileLen;
    DWORD       dwTmpBufferSize = dwBufferSize;

    // malloc.
    pWriterBuffer = (LPBYTE)LocalAlloc(0, i64FileLen);
    pTmpWriterBuffer = pWriterBuffer;

    if (pWriterBuffer == NULL)
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }
   
    // init buffer
    while (i64AllBuffLen )
    {
        if (i64AllBuffLen < dwTmpBufferSize )
        {
            dwTmpBufferSize = (DWORD)i64AllBuffLen;
        }
        memcpy(pTmpWriterBuffer, pTestBuffer, dwTmpBufferSize);

        pTmpWriterBuffer += dwTmpBufferSize;
        i64AllBuffLen -= dwTmpBufferSize;
    }

    // add CAP in buffer
    pTmpWriterBuffer = pWriterBuffer;
    i64AllBuffLen = i64FileLen;
    dwTmpBufferSize = 2048;
    
    while (i64AllBuffLen )
    {
        if (i64AllBuffLen < dwTmpBufferSize )
        {
            dwTmpBufferSize = (DWORD)i64AllBuffLen;
        }
        if ((_tcslen(szFileName)+sizeof(DWORD)) < dwTmpBufferSize )
        {
            _tcsncpy(pTmpWriterBuffer+sizeof(DWORD), szFileName, _tcslen(szFileName));
        }


        *(DWORD*)pTmpWriterBuffer = idCnt++;
        pTmpWriterBuffer += dwTmpBufferSize;
        i64AllBuffLen -= dwTmpBufferSize;
    }


    //Make sure the directory exists. call fails on the desktop if the path == "A:"
    if (!FSDT_FileExists(szFilePath) )
    {
        bReturnValue = (CompleteCreateDirectory(szFilePath ) == TRUE);
#ifdef UNDER_CE
        if (!bReturnValue )
        {
            ERR("CompleteCreateDirectory()");
            _CCprintf( _T("Unable to create directory \"%s\" Last error == 0x%08X\n"), szFilePath, GetLastError() );
            goto Bail;
        }

#else   //The CompleteCreateDirectory() call fails on the desktop if the path == "A:"
        bReturnValue = TRUE;
#endif
    }
    if (szFilePath[_tcslen(szFilePath)-1] == _T('\\'))
    {
        _stprintf(szFileSpec, _T("%s%s"), szFilePath, szFileName );
    }
    else
    {
        _stprintf(szFileSpec, _T("%s\\%s"), szFilePath, szFileName );
    }

    // _tcsncpy(pTestBuffer+sizeof(DWORD), szFileName, _tcslen(szFileName));


    hFile = CreateFile(szFileSpec, GENERIC_READ | GENERIC_WRITE, 0, 
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    dwLastError = GetLastError();
    if (INVALID_HANDLE_VALUE == hFile )
    {
        bReturnValue = FALSE;
        ERR("CreateFile()");
        _CCprintf( _T("Unable to create file \"%s\" Last error == 0x%08X\n"),szFileSpec, dwLastError);
        goto Bail;
    }


    _CCprintf( _T("Creating file %s of size %I64u\n"), szFileSpec, i64FileLen) ;

    pTmpWriterBuffer = pWriterBuffer;
    while (i64FileLen )
    {
        if (i64FileLen < dwBufferSize )
        {
            dwBufferSize = (DWORD)i64FileLen;
        }
        bReturnValue = WriteFile(hFile, pTmpWriterBuffer, dwBufferSize, &nBytesWritten, NULL );


        dwLastError = GetLastError();
        if (!bReturnValue)
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("WriteFile()");
            _CCprintf( _T("WriteFile %s return false; Last error == 0x%08X\n"), szFileSpec, dwLastError);
            goto Bail;
        }

        if ((nBytesWritten != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("WriteFile()");
            _CCprintf( _T("Writing file %s error, written len %d != buffer len %d; Last error == 0x%08X\n"), szFileSpec, nBytesWritten, dwBufferSize, dwLastError);
            goto Bail;
        }

        // if write len != 0, flush buffer every step.
        if (dwWriteLen != 0 && !FlushFileBuffers(hFile))
        {
            dwLastError = GetLastError();
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBlockSize)
            ERR("FlushFileBuffers()");
            _CCprintf( _T("Could not flush buffer when writing file %s Last error == 0x%08X\n"), szFileSpec, dwLastError);
            goto Bail;         

        }

        i64FileLen -= dwBufferSize;
        pTmpWriterBuffer+= dwBufferSize;
    }

Bail:

    if (VALID_HANDLE(hFile))
    {
        if (!CloseHandle(hFile ) )
        {
            ERR("CloseHandle()");
        }
    }

    if (pWriterBuffer != NULL)
    {
        LocalFree(pWriterBuffer);
    }
    return bReturnValue;
}

/*++
    File System Driver Test Function
    Verif  File use buffer
    by li_gangping
--*/
/*
BOOL FSDT_VerifFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64cpFileLen)
{
	BOOL		bReturnValue	= TRUE;
	HANDLE      hFile           = INVALID_HANDLE_VALUE;
	TCHAR        szFileSpec[MAX_PATH+20];
	DWORD       dwLastError      = NO_ERROR;
	DWORD       nBytesRead;


	LARGE_INTEGER		i64FileLen;
	PBYTE		pBufferRead       = NULL;

	pBufferRead = (PBYTE)LocalAlloc( 0, dwBufferSize );
	if ( pBufferRead == NULL)
	{
		bReturnValue = FALSE;
		ERR("LocalAlloc()");
		goto Bail;
	}


	//Make sure the directory exists. call fails on the desktop if the path == "A:"
	if (!FSDT_FileExists(szFilePath) )
	{
        _CCprintf( _T("ERROR: FilePath %s is NOT exists!\n"), szFilePath);
		bReturnValue = FALSE;
		goto Bail;
	}
    if (szFilePath[_tcslen(szFilePath)-1] == _T('\\'))
    {
        _stprintf(szFileSpec, _T("%s%s"), szFilePath, szFileName );
    }
    else
    {
        _stprintf(szFileSpec, _T("%s\\%s"), szFilePath, szFileName );
    }


	if (!FSDT_FileExists(szFileSpec) )
	{
        _CCprintf( _T("ERROR: File %s is NOT exists!\n"), szFileSpec);
		bReturnValue = FALSE;
		goto Bail;
	}


	hFile = CreateFile(szFileSpec, GENERIC_READ | GENERIC_WRITE, 0, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	dwLastError = GetLastError();
	if (INVALID_HANDLE_VALUE == hFile )
	{
		bReturnValue = FALSE;
        ERR("CreateFile()");
		_CCprintf( _T("Unable to open file \"%s\" Last error == 0x%08X\n"),  szFileSpec, dwLastError);
		goto Bail;
	}
	i64FileLen.LowPart = GetFileSize(hFile, &i64FileLen.HighPart);

	_CCprintf( _T("Verif file %s of size %I64u\n"), szFileSpec, i64FileLen.QuadPart) ;
    if (i64cpFileLen != i64FileLen.QuadPart)
    {
        _CCprintf( _T("FileLen %d(org) != %d"),  i64cpFileLen , i64FileLen.QuadPart);
        bReturnValue = FALSE;
    }


	while (i64FileLen.QuadPart )
	{
		if (i64FileLen.QuadPart < dwBufferSize )
		{
			dwBufferSize = (DWORD)i64FileLen.QuadPart;
		}


		//Read the data and verify that it is correct.
		bReturnValue = ReadFile( hFile, pBufferRead, dwBufferSize, &nBytesRead, NULL );
        dwLastError = GetLastError();
		if ( !bReturnValue)
		{
			bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("ReadFile()");
			_CCprintf( _T("Reading file %s error,  Last error == 0x%08X\n"),  szFileSpec, dwLastError);
			goto Bail;
		}

        if ( (nBytesRead != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("ReadFile()");
            _CCprintf( _T("Reading file %s error,  nBytesRead != dwBufferSize Last error == 0x%08X\n"),  szFileSpec, dwLastError);
            goto Bail;
        }

		if ( memcmp(pBufferRead, pTestBuffer, dwBufferSize) != 0 )
		{
			bReturnValue = FALSE;
            _CCprintf( _T("ERROR: File %s verif miscompare!\n"), szFileSpec);
			goto Bail;
		}

		i64FileLen.QuadPart -= dwBufferSize;
	}

Bail:

	if (VALID_HANDLE(hFile))
	{

		if (!CloseHandle(hFile ) )
		{
			ERR("CloseHandle()");
		}

	}
	if ( pBufferRead )
	{
		LocalFree( pBufferRead );
	}
	return bReturnValue;

}

BOOL FSDT_StressVerifFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64cpFileLen)
{
    BOOL		bReturnValue	= TRUE;
    HANDLE      hFile           = INVALID_HANDLE_VALUE;
    TCHAR        szFileSpec[MAX_PATH+20];
    DWORD       dwLastError      = NO_ERROR;
    DWORD       nBytesRead;
    LARGE_INTEGER		i64FileLen;
    PBYTE		pBufferRead       = NULL;
    DWORD       idCnt = 0;
    PBYTE       pAllReadBuffer = NULL;
    PBYTE       pTmpAllReadBuffer = NULL;
    __int64     i64AllBuffLen = i64cpFileLen;
    DWORD       dwTmpBufferSize = dwBufferSize;


    pBufferRead = (PBYTE)LocalAlloc( 0, dwBufferSize );
    if ( pBufferRead == NULL)
    {
        bReturnValue = FALSE;
        ERR("LocalAlloc()");
        goto Bail;
    }


    // malloc.
    pAllReadBuffer = (LPBYTE)LocalAlloc(0, i64cpFileLen);
    pTmpAllReadBuffer = pAllReadBuffer;

    if (pAllReadBuffer == NULL)
    {
        ERR("LocalAlloc()");
        bReturnValue = FALSE;
        goto Bail;
    }

    // init buffer
    while (i64AllBuffLen )
    {
        if (i64AllBuffLen < dwTmpBufferSize )
        {
            dwTmpBufferSize = (DWORD)i64AllBuffLen;
        }
        memcpy(pTmpAllReadBuffer, pTestBuffer, dwTmpBufferSize);

        pTmpAllReadBuffer += dwTmpBufferSize;
        i64AllBuffLen -= dwTmpBufferSize;
    }

    // add CAP in buffer
    pTmpAllReadBuffer = pAllReadBuffer;
    i64AllBuffLen = i64cpFileLen;
    dwTmpBufferSize = 2048;

    while (i64AllBuffLen )
    {
        if (i64AllBuffLen < dwTmpBufferSize )
        {
            dwTmpBufferSize = (DWORD)i64AllBuffLen;
        }
        _tcsncpy(pTmpAllReadBuffer+sizeof(DWORD), szFileName, _tcslen(szFileName));
        *(DWORD*)pTmpAllReadBuffer = idCnt++;
        pTmpAllReadBuffer += dwTmpBufferSize;
        i64AllBuffLen -= dwTmpBufferSize;
    }


    //Make sure the directory exists. call fails on the desktop if the path == "A:"
    if (!FSDT_FileExists(szFilePath) )
    {
        _CCprintf( _T("ERROR: FilePath %s is NOT exists!\n"), szFilePath);
        bReturnValue = FALSE;
        goto Bail;
    }
    if (szFilePath[_tcslen(szFilePath)-1] == _T('\\'))
    {
        _stprintf(szFileSpec, _T("%s%s"), szFilePath, szFileName );
    }
    else
    {
        _stprintf(szFileSpec, _T("%s\\%s"), szFilePath, szFileName );
    }


    if (!FSDT_FileExists(szFileSpec) )
    {
        _CCprintf( _T("ERROR: File %s is NOT exists!\n"), szFileSpec);
        bReturnValue = FALSE;
        goto Bail;
    }


    hFile = CreateFile(szFileSpec, GENERIC_READ | GENERIC_WRITE, 0, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    dwLastError = GetLastError();
    if (INVALID_HANDLE_VALUE == hFile )
    {
        bReturnValue = FALSE;
        ERR("CreateFile()");
        _CCprintf( _T("Unable to open file \"%s\" Last error == 0x%08X\n"),  szFileSpec, dwLastError);
        goto Bail;
    }
    i64FileLen.LowPart = GetFileSize(hFile, &i64FileLen.HighPart);

    _CCprintf( _T("Verif file %s of size %I64u\n"), szFileSpec, i64FileLen.QuadPart) ;
    if (i64cpFileLen != i64FileLen.QuadPart)
    {
        _CCprintf( _T("FileLen %d(org) != %d"),  i64cpFileLen , i64FileLen.QuadPart);
        bReturnValue = FALSE;
    }

    pTmpAllReadBuffer = pAllReadBuffer;
    while (i64FileLen.QuadPart )
    {
        if (i64FileLen.QuadPart < dwBufferSize )
        {
            dwBufferSize = (DWORD)i64FileLen.QuadPart;
        }


        //Read the data and verify that it is correct.
        bReturnValue = ReadFile( hFile, pBufferRead, dwBufferSize, &nBytesRead, NULL );
        dwLastError = GetLastError();
        if ( !bReturnValue)
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("ReadFile()");
            _CCprintf( _T("Reading file %s error,  Last error == 0x%08X\n"),  szFileSpec, dwLastError);
            goto Bail;
        }

        if ( (nBytesRead != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("ReadFile()");
            _CCprintf( _T("Reading file %s error, nBytesRead != dwBufferSize Last error == 0x%08X\n"),  szFileSpec, dwLastError);
            goto Bail;
        }

        if ( memcmp(pBufferRead, pTmpAllReadBuffer, dwBufferSize) != 0 )
        {
            bReturnValue = FALSE;
            _CCprintf( _T("ERROR: File %s verif miscompare!\n"), szFileSpec);
            goto Bail;
        }

        i64FileLen.QuadPart -= dwBufferSize;
        pTmpAllReadBuffer += dwBufferSize;
    }

Bail:

    if (VALID_HANDLE(hFile))
    {

        if (!CloseHandle(hFile ) )
        {
            ERR("CloseHandle()");
        }

    }
    if ( pBufferRead )
    {
        LocalFree( pBufferRead );
    }
    if (pAllReadBuffer)
    {
        LocalFree(pAllReadBuffer);
    }
    return bReturnValue;
    
}

BOOL FSDT_ReadFile(PTCHAR szFilePath, PTCHAR szFileName,PBYTE pTestBuffer, DWORD dwBufferSize)
{
    BOOL		bReturnValue	= TRUE;
    HANDLE      hFile           = INVALID_HANDLE_VALUE;
    TCHAR        szFileSpec[MAX_PATH+20];
    DWORD       dwLastError      = NO_ERROR;
    DWORD       nBytesRead;


    LARGE_INTEGER		i64FileLen;
    PBYTE		pBufferRead       = NULL;

    pBufferRead = (PBYTE)LocalAlloc( 0, dwBufferSize );
    if ( pBufferRead == NULL)
    {
        bReturnValue = FALSE;
        ERR("LocalAlloc()");
        goto Bail;
    }


    //Make sure the directory exists. call fails on the desktop if the path == "A:"
    if (!FSDT_FileExists(szFilePath) )
    {
        _CCprintf( _T("ERROR: FilePath %s is NOT exists!\n"), szFilePath);
        bReturnValue = FALSE;
        goto Bail;
    }
    if (szFilePath[_tcslen(szFilePath)-1] == _T('\\'))
    {
        _stprintf(szFileSpec, _T("%s%s"), szFilePath, szFileName );
    }
    else
    {
        _stprintf(szFileSpec, _T("%s\\%s"), szFilePath, szFileName );
    }


    if (!FSDT_FileExists(szFileSpec) )
    {
        _CCprintf( _T("ERROR: File %s is NOT exists!\n"), szFileSpec);
        bReturnValue = FALSE;
        goto Bail;
    }


    hFile = CreateFile(szFileSpec, GENERIC_READ | GENERIC_WRITE, 0, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    dwLastError = GetLastError();
    if (INVALID_HANDLE_VALUE == hFile )
    {
        bReturnValue = FALSE;
        ERR("CreateFile()");
        _CCprintf( _T("Unable to open file \"%s\" Last error == 0x%08X\n"),  szFileSpec, dwLastError);
        goto Bail;
    }
    i64FileLen.LowPart = GetFileSize(hFile, &i64FileLen.HighPart);

    _CCprintf( _T("Verif file %s of size %I64u\n"), szFileSpec, i64FileLen.QuadPart) ;

    

    while (i64FileLen.QuadPart )
    {
        if (i64FileLen.QuadPart < dwBufferSize )
        {
            dwBufferSize = (DWORD)i64FileLen.QuadPart;
        }


        //Read the data and verify that it is correct.
        bReturnValue = ReadFile( hFile, pBufferRead, dwBufferSize, &nBytesRead, NULL );
        if ( !bReturnValue || (nBytesRead != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            ERR("ReadFile()");
            _CCprintf( _T("Reading file %s error,  Last error == 0x%08X\n"),  szFileSpec, dwLastError);
            goto Bail;
        }

        i64FileLen.QuadPart -= dwBufferSize;
    }

Bail:

    if (VALID_HANDLE(hFile))
    {

        if (!CloseHandle(hFile ) )
        {
            ERR("CloseHandle()");
        }

    }
    if ( pBufferRead )
    {
        LocalFree( pBufferRead );
    }
    return bReturnValue;

}
/*++
    File System Driver Test Function
    Compare 2 Files
    by li_gangping
--*/
/*
BOOL FSDT_CompareFiles(PTCHAR szSrcFileSpec, PTCHAR szDstFileSpec)
{
	BOOL		bReturnValue	= TRUE;

	char    *pBuf1          = NULL;
	char    *pBuf2          = NULL;
	HANDLE  hFile1          = INVALID_HANDLE_VALUE;
	HANDLE  hFile2          = INVALID_HANDLE_VALUE;

	DWORD   dwBufSize       = 4096; //Use 4k buffer be           cause ObjStore uses 4k byte allocation units.
	LARGE_INTEGER   i64FileSize1;
    LARGE_INTEGER   i64FileSize2;
	DWORD   dwBytesRead1;
	DWORD   dwBytesRead2;
	__int64   i64BytesLeftToCompare;
	DWORD   dwBytesToCompare;

	pBuf1 = (char *)LocalAlloc( 0, dwBufSize );
	pBuf2 = (char *)LocalAlloc( 0, dwBufSize );

	if ( (pBuf1 == NULL) || (pBuf2 == NULL) )
	{
		bReturnValue = FALSE;
		ERR("LocalAlloc()");
		goto Bail;
	}

	//Attempt to open the files for read.
	hFile1 = CreateFile( szSrcFileSpec, GENERIC_READ, 0, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	hFile2 = CreateFile( szDstFileSpec, GENERIC_READ, 0, 
		NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if ( INVALID_HANDLE_VALUE == hFile1)
	{
		bReturnValue = FALSE;
		ERR("CreateFile()");
        _CCprintf( _T("Could not open file: %s Last error == 0x%08X\n"), szSrcFileSpec, GetLastError());
		goto Bail;
	}
    if ( INVALID_HANDLE_VALUE == hFile2 )
    {
        bReturnValue = FALSE;
        ERR("CreateFile() 2");
        _CCprintf( _T("Could not open file: %s Last error == 0x%08X\n"), szDstFileSpec, GetLastError());
        goto Bail;
    }
	//Reuse dwBytesToCompare below to save memory!
	i64FileSize1.LowPart = GetFileSize(hFile1, &i64FileSize1.HighPart);
    i64FileSize2.LowPart = GetFileSize(hFile2, &i64FileSize2.HighPart);
	if ( i64FileSize1.QuadPart != i64FileSize2.QuadPart )
	{
		bReturnValue = FALSE;
        _CCprintf( _T("ERROR: File size mismatch. CompareFiles()\n") );
		_CCprintf( _T("Src: %s\n"), szSrcFileSpec);
		_CCprintf( _T("Dst: %s\n"), szDstFileSpec);
		goto Bail;
	}

	i64BytesLeftToCompare = i64FileSize1.QuadPart;
	while ( i64BytesLeftToCompare )
	{
		if ( i64BytesLeftToCompare >= dwBufSize )
			dwBytesToCompare = dwBufSize;
		else
			dwBytesToCompare = (DWORD)i64BytesLeftToCompare;

		//Read the data and verify that it is correct.
		bReturnValue = ReadFile( hFile1, pBuf1, dwBytesToCompare, &dwBytesRead1, NULL );
		if ( !bReturnValue || (dwBytesRead1 != dwBytesToCompare) )
		{
			bReturnValue = FALSE;   //In case (dwBytesRead1 != dwBytesToCompare)
			ERR("ReadFile()");
            _CCprintf( _T("Could not open file: %s, Last error == 0x%08X\n"), szSrcFileSpec, GetLastError());
			goto Bail;
		}
		bReturnValue = ReadFile( hFile2, pBuf2, dwBytesToCompare, &dwBytesRead2, NULL );
		if ( !bReturnValue || (dwBytesRead2 != dwBytesToCompare) )
		{
			bReturnValue = FALSE;   //In case (dwBytesRead2 != dwBytesToCompare)
			ERR("ReadFile()");
            _CCprintf( _T("Could not open file: %s, Last error == 0x%08X\n"), szDstFileSpec, GetLastError());
			goto Bail;
		}

		if ( memcmp(pBuf1, pBuf2, dwBytesToCompare) != 0 )
		{
			bReturnValue = FALSE;
            _CCprintf( _T("ERROR: file miscompare!\n") );
            _CCprintf( _T("Src: %s\n"), szSrcFileSpec);
            _CCprintf( _T("Dst: %s\n"), szDstFileSpec);
			goto Bail;
		}

		i64BytesLeftToCompare -= dwBytesToCompare;
	}
Bail:

	if ( VALID_HANDLE(hFile1))
	{
		CloseHandle( hFile1 );
	}

	if ( VALID_HANDLE(hFile2))
	{
		CloseHandle( hFile2 );
	}

	if ( pBuf1 )
	{
		LocalFree( pBuf1 );
	}

	if ( pBuf2 )
	{
		LocalFree( pBuf2 );
	}

	return bReturnValue;
}
/*++
    File System Driver Test Function
    Copy dir
    by li_gangping
--*/
/*
BOOL FSDT_CopyDirectory(PTCHAR szSourcePath,PTCHAR szDestPath, BOOL bOverLay)
{
	BOOL		bReturnValue	= TRUE;

	TCHAR			szSourcePathAppend[MAX_PATH];
	TCHAR           szSrcFileSpec[MAX_PATH+20];
	TCHAR           szDstFileSpec[MAX_PATH+20];

	WIN32_FIND_DATA w32fd;
	HANDLE 			hFind = INVALID_HANDLE_VALUE;

	//Make sure the directory exists!
	if (!FSDT_FileExists(szDestPath) )
	{
		bReturnValue = (CompleteCreateDirectory(szDestPath ) == TRUE);
	}

	// Append our search spec to the path.
	_stprintf(szSourcePathAppend,_T("%s\\*.*"),szSourcePath);

	// Start the file/subdirectory find.
	hFind = FindFirstFile(szSourcePathAppend, &w32fd);

	if (INVALID_HANDLE(hFind)) {
		ERR("FindFirstFile()");
		bReturnValue = FALSE;
		goto Bail; 
	}


	do{
		if((_tcscmp(w32fd.cFileName,_T(".")) ==0) 
			||(_tcscmp(w32fd.cFileName,_T("..")) == 0))
		{
			continue;
		}

		_stprintf(szSrcFileSpec,_T("%s\\%s"), szSourcePath, w32fd.cFileName);
		_stprintf(szDstFileSpec,_T("%s\\%s"), szDestPath, w32fd.cFileName);

		// if not dir
		if(!(w32fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{


            _CCprintf( TEXT("Copy \"%s\"...to...\"%s\"\n"), szSrcFileSpec, szDstFileSpec);

            
			if(!CopyFile(szSrcFileSpec,szDstFileSpec, bOverLay))
			{
				if (!(TRUE == bOverLay && GetLastError() == ERROR_FILE_EXISTS) )
				{
					ERR("CopyFile()");
                    _CCprintf( _T("Could not copy file: %s to %s Last error == 0x%08X\n"), szSrcFileSpec, szDstFileSpec, GetLastError());
					bReturnValue = FALSE;
					goto Bail;
				}
			}
		}
		else
		{
			bReturnValue &= FSDT_CopyDirectory(szSrcFileSpec,szDstFileSpec,bOverLay);
			if (!bReturnValue)
			{
				goto Bail;
			}
		}
	}while(FindNextFile(hFind, &w32fd));

Bail:
	if (VALID_HANDLE(hFind))
	{
		FindClose(hFind);
	}
	return bReturnValue;
}

/*++
    File System Driver Test Function
    Move dir
    by li_gangping
--*/
/*
BOOL FSDT_MoveDirectory(PTCHAR szSourcePath,PTCHAR szDestPath)
{
	BOOL		bReturnValue	= TRUE;

	TCHAR			szSourcePathAppend[MAX_PATH];
	TCHAR           szSrcFileSpec[MAX_PATH+20];
	TCHAR           szDstFileSpec[MAX_PATH+20];

	WIN32_FIND_DATA w32fd;
	HANDLE 			hFind = INVALID_HANDLE_VALUE;

	//Make sure the directory exists!
	if (!FSDT_FileExists(szDestPath) )
	{
		bReturnValue = (CompleteCreateDirectory(szDestPath ) == TRUE);
	}

	// Append our search spec to the path.
	_stprintf(szSourcePathAppend,_T("%s\\*.*"),szSourcePath);

	// Start the file/subdirectory find.
	hFind = FindFirstFile(szSourcePathAppend, &w32fd);

	if (INVALID_HANDLE(hFind)) {
		ERR("FindFirstFile()");
		bReturnValue = FALSE;
		goto Bail; 
	}


	do{
		if((_tcscmp(w32fd.cFileName,_T(".")) ==0) 
			||(_tcscmp(w32fd.cFileName,_T("..")) == 0))
		{
			continue;
		}

		_stprintf(szSrcFileSpec,_T("%s\\%s"), szSourcePath, w32fd.cFileName);
		_stprintf(szDstFileSpec,_T("%s\\%s"), szDestPath, w32fd.cFileName);

		// if not dir
		if(!(w32fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{


            _CCprintf( TEXT("Move \"%s\"...to...\"%s\"\n"), szSrcFileSpec, szDstFileSpec);
            
			if(!MoveFile(szSrcFileSpec,szDstFileSpec))
			{
				ERR("MoveFile()");
                _CCprintf( _T("Could not move file: %s to %s\n"), szSrcFileSpec, szDstFileSpec, GetLastError());
				bReturnValue = FALSE;
				goto Bail;
			}
		}
		else
		{
			bReturnValue &= FSDT_MoveDirectory(szSrcFileSpec,szDstFileSpec);
			if (!bReturnValue)
			{
				goto Bail;
			}
		}
	}while(FindNextFile(hFind, &w32fd));
	if (VALID_HANDLE(hFind))
	{
		FindClose(hFind);

		//delete szSourcePath
		if (!RemoveDirectory(szSourcePath))
		{
			bReturnValue = FALSE;
			ERR("RemoveDirectory()");
            _CCprintf( _T("Could not remove directory: %s Last error == 0x%08X\n"), szSourcePath, GetLastError());
			goto Bail;
		}
	}


Bail:
	if (VALID_HANDLE(hFind))
	{
		FindClose(hFind);
	}

	return bReturnValue;
}

/*++
    File System Driver Test Function
    Compare 2 directory
    by li_gangping
--*/
/*
BOOL FSDT_CompareDirectory(PTCHAR szSourcePath,PTCHAR szDestPath)
{
	BOOL		bReturnValue	= TRUE;

	TCHAR			szSourcePathAppend[MAX_PATH];
	TCHAR           szSrcFileSpec[MAX_PATH+20];
	TCHAR           szDstFileSpec[MAX_PATH+20];

	WIN32_FIND_DATA w32fd;
	HANDLE 			hFind = INVALID_HANDLE_VALUE;

	//Make sure the directory exists!
	if (!FSDT_FileExists(szSourcePath) )
	{
        _CCprintf( _T("ERROR: DestPath %s is NOT exists!\n"), szSourcePath);
		bReturnValue = FALSE;
		goto Bail;
	}
	if (!FSDT_FileExists(szDestPath) )
	{
        _CCprintf( _T("ERROR: DestPath %s is NOT exists!\n"), szDestPath);
		bReturnValue = FALSE;
		goto Bail;
	}

	// Append our search spec to the path.
	_stprintf(szSourcePathAppend,_T("%s\\*.*"),szSourcePath);

	// Start the file/subdirectory find.
	hFind = FindFirstFile(szSourcePathAppend, &w32fd);

	if (INVALID_HANDLE(hFind)) {
		ERR("FindFirstFile()");
		bReturnValue = FALSE;
		goto Bail; 
	}


	do{
		if((_tcscmp(w32fd.cFileName,_T(".")) ==0) 
			||(_tcscmp(w32fd.cFileName,_T("..")) == 0))
		{
			continue;
		}

		_stprintf(szSrcFileSpec,_T("%s\\%s"), szSourcePath, w32fd.cFileName);
		_stprintf(szDstFileSpec,_T("%s\\%s"), szDestPath, w32fd.cFileName);

		// if not dir
		if(!(w32fd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY))
		{
			if(!FSDT_CompareFiles(szSrcFileSpec,szDstFileSpec))
			{
                _CCprintf( _T("ERROR: Files mismatch\n"));
				bReturnValue = FALSE;
				goto Bail;
			}
		}
		else
		{
			bReturnValue &= FSDT_CompareDirectory(szSrcFileSpec,szDstFileSpec);
			if (!bReturnValue)
			{
				goto Bail;
			}
		}
	}while(FindNextFile(hFind, &w32fd));


Bail:
	if (VALID_HANDLE(hFind))
	{
		FindClose(hFind);
	}

	return bReturnValue;
}

/*++
    File System Driver Test Function
    Return the special file's size
    by li_gangping
--*/
/*
__int64 FSDT_FileSize(PTCHAR szFileName )
{
    LARGE_INTEGER      i64FileSize;
    HANDLE      hFile=INVALID_HANDLE_VALUE ;
    DWORD       dwLastError=0;

    hFile = CreateFile(szFileName, 0, 0, 
        NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    dwLastError = GetLastError();

    if (INVALID_HANDLE_VALUE == hFile )
    {
        ERR("CreateFile()");
        _CCprintf( _T("Unable to open file %s Last error == 0x%08X\n"), szFileName, dwLastError );
        return 0;
    }
    else
    {
        i64FileSize.LowPart = GetFileSize(hFile, &i64FileSize.HighPart );
        CloseHandle(hFile );
    }

    return i64FileSize.QuadPart;
}

/*++
    use to str output: 1000-> 1,000
    by li_gangping
--*/
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

/*++
    use by FSDT_GetFATInfo. to display infomation.
    by li_gangping
--*/

#ifdef UNDER_CE
VOID DisplayFATInfo(PFATINFO pFatInfo)
{
    TCHAR szValue[32];
    DWORD dwClusterSize = pFatInfo->dwBytesPerSector * pFatInfo->dwSectorsPerCluster;

    _CCprintf( TEXT("\n"));
    _CCprintf( TEXT("FAT Format Information:\n"));
    _CCprintf( TEXT("  Total Bytes:     %12s\n"), FormatValue((__int64)pFatInfo->dwTotalSectors * pFatInfo->dwBytesPerSector, szValue));
    _CCprintf( TEXT("  Usable Bytes:    %12s\n"), FormatValue((__int64)pFatInfo->dwUsableClusters * dwClusterSize, szValue));
    _CCprintf( TEXT("  Used Bytes:      %12s\n"), FormatValue((__int64)(pFatInfo->dwUsableClusters - pFatInfo->dwFreeClusters) * dwClusterSize, szValue));
    _CCprintf( TEXT("  Free Bytes:      %12s\n"), FormatValue((__int64)pFatInfo->dwFreeClusters * dwClusterSize, szValue));
    _CCprintf( TEXT("  Media Descriptor:        0x%02X\n"), pFatInfo->dwMediaDescriptor);
    _CCprintf( TEXT("  Sector Size:     %12s\n"), FormatValue(pFatInfo->dwBytesPerSector, szValue));
    _CCprintf( TEXT("  Sectors/Cluster: %12s\n"), FormatValue(pFatInfo->dwSectorsPerCluster, szValue));
    _CCprintf( TEXT("  Cluster Size:    %12s\n"), FormatValue(dwClusterSize, szValue));
    _CCprintf( TEXT("  Total Sectors:   %12s\n"), FormatValue(pFatInfo->dwTotalSectors, szValue));
    _CCprintf( TEXT("  Reserved Sectors:%12s\n"), FormatValue(pFatInfo->dwReservedSecters, szValue));
    _CCprintf( TEXT("  Hidden Sectors:  %12s\n"), FormatValue(pFatInfo->dwHiddenSectors, szValue));
    _CCprintf( TEXT("  Sectors/Track:   %12s\n"), FormatValue(pFatInfo->dwSectorsPerTrack, szValue));
    _CCprintf( TEXT("  Number of Heads: %12s\n"), FormatValue(pFatInfo->dwHeads, szValue));
    _CCprintf( TEXT("  Sectors/FAT:     %12s\n"), FormatValue(pFatInfo->dwSectorsPerFAT, szValue));
    _CCprintf( TEXT("  Number of FATs:  %12s\n"), FormatValue(pFatInfo->dwNumberOfFATs, szValue));
    _CCprintf( TEXT("  Root Entries:    %12s\n"), FormatValue(pFatInfo->dwRootEntries, szValue));
    _CCprintf( TEXT("  Usable Clusters: %12s\n"), FormatValue(pFatInfo->dwUsableClusters, szValue));
    _CCprintf( TEXT("  Used Clusters:   %12s\n"), FormatValue(pFatInfo->dwUsableClusters - pFatInfo->dwFreeClusters, szValue));
    _CCprintf( TEXT("  Free Clusters:   %12s\n"), FormatValue(pFatInfo->dwFreeClusters, szValue));
    _CCprintf( TEXT("\n"));
}
#endif

/*++
    File System Driver Test Function
    Now, you can use GetFATInfo to dump the fat infomation. It can help you check what happen when case fail.
    by li_gangping
--*/
BOOL FSDT_GetFATInfo(PTCHAR szPath)
{
#ifdef UNDER_CE
    BOOL bReturnValue = TRUE;

    DEVPB volpb = {0};
    TCHAR szVol[MAX_PATH];
    HANDLE hVol;
    DWORD cb = 0;
    FREEREQ fr = {0};
    DWORD dwNumClusters, dwStartDataSector, dwDataSectors, dwRootSectors;

    _stprintf(szVol, _T("%s\\VOL:"), szPath);
    
    hVol = CreateFile(szVol, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    if(INVALID_HANDLE(hVol))
    {
        bReturnValue = FALSE;

        ERR("CreateFile()");
        _CCprintf( _T("Unable to open disk: %s Last error == 0x%08X\n"), szVol, GetLastError());
        goto EXIT;
    }

    _CCprintf(_T("Getting info of vol :%s \n"),szVol);

    if(!DeviceIoControl(hVol, IOCTL_DISK_GET_VOLUME_PARAMETERS, NULL, 0, &volpb, 
        sizeof(volpb), &cb, NULL) || cb != sizeof(volpb))
    {
        bReturnValue = FALSE;
        ERR("DeviceIoControl(IOCTL_DISK_GET_VOLUME_PARAMETERS)");
        goto EXIT;
    }
    else
    {
        g_sfatInfo.dwMediaDescriptor   = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_MediaDescriptor;
        g_sfatInfo.dwBytesPerSector    = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_BytesPerSector;
        g_sfatInfo.dwSectorsPerCluster = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_SectorsPerCluster;
        g_sfatInfo.dwTotalSectors      = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_TotalSectors ?
            (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_TotalSectors :
        (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_BigTotalSectors;
        g_sfatInfo.dwReservedSecters   = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_ReservedSectors;
        g_sfatInfo.dwHiddenSectors     = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_HiddenSectors;
        g_sfatInfo.dwSectorsPerTrack   = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_SectorsPerTrack;
        g_sfatInfo.dwHeads             = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_Heads;
        g_sfatInfo.dwSectorsPerFAT     = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_SectorsPerFAT;
        g_sfatInfo.dwNumberOfFATs      = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_NumberOfFATs;
        g_sfatInfo.dwRootEntries       = (DWORD)volpb.DevPrm.ExtDevPB.BPB.oldBPB.BPB_RootEntries;

        // calculate the FAT version by determining the total count of clusters
        //DWORD dwNumClusters, dwStartDataSector, dwDataSectors, dwRootSectors;
        dwRootSectors = (32 * g_sfatInfo.dwRootEntries + g_sfatInfo.dwBytesPerSector - 1) / g_sfatInfo.dwBytesPerSector;
        dwStartDataSector = g_sfatInfo.dwReservedSecters + dwRootSectors + g_sfatInfo.dwNumberOfFATs * g_sfatInfo.dwSectorsPerFAT;
        dwDataSectors = g_sfatInfo.dwTotalSectors - dwStartDataSector;
        dwNumClusters = dwDataSectors / g_sfatInfo.dwSectorsPerCluster;

        if(dwNumClusters >= 65525)
        {
            // FAT32
            g_sfatInfo.dwFATVersion = 32;
        } 
        else if(dwNumClusters >= 4085)
        {
            // FAT16
            g_sfatInfo.dwFATVersion = 16;
        }   
        else
        {
            // FAT12
            g_sfatInfo.dwFATVersion = 12;
        }

        if(!DeviceIoControl(hVol, IOCTL_DISK_GET_FREE_SPACE, NULL, 0, &fr, sizeof(fr), 
            &cb, NULL) || cb != sizeof(fr))
            {
                bReturnValue = FALSE;
                ERR("DeviceIoControl(IOCTL_DISK_GET_FREE_SPACE");
                goto EXIT;
            }
            else
            {
                g_sfatInfo.dwUsableClusters = fr.fr_Clusters;
                g_sfatInfo.dwFreeClusters   = fr.fr_FreeClusters;
            }
    }
    DisplayFATInfo(&g_sfatInfo);
EXIT:
    if (VALID_HANDLE(hVol))
    {
        CloseHandle(hVol);
    }
    return bReturnValue;
#else
    return FALSE;
#endif
}

BOOL FSDT_FillDiskMinusXBytesWithMapping(PTCHAR szPath, PTCHAR szFileNameTmp, DWORD dwFreeSpace, DWORD *pdwFileCnt)
{
    BOOL fRet = FALSE;
    TCHAR szFileName[MAX_PATH];
    HANDLE hFile = INVALID_HANDLE_VALUE;
    HANDLE hMapping = INVALID_HANDLE_VALUE;
    ULARGE_INTEGER fileSize;
    ULARGE_INTEGER fb, tb, tfb;
    ULARGE_INTEGER sizeToCreate;
    UINT fileCount; 
    DWORD dwClusterSize = 0;
    DWORD dwBytesPerSector, dwSectorsPerCluster;
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
            dwClusterSize = dwSectorsPerCluster * dwBytesPerSector;
        }

    }
#else
    {
        
        GetDiskFreeSpace(szPath, &dwBytesPerSector,&dwSectorsPerCluster, NULL, NULL);
        dwClusterSize = dwBytesPerSector * dwSectorsPerCluster;
    }
#endif
    fileCount = 0;
    _CCprintf( _T("Filling [%s] so that %u bytes remain (FileMapping version)\n"), 
        szPath, dwFreeSpace);

    // the fill file might already exist, so delete it before we query disk space
//    UnFillDisk();  // this will fail when the file doesn't exist    

    if(!GetDiskFreeSpaceEx(szPath, &fb, &tb, &tfb))
    {
        ERR("GetDiskFreeSpaceEx()");
        _CCprintf( _T("GetDiskFreeSpaceEx: %s Last error == 0x%08X\n"), szPath, GetLastError());
        goto EXIT;
    }

    _CCprintf( _T("Partition [%s] has %I64u free bytes\n"),  
        szPath, fb.QuadPart);

    // the free space requested was more than is currently free on the disk
    if(fb.QuadPart < dwFreeSpace)
    {
        _CCprintf( _T("Error Disk has only %I64u free bytes"), fb.QuadPart);
        _CCprintf( _T("Not enough disk space for fill file\n"));
        goto EXIT;
    }

    // size of the file to create is current free space minus requested free space
    fileSize.QuadPart = fb.QuadPart - dwFreeSpace;

    // some things can occur when we're trying to fill a disk:
    //
    //  the disk could be larger than 2 GB, the FATFS file size limit -- this 
    //  require multiple fill files
    //
    //  the disk could advertise more free space than is actually available; 
    //  TFAT seems to have this issue
    //
    while(fileSize.QuadPart > 0)
    {
#define MIN(x, y)   ((x) < (y) ? (x) : (y))

        // create several fill files, if required, because
        // SetFilePointer won't go past 32-bit address of a file
        sizeToCreate.QuadPart = (LONG)MIN(MAXDWORD>>1, fileSize.QuadPart);
        //sizeToCreate.QuadPart = (LONG)MIN(500, fileSize.QuadPart);
        // build the fill file name
        _stprintf(szFileName, _T("%s\\%s%u"), szPath, szFileNameTmp,
            ++fileCount);
        _CCprintf( _T("Creating file %s of %u bytes to fill disk\n"), szFileName, sizeToCreate);

        hFile = CreateFile ( 
            szFileName,					
            GENERIC_READ|GENERIC_WRITE, 
            FILE_SHARE_READ|FILE_SHARE_WRITE,  
            NULL, 
            CREATE_ALWAYS, 
            FILE_ATTRIBUTE_NORMAL ,
            NULL 
            );
        if(INVALID_HANDLE(hFile))
        {
            ERR("CreateFile()");
            _CCprintf( _T("Could not create file \"%s\" for filling Last error == 0x%08X\n"), szFileName, GetLastError());
            goto EXIT;
        }

        hMapping = CreateFileMapping(hFile, NULL, PAGE_READWRITE, sizeToCreate.HighPart, sizeToCreate.LowPart, NULL);
       
#if 0
        if(INVALID_HANDLE(hMapping))
        {
            ERR("CreateFileMapping()");
            goto EXIT;
        }
#endif

        if(!GetDiskFreeSpaceEx(szPath, &fb, &tb, &tfb))
        {
            ERR("GetDiskFreeSpaceEx()");
            _CCprintf( _T("GetDiskFreeSpaceEx: %s Last error == 0x%08X\n"), szPath, GetLastError());
            goto EXIT;
        }
 
        _CCprintf( _T("Partition [%s] now has %I64u free bytes\n"),  szPath, fb.QuadPart);
        fileSize.QuadPart = fb.QuadPart - dwFreeSpace;

        if(VALID_HANDLE(hFile))
            CloseHandle(hFile);

        if(VALID_HANDLE(hMapping))
            CloseHandle(hMapping);
        hFile = INVALID_HANDLE_VALUE;
    }

    fRet = TRUE;

EXIT:
    if(VALID_HANDLE(hFile))
        CloseHandle(hFile);

    if(VALID_HANDLE(hMapping))
        CloseHandle(hMapping);
    *pdwFileCnt = fileCount;
    return fRet;
}



BOOL FSDT_SeekTest(HANDLE hFile, PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64SeekPos, DWORD OperatorLen, DWORD dwMoveMethod)
{
    BOOL bReturnValue = TRUE;
    DWORD nBytesRW;
    DWORD dwLastError  = NO_ERROR;
    DWORD RWLen = 0;
    PBYTE		pBufferRead       = NULL;
    LARGE_INTEGER li64SeekPos;
    DWORD dwTmpBufferSize = dwBufferSize;
    UINT32 cnt = 0;

    li64SeekPos.QuadPart = i64SeekPos;

    pBufferRead = (PBYTE)LocalAlloc( 0, dwBufferSize );
    if ( pBufferRead == NULL)
    {
        bReturnValue = FALSE;
        ERR("LocalAlloc()");
        goto EXIT;
    }

    if (INVALID_HANDLE(hFile))
    {
        bReturnValue = FALSE;
        _CCprintf( _T("ERRPR: Invaild file handle in FSDT_SeekTest() \n"));
        goto EXIT;  
    }

    if(0xFFFFFFFF == SetFilePointer( hFile, li64SeekPos.LowPart, &li64SeekPos.HighPart, dwMoveMethod) &&
        NO_ERROR != GetLastError())
    {
        bReturnValue = FALSE;
        ERR("SetFilePointer()");
        goto EXIT;
    }
    li64SeekPos.QuadPart = i64SeekPos;
    // write data.
    RWLen = OperatorLen;
    while (RWLen )
    {   
        //    
        if (RWLen < dwBufferSize )
        {
            dwBufferSize = RWLen;
        }
        bReturnValue = WriteFile(hFile, pTestBuffer, dwBufferSize, &nBytesRW, NULL );


        // if write len != 0, flush buffer every step.
        if (!FlushFileBuffers(hFile))
        {
            dwLastError = GetLastError();
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBlockSize)
            ERR("FlushFileBuffers()");
            _CCprintf( _T("Error on FlushFileBuffers()" ));
            _CCprintf( _T("Last error == 0x%08X\n"), dwLastError) ;
            goto EXIT;         

        }


        dwLastError = GetLastError();
        if (!bReturnValue || (nBytesRW != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            _CCprintf( _T("Error writing to file in test.SeekTest()" ));
            _CCprintf( _T("Last error == 0x%08X\n"), dwLastError) ;
            goto EXIT;
        }
        RWLen -= dwBufferSize;
    }

    if (INVALID_HANDLE(hFile))
    {
        bReturnValue = FALSE;
        _CCprintf( _T("Invaild file handle in FSDT_SeekTest() \n"));
        goto EXIT;  
    }
    

    
    li64SeekPos.QuadPart = OperatorLen;
    li64SeekPos.QuadPart = -li64SeekPos.QuadPart;
    
    if(0xFFFFFFFF == SetFilePointer( hFile, li64SeekPos.LowPart, &li64SeekPos.HighPart, FILE_CURRENT) &&
        NO_ERROR != GetLastError())
    {

        _CCprintf( TEXT("\n======%I64d %d\n"),li64SeekPos.QuadPart, dwMoveMethod);
        ERR("SetFilePointer()");
        goto EXIT;
    }
    
    //read and verif data
    RWLen = OperatorLen;
    while (RWLen )
    {
        //    
        if (RWLen < dwBufferSize )
        {
            dwBufferSize = RWLen;
        }


        //Read the data and verify that it is correct.
        bReturnValue = ReadFile( hFile, pBufferRead, dwBufferSize, &nBytesRW, NULL );
        if ( !bReturnValue || (nBytesRW != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            _CCprintf( _T("Error reading file in test.SeekTest()" ));
            _CCprintf( _T("Last error == 0x%08X\n"), dwLastError);
            goto EXIT;
        }


        if ( memcmp(pBufferRead, pTestBuffer, dwBufferSize) != 0 )
        {

            bReturnValue = FALSE;
            _CCprintf( _T("SEEK ERROR: File verif miscompare!\n")) ;
            goto EXIT;
        }

        RWLen -= dwBufferSize;
    }

EXIT:
    if ( pBufferRead )
    {
        LocalFree( pBufferRead );
    }
    return bReturnValue;
}

BOOL FSDT_SeekReadTest(HANDLE hFile, PBYTE pTestBuffer, DWORD dwBufferSize, __int64 i64SeekPos, DWORD OperatorLen, DWORD dwMoveMethod)
{
    BOOL bReturnValue = TRUE;
    DWORD nBytesRW;
    DWORD dwLastError  = NO_ERROR;
    DWORD RWLen = 0;
    PBYTE		pBufferRead       = NULL;
    LARGE_INTEGER li64SeekPos;
    DWORD dwTmpBufferSize = dwBufferSize;
    UINT32 cnt = 0;

    li64SeekPos.QuadPart = i64SeekPos;

    pBufferRead = (PBYTE)LocalAlloc( 0, dwBufferSize );
    if ( pBufferRead == NULL)
    {
        bReturnValue = FALSE;
        ERR("LocalAlloc()");
        goto EXIT;
    }

    if (INVALID_HANDLE(hFile))
    {
        bReturnValue = FALSE;
        _CCprintf( _T("ERRPR: Invaild file handle in FSDT_SeekTest() \n"));
        goto EXIT;  
    }

    li64SeekPos.QuadPart = OperatorLen;
    li64SeekPos.QuadPart = -li64SeekPos.QuadPart;

    if(0xFFFFFFFF == SetFilePointer( hFile, li64SeekPos.LowPart, &li64SeekPos.HighPart, FILE_CURRENT) &&
        NO_ERROR != GetLastError())
    {

        _CCprintf( TEXT("\n======%I64d %d\n"),li64SeekPos.QuadPart, dwMoveMethod);
        ERR("SetFilePointer()");
        goto EXIT;
    }

    //read and verif data
    RWLen = OperatorLen;
    while (RWLen )
    {
        //    
        if (RWLen < dwBufferSize )
        {
            dwBufferSize = RWLen;
        }


        //Read the data and verify that it is correct.
        bReturnValue = ReadFile( hFile, pBufferRead, dwBufferSize, &nBytesRW, NULL );
        if ( !bReturnValue || (nBytesRW != dwBufferSize) )
        {
            bReturnValue = FALSE;   //In case (nBytesWritten != dwBufferSize)
            _CCprintf( _T("Error reading file in test.SeekTest()" ));
            _CCprintf( _T("Last error == 0x%08X\n"), dwLastError);
            goto EXIT;
        }


        if ( memcmp(pBufferRead, pTestBuffer, dwBufferSize) != 0 )
        {

            bReturnValue = FALSE;
            _CCprintf( _T("SEEK ERROR: File verif miscompare!\n")) ;
            goto EXIT;
        }

        RWLen -= dwBufferSize;
    }

EXIT:
    if ( pBufferRead )
    {
        LocalFree( pBufferRead );
    }
    return bReturnValue;
}
BOOL FSDT_CutFile(HANDLE hFile, __int64 i64SeekPos, DWORD dwMoveMethod)
{
     BOOL bReturnValue = TRUE;
     LARGE_INTEGER li64SeekPos;

     li64SeekPos.QuadPart = i64SeekPos;

     if (INVALID_HANDLE(hFile))
     {
         bReturnValue = FALSE;
         _CCprintf( _T("Invaild file handle in FSDT_CutFile() \n"));
         goto EXIT;  
     }

     if(0xFFFFFFFF == SetFilePointer( hFile, li64SeekPos.LowPart, &li64SeekPos.HighPart, dwMoveMethod) &&
         NO_ERROR != GetLastError())
     {
         bReturnValue = FALSE;
         ERR("SetFilePointer()");
         goto EXIT;
     }

     if (!SetEndOfFile( hFile))
     {
         bReturnValue = FALSE;
         ERR("SetFilePointer()");
         goto EXIT;
     }


EXIT:
     return bReturnValue;
}

__int64 FSDT_GetDiskFreeSpace(PTCHAR szPath)
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