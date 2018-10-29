#define _DEBUGSTREAMINFODATAENTRY_GV
#include "DebugStreamInfoDataEntry.h"

BOOL WINAPI InitializeDebugStreamInfoDataEntry()
{
	DWORD	dwWaitStatus;
	RtlZeroMemory(&g_DebugStreamInfoDataEntry, sizeof(DEBUG_STREAM_INFO_DATAENTRY));
	if (NULL == g_hLoggingDbgStreamThread)
	{
		g_hLoggingDbgStreamThread = CreateThread(NULL, 0, LoggingDbgStreamThreadProc, NULL, 0, NULL);
	}
	dwWaitStatus = WaitForSingleObject(g_hDbgStreamTokenSendData, MAX_WAIT_MILLISECONDS);
	if (WAIT_FAILED == dwWaitStatus || WAIT_TIMEOUT == dwWaitStatus)
	{
		return FALSE;
	}
	RequestAccessDataEntryToken();
	StringCchPrintf(g_DebugStreamInfoDataEntry.szDbgStreamInfoBuff,
		DEBUG_STREAM_INFO_MAX_CHAR_BUFFER,
		L"%s%s",
		L"DbgStream engine loaded OK",
		TEXT_NEW_LINE);
	ReleaseAccessDataEntryToken();
	SetEvent(g_hDbgStreamTokenRecvData);
	return TRUE;
}

BOOL WINAPI DestroyDebugStreamInfoDataEntry()
{
	PWCHAR	pszText = L"Destroy debug stream information";
	DWORD	dwStatus = WAIT_TIMEOUT;
	LoggingDebugStream(pszText, (DWORD)wcslen(pszText));
	if (NULL != g_hLoggingDbgStreamThread)
	{
		g_bIsExitLoggingDbgStreamThd = TRUE;
		QueueUserAPC((PAPCFUNC)DummyLoggingDbgStreamThreadAPCProc, g_hLoggingDbgStreamThread, (ULONG_PTR)NULL);
		dwStatus = WaitForSingleObject(g_hLoggingDbgStreamThread, 4000);
		if (WAIT_TIMEOUT == dwStatus)
		{
			pszText = L"Dbg stream thread exit timeout!";
			LoggingDebugStream(pszText, (DWORD)wcslen(pszText));
			//Force to terminate thread asynchronizly ,extreme case.
			TerminateThread(g_hLoggingDbgStreamThread,0);
			WaitForSingleObject(g_hLoggingDbgStreamThread, 4000);
		}
		CloseHandle(g_hLoggingDbgStreamThread);
		g_hLoggingDbgStreamThread = NULL;
	}
	return TRUE;
}
VOID WINAPI InitializeDbgStreamInfo()
{
	if (NULL == g_hDbgStreamTokenSendData)
	{
		//Init signal.
		g_hDbgStreamTokenSendData = CreateEvent(NULL, FALSE, FALSE, DEBUG_STREAM_INFO_EVENT_NAME_SD);
	}
	if (NULL == g_hDbgStreamTokenRecvData)
	{
		//Init nonsignal.
		g_hDbgStreamTokenRecvData = CreateEvent(NULL, FALSE, FALSE, DEBUG_STREAM_INFO_EVENT_NAME_RD);
	}
	return;
}
VOID WINAPI DestroyDbgStreamInfo()
{
	if (NULL != g_hDbgStreamTokenSendData)
	{
		CloseHandle(g_hDbgStreamTokenSendData);
		g_hDbgStreamTokenSendData = NULL;
	}
	if (NULL != g_hDbgStreamTokenRecvData)
	{
		CloseHandle(g_hDbgStreamTokenRecvData);
		g_hDbgStreamTokenRecvData = NULL;
	}
	return;
}
VOID WINAPI UpdateDebugStreamInfoDataEntry()
{
	//Reserve.
	return;
}



