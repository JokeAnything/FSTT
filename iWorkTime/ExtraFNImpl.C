#define		_EXTRAFNIMPL_GV
#include	"EXTRAFNIMPL.H"

BOOL WINAPI JustSingleApplication(BOOL bIsCheck)
{
	BOOL	bRet;
	static	HANDLE hEvent=NULL;
	bRet=FALSE;
	if(bIsCheck)	//Process is starting.
	{
		hEvent=CreateEvent(NULL,FALSE,FALSE, IWORKTIME_SINGLE_INSTANCE);
		if(hEvent==NULL||GetLastError()==ERROR_ALREADY_EXISTS)
		{
			if(hEvent)
			{	//ERROR_ALREADY_EXISTS
				CloseHandle(hEvent);
				hEvent=NULL;
			}
			bRet=FALSE;		//Else no more resource to create event,return false.
		}
		else
		{
			bRet=TRUE;
		}
	}
	else			//Process is ending.
	{
		if(hEvent)
		{
			CloseHandle(hEvent);
			hEvent=NULL;
		}
		bRet=TRUE;
	}
	if(TRUE == bIsCheck && FALSE == bRet)
	{
		ShowPreviewInstanceWnd();
	}
	return bRet;
}

VOID WINAPI ShowPreviewInstanceWnd()
{
	if(NULL == g_hTalkShowWnd)
	{
		return ;
	}
	//Show previwe window instance,Post UM_TRAY_MSG_NOTIFY message.
	PostMessage(g_hTalkShowWnd, UM_TRAY_MSG_NOTIFY, 0, (LPARAM)WM_LBUTTONUP);
	return ;
}

BOOL WINAPI GetModuleBaseFileName(HMODULE hMod,PWCHAR pszBuff, DWORD dwMaxCharLen)
{
	DWORD	dwRetSize;
	INT     nIndex;
	DWORD   dwReturnedCharCount = 0;
	WCHAR szModuleFullName[MAX_PATH];
	if (NULL == hMod || NULL == pszBuff || 0 == dwMaxCharLen)
	{
		return FALSE;
	}
	dwRetSize = GetModuleFileName(hMod, szModuleFullName, sizeof(szModuleFullName) / sizeof(WCHAR));

	if (dwRetSize <= 0 || ERROR_INSUFFICIENT_BUFFER == GetLastError())
	{
		return FALSE;
	}
	nIndex = (INT)(dwRetSize-1);
	while (L'\\' != szModuleFullName[nIndex])
	{
		--nIndex;
	}
	do
	{
		nIndex++;
		pszBuff[dwReturnedCharCount] = szModuleFullName[nIndex];
		dwReturnedCharCount++;
	} while (L'\0' != szModuleFullName[nIndex] && dwReturnedCharCount < dwMaxCharLen);

	if (dwReturnedCharCount > dwMaxCharLen)
	{
		pszBuff[0] = L'\0';
		return FALSE;
	}
	return  TRUE;
}

LONG WINAPI ShowExcptInfo(PEXCEPTION_POINTERS	pstExcptInfo)
{
	DWORD					 dwSizeRet;
	ULONG_PTR				 uExptMmAddress;	//Fault memory address.
	PWCHAR					 pszTipsPtr;
	WCHAR					 szExceptionAddress[56];
	WCHAR                    szExceptionCode[36];
	WCHAR					 szErrorDetail[120];
	WCHAR					 szErrorModuleName[64];
	WCHAR					 szShowError[300];
	MEMORY_BASIC_INFORMATION stMBI;

	pszTipsPtr = L"Sorry,iWorkTime application encountered an serious problem,Stopped!";
	szExceptionAddress[0] = '\0';
	szExceptionCode[0] = '\0';
	szErrorDetail[0] = '\0';
	szErrorModuleName[0] = '\0';
	if (NULL == pstExcptInfo)
	{
		goto EXIT;
	}
	RtlZeroMemory(&stMBI, sizeof(MEMORY_BASIC_INFORMATION));
	dwSizeRet = (DWORD)VirtualQuery((LPCVOID)(pstExcptInfo->ExceptionRecord->ExceptionAddress),
		&stMBI,sizeof(MEMORY_BASIC_INFORMATION));
	if (dwSizeRet == (DWORD)sizeof(MEMORY_BASIC_INFORMATION))
	{
		GetModuleBaseFileName((HMODULE)stMBI.AllocationBase,szErrorModuleName,sizeof(szErrorModuleName) / sizeof(WCHAR));
	}
	StringCchPrintf(szExceptionAddress,
		sizeof(szExceptionAddress)/sizeof(WCHAR),
		L"Exception address:0x%08X",
		pstExcptInfo->ExceptionRecord->ExceptionAddress);
	StringCchPrintf(szExceptionCode,
		sizeof(szExceptionCode) / sizeof(WCHAR),
		L"Exception code:%08X",
		pstExcptInfo->ExceptionRecord->ExceptionCode);
	if(EXCEPTION_ACCESS_VIOLATION == pstExcptInfo->ExceptionRecord->ExceptionCode)
	{
		uExptMmAddress=pstExcptInfo->ExceptionRecord->ExceptionInformation[1];
		if(0 == pstExcptInfo->ExceptionRecord->ExceptionInformation[0])
		{
			StringCchPrintf(szErrorDetail,sizeof(szErrorDetail)/sizeof(WCHAR),L"Exception address:0x%08X,Try to read error address:0x%08X",
					pstExcptInfo->ExceptionRecord->ExceptionAddress,uExptMmAddress);
		}
		else
		{
			StringCchPrintf(szErrorDetail,sizeof(szErrorDetail)/sizeof(WCHAR),L"Exception address:0x%08X,Try to write or execute error address:0x%08X",
					pstExcptInfo->ExceptionRecord->ExceptionAddress,uExptMmAddress);
		}
	}
	else
	{
		StringCchPrintf(szErrorDetail, sizeof(szErrorDetail) / sizeof(WCHAR),L"%s",L"No more detail");
	}
EXIT:
	AddDbgPrintStream(DBGFMTMSG(L"%s"), CURTID, USERMODE, FUNCNAME(L"ShowExcptInfo"),
		DBG_ERROR, pszTipsPtr);
	AddDbgPrintStream(DBGFMTMSG(L"ModuleName:%s"), CURTID, USERMODE, FUNCNAME(L"ShowExcptInfo"),
		DBG_ERROR, szErrorModuleName);
	AddDbgPrintStream(DBGFMTMSG(L"Exception address:%s"), CURTID, USERMODE, FUNCNAME(L"ShowExcptInfo"),
		DBG_ERROR, szExceptionAddress);
	AddDbgPrintStream(DBGFMTMSG(L"Exception code:%s"), CURTID, USERMODE, FUNCNAME(L"ShowExcptInfo"),
		DBG_ERROR, szExceptionCode);
	AddDbgPrintStream(DBGFMTMSG(L"Error detail:%s"), CURTID, USERMODE, FUNCNAME(L"ShowExcptInfo"),
		DBG_ERROR, szErrorDetail);

	StringCchPrintf(szShowError,
		sizeof(szShowError)/sizeof(WCHAR),
		L"%s\nModuleName:%s\nException address:%s\nException code:%s\nError detail:%s", 
		pszTipsPtr,
		szErrorModuleName,
		szExceptionAddress,
		szExceptionCode,
		szErrorDetail);
	MessageBox(NULL,szShowError,L"iWorkTime",MB_OK|MB_ICONWARNING);
	return EXCEPTION_EXECUTE_HANDLER;
}