DWORD WINAPI LoggingDbgStreamThreadProc(LPVOID lpParameter)
{
	DWORD			dwWaitStatus;
	BOOL			bIsReset = TRUE;
	BOOL            bIsOK = FALSE;
	DWORD			dwNeedToWriteByteCount = 0;
	DWORD			dwWritedCount = 0;
	HANDLE			hLoggingFile = INVALID_HANDLE_VALUE;
	WCHAR           szLoggingFileDir[MAX_PATH];
	WCHAR			szTempBuff[DEBUG_STREAM_INFO_MAX_CHAR_BUFFER];

	bIsOK = GetModuleFileDirectory(NULL, szLoggingFileDir, sizeof(szLoggingFileDir)/sizeof(WCHAR));
	if (FALSE == bIsOK)
	{
		StringCchPrintf(szTempBuff, DEBUG_STREAM_INFO_MAX_CHAR_BUFFER, L"%s", LOGGING_FILE_NAME);
	}
	else
	{
		StringCchPrintf(szTempBuff,DEBUG_STREAM_INFO_MAX_CHAR_BUFFER,L"%s\\%s",szLoggingFileDir,LOGGING_FILE_NAME);
	}
	hLoggingFile = CreateFile(szTempBuff,GENERIC_WRITE, FILE_SHARE_READ,NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL,NULL);
	if (INVALID_HANDLE_VALUE == hLoggingFile || NULL == g_hDbgStreamTokenSendData || NULL == g_hDbgStreamTokenRecvData)
	{
		return 0;
	}
	RequestAccessDataEntryToken();
	g_DebugStreamInfoDataEntry.bIsLoggingReady = 1;
	ReleaseAccessDataEntryToken();
	//Allow any threads send dbg info.
	SetEvent(g_hDbgStreamTokenSendData);
	while (!g_bIsExitLoggingDbgStreamThd)
	{
		dwWaitStatus = WaitForSingleObjectEx(g_hDbgStreamTokenRecvData, MAX_WAIT_DETECT_DEADLOCK_MILLISECONDS, TRUE);
		if (WAIT_FAILED == dwWaitStatus)
		{
			break;
		}
		else if (WAIT_OBJECT_0 == dwWaitStatus)
		{
			RequestAccessDataEntryToken();
			dwNeedToWriteByteCount = (DWORD)wcslen(g_DebugStreamInfoDataEntry.szDbgStreamInfoBuff);
			dwNeedToWriteByteCount = dwNeedToWriteByteCount * sizeof(WCHAR);
			StringCchCopy(szTempBuff, DEBUG_STREAM_INFO_MAX_CHAR_BUFFER, g_DebugStreamInfoDataEntry.szDbgStreamInfoBuff);
			ReleaseAccessDataEntryToken();

			if (dwNeedToWriteByteCount > 0)
			{
				WriteFile(hLoggingFile, (LPCVOID)szTempBuff, dwNeedToWriteByteCount,&dwWritedCount, NULL);
			}
			SetEvent(g_hDbgStreamTokenSendData);
		}
		else if (WAIT_TIMEOUT == dwWaitStatus)
		{
			//Process detact deadlock possibly.
		}
	}
	RequestAccessDataEntryToken();
	g_DebugStreamInfoDataEntry.bIsLoggingReady = 0;
	ReleaseAccessDataEntryToken();
	if (INVALID_HANDLE_VALUE != hLoggingFile)
	{
		CloseHandle(hLoggingFile);
		hLoggingFile = INVALID_HANDLE_VALUE;
	}
	return 1;
}

VOID WINAPI LoggingDebugStream(PWCHAR pszDbgString, DWORD dwStringLen)
{
	DWORD	dwWaitStatus;
	DWORD   bIsReadyTemp = 0;
	if (NULL == pszDbgString || 0 == dwStringLen)
	{
		return;
	}
	RequestAccessDataEntryToken();
	bIsReadyTemp = g_DebugStreamInfoDataEntry.bIsLoggingReady;
	ReleaseAccessDataEntryToken();

	if (0 == bIsReadyTemp)
	{
		return;
	}
	if (NULL == g_hDbgStreamTokenSendData || NULL == g_hDbgStreamTokenRecvData)
	{
		return;
	}

	if (FALSE == g_bIsDbgStreamHeaderProcessed)
	{
		g_bIsDbgStreamHeaderProcessed = TRUE;
		LoggingDebugStreamHeader();
	}
	dwWaitStatus = WaitForSingleObject(g_hDbgStreamTokenSendData,MAX_WAIT_MILLISECONDS);
	if (WAIT_FAILED == dwWaitStatus || WAIT_TIMEOUT == dwWaitStatus)
	{
		return;
	}
	RequestAccessDataEntryToken();
	StringCchPrintf(g_DebugStreamInfoDataEntry.szDbgStreamInfoBuff,
		DEBUG_STREAM_INFO_MAX_CHAR_BUFFER, 
		L"%s%s", 
		pszDbgString, 
		TEXT_NEW_LINE);
	ReleaseAccessDataEntryToken();
	SetEvent(g_hDbgStreamTokenRecvData);
	return;
}


VOID WINAPI LoggingDebugStreamHeader()
{
	SYSTEM_INFO			   stOSSI;
	OSVERSIONINFO		   stOSVI;
	SYSTEMTIME             stLocalSysTime;
	FN_GETNATIVESYSTEMINFO pGetNativeSystemInfo = NULL;
	FN_GETVERSIONEX        pGetVersionEx = NULL;
	WCHAR				   szTextFmt[160];
	INT					   nProcessorArchType = 0;
	PWCHAR                 pszProcessorArchList[] = { L"Unknown",L"X86",L"AMD64",L"IA64"};

	ZeroMemory(&stLocalSysTime, sizeof(SYSTEMTIME));
	GetLocalTime(&stLocalSysTime);
	StringCchPrintf(szTextFmt, sizeof(szTextFmt)/sizeof(WCHAR),
		L"Debugging stream time-%d/%d/%d %02d:%02d:%02d",
		stLocalSysTime.wYear, stLocalSysTime.wMonth, stLocalSysTime.wDay,
		stLocalSysTime.wHour, stLocalSysTime.wMinute, stLocalSysTime.wSecond);
	LoggingDebugStream(szTextFmt, (DWORD)wcslen(szTextFmt));

	ZeroMemory(&stOSVI, sizeof(OSVERSIONINFO));
	stOSVI.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	pGetVersionEx = (FN_GETVERSIONEX)GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"),
		"GetVersionExW");
	if (NULL != pGetVersionEx &&  pGetVersionEx((POSVERSIONINFO)&stOSVI))
	{
		StringCchPrintf(szTextFmt, sizeof(szTextFmt) / sizeof(WCHAR),
			L"Windows version-MajorVersion:%02d,MinorVersion:%02d,BuildNumber:%d",
			stOSVI.dwMajorVersion, stOSVI.dwMinorVersion, stOSVI.dwBuildNumber);
		LoggingDebugStream(szTextFmt, (DWORD)wcslen(szTextFmt));
	}
	ZeroMemory(&stOSSI, sizeof(SYSTEM_INFO));
	pGetNativeSystemInfo = (FN_GETNATIVESYSTEMINFO)GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"),
		"GetNativeSystemInfo");
	if (NULL == pGetNativeSystemInfo)
	{
		GetSystemInfo(&stOSSI);
	}
	else 
	{
		pGetNativeSystemInfo(&stOSSI);
	}
	if (PROCESSOR_ARCHITECTURE_INTEL == stOSSI.wProcessorArchitecture)
	{
		nProcessorArchType = 1;
	}
	else if (PROCESSOR_ARCHITECTURE_AMD64 == stOSSI.wProcessorArchitecture)
	{
		nProcessorArchType = 2;
	}
	else if (PROCESSOR_ARCHITECTURE_IA64 == stOSSI.wProcessorArchitecture)
	{
		nProcessorArchType = 3;
	}
	StringCchPrintf(szTextFmt, sizeof(szTextFmt) / sizeof(WCHAR),
		L"ProcessorArchitecture-%s  ProcessorType-%d",
		pszProcessorArchList[nProcessorArchType], stOSSI.dwProcessorType);
	LoggingDebugStream(szTextFmt, (DWORD)wcslen(szTextFmt));
	return;
}
VOID CALLBACK DummyLoggingDbgStreamThreadAPCProc(ULONG_PTR dwParam)
{
	return;
}
BOOL WINAPI GetModuleFileDirectory(HMODULE hMod, PWCHAR pszBuff, DWORD dwMaxCharCount)
{
	DWORD	dwRetSize;
	INT     nIndex;
	DWORD   dwReturnedCharCount = 0;
	if (NULL == pszBuff || 0 == dwMaxCharCount)
	{
		return FALSE;
	}
	dwRetSize = GetModuleFileName(hMod, pszBuff, dwMaxCharCount);

	if (dwRetSize <= 0 || ERROR_INSUFFICIENT_BUFFER == GetLastError())
	{
		pszBuff[0] = L'\0';
		return FALSE;
	}
	nIndex = (INT)(dwRetSize - 1);
	while (L'\\' != pszBuff[nIndex])
	{
		--nIndex;
	}
	pszBuff[nIndex] = L'\0';
	return  TRUE;
}
