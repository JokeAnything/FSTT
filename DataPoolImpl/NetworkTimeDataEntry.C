#define	 _NETWORKTIMEDATAENTRY_GV
#include "NetworkTimeDataEntry.h"

BOOL WINAPI InitializeNetworkTimeDataEntry()
{
	RtlZeroMemory(&g_NetworkTimeDataEntry, sizeof(NETWORK_TIME_DATAENTRY));
	g_hNetworkTimeThread = CreateThread(NULL, 0, WorkThreadProc, NULL, 0, NULL);
	if (NULL == g_hNetworkTimeThread)
	{
		return FALSE;
	}
	return TRUE;
}

BOOL WINAPI DestroyNetworkTimeDataEntry()
{
	DWORD	dwStatus;
	if (NULL != g_hNetworkTimeThread)
	{
		g_bIsExitThread = TRUE;
		AddDbgPrintStream(DBGFMTMSG(L"Now request network time component thread to exit."), CURTID, USERMODE, FUNCNAME(L"DestroyNetworkTimeDataEntry"), DBG_TIPS);
		QueueUserAPC((PAPCFUNC)DummyAPCProc, g_hNetworkTimeThread, (ULONG_PTR)NULL);
		dwStatus = WaitForSingleObject(g_hNetworkTimeThread, MAX_SLEEP_MILLISECOND_VALUE_LEVEL1);
		if (WAIT_TIMEOUT == dwStatus)
		{
			AddDbgPrintStream(DBGFMTMSG(L"Network time component thread exited manner extreme!"), CURTID, USERMODE, FUNCNAME(L"DestroyNetworkTimeDataEntry"), DBG_WARNING);
			//Asyn to terminate thread,extreme case.
			TerminateThread(g_hNetworkTimeThread, 0);
			WaitForSingleObject(g_hNetworkTimeThread, MAX_SLEEP_MILLISECOND_VALUE_LEVEL1);
		}
		AddDbgPrintStream(DBGFMTMSG(L"Network time component thread exit normally!"), CURTID, USERMODE, FUNCNAME(L"DestroyNetworkTimeDataEntry"), DBG_TIPS);
		CloseHandle(g_hNetworkTimeThread);
		g_hNetworkTimeThread = NULL;
	}
	return TRUE;
}

VOID WINAPI UpdateNetworkTimeDataEntry()
{
	return;
}

BOOL WINAPI GetGMTNetworkTime(PSYSTEMTIME pSystemTime, PBOOL bIsLocalTime)
{
	DWORD			dwTempTickcountStamp = 0;
	DWORD			dwCurrentTickcountStamp = 0;
	DWORD			bIsTempValid = 0;
	DWORD			bIsOverflow = 0;
	SYSTEMTIME		stGMNetworkTime;
	FILETIME		stFileTime;
	LARGE_INTEGER	stLI;
	LARGE_INTEGER   stLIInterval;
	BOOL			bIsOK;

	if (NULL == pSystemTime || NULL == bIsLocalTime)
	{
		return FALSE;
	}
	RequestAccessDataEntryToken();
	dwTempTickcountStamp = g_NetworkTimeDataEntry.dwTickcountStamp;
	bIsTempValid = g_NetworkTimeDataEntry.bIsGMTTimeValid;
	stGMNetworkTime = g_NetworkTimeDataEntry.stGMT_Time;
	dwCurrentTickcountStamp = GetTickCount();
	dwCurrentTickcountStamp < dwTempTickcountStamp ? (bIsOverflow = 1) : (bIsOverflow = 0);
	ReleaseAccessDataEntryToken();

	if (0 == bIsTempValid || 1 == bIsOverflow)
	{
		*bIsLocalTime = TRUE;
		GetSystemTime(pSystemTime);
	}
	else
	{
		bIsOK = SystemTimeToFileTime(&stGMNetworkTime, &stFileTime);
		if (TRUE == bIsOK)
		{
			stLI.HighPart = stFileTime.dwHighDateTime;
			stLI.LowPart = stFileTime.dwLowDateTime;
			stLIInterval.QuadPart = dwCurrentTickcountStamp - dwTempTickcountStamp;
			stLIInterval.QuadPart = stLIInterval.QuadPart * 1000 * 10;
			stLI.QuadPart = stLI.QuadPart + stLIInterval.QuadPart;
			stFileTime.dwHighDateTime = stLI.HighPart;
			stFileTime.dwLowDateTime = stLI.LowPart;
			*bIsLocalTime = FALSE;
			bIsOK = FileTimeToSystemTime(&stFileTime, pSystemTime);
		}
		if (FALSE == bIsOK)
		{
			*bIsLocalTime = TRUE;
			GetSystemTime(pSystemTime);
		}
	}
	return TRUE;
}

DWORD WINAPI WorkThreadProc(LPVOID lpParameter)
{
	BOOL			bIsOK;
	BOOL			bIsNeedRetrieve = FALSE;
	DWORD			dwTempTickcountStamp = 0;
	DWORD			dwCurrentTickcountStamp = 0;
	DWORD			bIsTempValid = 0;
	DWORD			bIsOverflow = 0;
	DWORD			dwFailedCount = 0;
	DWORD			dwWaitMillisecond = MAX_SLEEP_MILLISECOND_FAILBASE;
	SYSTEMTIME		stSysTime;
	static  DWORD   dwStaticCnt = 0;

	while (!g_bIsExitThread)
	{
		RequestAccessDataEntryToken();
		dwTempTickcountStamp = g_NetworkTimeDataEntry.dwTickcountStamp;
		bIsTempValid = g_NetworkTimeDataEntry.bIsGMTTimeValid;
		dwCurrentTickcountStamp = GetTickCount();
		dwCurrentTickcountStamp < dwTempTickcountStamp ? (bIsOverflow = TRUE) : (bIsOverflow = FALSE);
		bIsOverflow ? (g_NetworkTimeDataEntry.bIsGMTTimeValid = 0) : (g_NetworkTimeDataEntry.bIsGMTTimeValid = g_NetworkTimeDataEntry.bIsGMTTimeValid);
		ReleaseAccessDataEntryToken();
		if (0 == bIsTempValid || TRUE == bIsOverflow)
		{
			AddDbgPrintStream(DBGFMTMSG(L"Try to obtain network time."), CURTID, USERMODE, FUNCNAME(L"NTThreadProc"), DBG_TIPS);
			RtlZeroMemory(&stSysTime, sizeof(SYSTEMTIME));
			bIsOK = GetNetworkTimeByHTTPProtocol(&stSysTime);
			if (TRUE == bIsOK)
			{
				AddDbgPrintStream(DBGFMTMSG(L"Obtain network time successfully."), CURTID, USERMODE, FUNCNAME(L"NTThreadProc"), DBG_TIPS);
				RequestAccessDataEntryToken();
				g_NetworkTimeDataEntry.stGMT_Time = stSysTime;
				g_NetworkTimeDataEntry.dwTickcountStamp = GetTickCount();
				g_NetworkTimeDataEntry.bIsGMTTimeValid = 1;
				ReleaseAccessDataEntryToken();
				dwFailedCount = 0;
			}
			else
			{
				AddDbgPrintStream(DBGFMTMSG(L"Obtain network time failed."), CURTID, USERMODE, FUNCNAME(L"NTThreadProc"), DBG_WARNING);
				dwFailedCount++;
				dwWaitMillisecond = dwWaitMillisecond + dwFailedCount*MAX_SLEEP_MILLISECOND_FAILXFACTOR;
				dwWaitMillisecond > MAX_SLEEP_MILLISECOND_VALUE ? (dwWaitMillisecond = MAX_SLEEP_MILLISECOND_VALUE) : (dwWaitMillisecond = dwWaitMillisecond);
				SleepEx(dwWaitMillisecond, TRUE);

				continue;
			}
		}
		SleepEx(MAX_SLEEP_MILLISECOND_VALUE_LEVEL4, TRUE);
		dwStaticCnt++;
		if (dwStaticCnt >= 20) // MAX_SLEEP_MILLISECOND_VALUE_LEVEL4*20 == 15000*4*5 = 5 minute
		{
			dwStaticCnt = 0;
			RequestAccessDataEntryToken();
			//Get network time again.
			g_NetworkTimeDataEntry.bIsGMTTimeValid = 0;
			ReleaseAccessDataEntryToken();
		}
	}
	return 1;
}


BOOL WINAPI GetNetworkTimeByHTTPProtocol(PSYSTEMTIME pSystemTime)
{
	INT			iResult;
	INT			iSvrHostNameCount;
	INT			iIndex;
	CHAR		szIpAddress[64];
	CHAR		szRequestedHeader[120];
	CHAR		szDateBuff[70];
	PCHAR		szRequestedAction = "GET / HTTP/1.1\n";
	PCHAR		szRequestedEnd = "\n\n";
	WORD		wVersionRequested;
	SOCKET		hSvrSocket = INVALID_SOCKET;
	WSADATA		stWSAData;
	SOCKADDR_IN	svrAddr;
	CHAR		cbRecvBuff[256];
	BOOL		bIsOK = 0;

	if (NULL == pSystemTime)
	{
		return FALSE;
	}
	RtlZeroMemory(pSystemTime, sizeof(SYSTEMTIME));
	wVersionRequested = MAKEWORD(2, 2);
	iResult = WSAStartup(wVersionRequested, &stWSAData);
	if (iResult != 0)
	{
		//Error show:printf("WSAStartup failed: %d\n", iResult);
		return FALSE;
	}
	iSvrHostNameCount = sizeof(g_szServerHostNameList) / sizeof(PCHAR);
	for (iIndex = 0; iIndex < iSvrHostNameCount; iIndex++)
	{
		bIsOK = GetHostIPAddrByHostName(g_szServerHostNameList[iIndex], szIpAddress, sizeof(szIpAddress) / sizeof(CHAR));
		if (FALSE == bIsOK) continue;
		AddDbgPrintStream(DBGFMTMSG(L"Try to obtain network time at:%S."), CURTID, USERMODE, FUNCNAME(L"NTThreadProc"),
			DBG_TIPS, g_szServerHostNameList[iIndex]);
		hSvrSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (hSvrSocket == INVALID_SOCKET) continue;

		svrAddr.sin_family = AF_INET;
		svrAddr.sin_port = htons(80);//80 port.
		svrAddr.sin_addr.S_un.S_addr = inet_addr(szIpAddress);
		iResult = connect(hSvrSocket, (SOCKADDR*)&svrAddr, sizeof(SOCKADDR_IN));
		if (SOCKET_ERROR == iResult)
		{
			closesocket(hSvrSocket);
			hSvrSocket = (SOCKET)NULL;
			continue;
		}
		StringCchPrintfA(szRequestedHeader, sizeof(szRequestedHeader), "%sHost:%s\n%s",
			szRequestedAction, g_szServerHostNameList[iIndex], szRequestedEnd);
		iResult = send(hSvrSocket, szRequestedHeader, (INT)strlen(szRequestedHeader), 0);
		if (SOCKET_ERROR == iResult)
		{
			closesocket(hSvrSocket);
			hSvrSocket = (SOCKET)NULL;
			continue;
		}
		iResult = recv(hSvrSocket, cbRecvBuff, (sizeof(cbRecvBuff) / sizeof(CHAR)) - 1, 0);
		closesocket(hSvrSocket);
		hSvrSocket = (SOCKET)NULL;
		if (iResult <= 0)
		{
			continue;
		}
		cbRecvBuff[iResult] = '\0';
		bIsOK = RetriveDateInfoFromRespondContent(cbRecvBuff, szDateBuff, sizeof(szDateBuff) / sizeof(CHAR));
		if (TRUE == bIsOK)
		{
			AddDbgPrintStream(DBGFMTMSG(L"Obtain network time at:%S successfully."), CURTID, USERMODE, FUNCNAME(L"NTThreadProc"),
				DBG_TIPS, g_szServerHostNameList[iIndex]);
			*pSystemTime = ResolveTimeStringToSysTime(szDateBuff);
			break;
		}
	}
	WSACleanup();
	iIndex >= iSvrHostNameCount ? (bIsOK = FALSE) : (bIsOK = TRUE);
	return bIsOK;
}



BOOL WINAPI GetHostIPAddrByHostName(PCHAR pstrHostName, PCHAR szBuff, INT nBuffSize)
{
	struct hostent* remoteHost = NULL;
	struct in_addr addr;

	if (isalpha(pstrHostName[0]))
	{
		remoteHost = gethostbyname(pstrHostName);
	}
	if (NULL == remoteHost)
	{
		return FALSE;
	}
	addr.s_addr = *(PULONG)(remoteHost->h_addr_list[0]);//ULONG.
	StringCchCopyA(szBuff, nBuffSize, inet_ntoa(addr));
	return TRUE;
}

//Format: Wed, 02 Nov 2016 13:17:21
//GM.
SYSTEMTIME WINAPI ResolveTimeStringToSysTime(PCHAR pTimeString)
{
	INT			iIndex;
	INT			iSeparatorIndex;
	INT			bIsMeetDigital = 0;
	INT			bIsSkip = 0;
	INT			iTempBuffIndex = 0;
	INT			idateOrderIndex = 0;
	CHAR		szTempBuff[20];
	WORD		wordTemp = 0;
	CHAR		cbSeparator[] = { ' ',',',':','\n' };
	SYSTEMTIME  stTM;
	PWORD		dateOrderArray[] = { &stTM.wDayOfWeek,&stTM.wDay,&stTM.wMonth,&stTM.wYear,
		&stTM.wHour,&stTM.wMinute,&stTM.wSecond };
	RtlZeroMemory(&stTM, sizeof(SYSTEMTIME));
	if (NULL == pTimeString || '\0' == pTimeString[0])
	{
		return stTM;
	}
	for (iIndex = 0; *(pTimeString + iIndex) != '\0' && idateOrderIndex < (sizeof(dateOrderArray) / sizeof(PWORD)); iIndex++)
	{
		for (iSeparatorIndex = 0; iSeparatorIndex < (sizeof(cbSeparator) / sizeof(CHAR)); iSeparatorIndex++)
		{
			if (*(pTimeString + iIndex) != cbSeparator[iSeparatorIndex]) continue;
			szTempBuff[iTempBuffIndex] = '\0';
			//if iTempBuffIndex == 0,string length is zero,skipped!
			if (0 != iTempBuffIndex)
			{
				wordTemp = ((1 == bIsMeetDigital) ? (WORD)atoi(szTempBuff) : (WORD)ResolveStringDate(szTempBuff));
				*dateOrderArray[idateOrderIndex] = wordTemp;
				idateOrderIndex++;
				iTempBuffIndex = 0;
			}
			bIsMeetDigital = 0;
			//skip current separator character.
			bIsSkip = 1;
			break;
		}
		if (bIsSkip)
		{
			bIsSkip = 0;
			continue;
		}
		if (*(pTimeString + iIndex) >= '0' && *(pTimeString + iIndex) <= '9')
		{
			bIsMeetDigital = 1;
		}
		szTempBuff[iTempBuffIndex] = *(pTimeString + iIndex);
		iTempBuffIndex++;
	}
	return stTM;
}
INT WINAPI ResolveStringDate(PCHAR pDateString)
{
	INT		iIndex;
	INT		iArraySize;
	PCHAR	weekdayArray[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat" };
	PCHAR	monthArray[] = { "","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
	if (NULL == pDateString || '\0' == pDateString[0])
	{
		//Error case.
		return 0;
	}
	iArraySize = sizeof(weekdayArray) / sizeof(PCHAR);
	for (iIndex = 0; iIndex<iArraySize; iIndex++)
	{
		// Case insensitive 
		if (0 == _stricmp(pDateString, weekdayArray[iIndex]))
		{
			return iIndex;
		}
	}
	iArraySize = sizeof(monthArray) / sizeof(PCHAR);
	for (iIndex = 0; iIndex<iArraySize; iIndex++)
	{
		// Case insensitive 
		if (0 == _stricmp(pDateString, monthArray[iIndex]))
		{
			return iIndex;
		}
	}
	//Error case.
	return 0;
}

PCHAR WINAPI StrStrInsensitive(PCCH str, PCCH strSearch)
{
	INT		iIndex = 0;
	PCHAR	pszDupStr;
	PCHAR	pszDupStrSearch;
	PCHAR	pszReturnedPtr = NULL;
	PCHAR	pszTempPtr = NULL;

	pszReturnedPtr = (PCHAR)str;
	if (NULL == strSearch || '\0' == strSearch[0])
	{
		return pszReturnedPtr;
	}
	pszDupStr = _strdup(str);
	pszDupStrSearch = _strdup(strSearch);
	if ((NULL == pszDupStr) || (NULL == pszDupStrSearch))
	{
		goto EXIT;
	}
	_strlwr(pszDupStr);
	_strlwr(pszDupStrSearch);
	pszTempPtr = strstr(pszDupStr, pszDupStrSearch);
	if (NULL == pszTempPtr)
	{
		pszReturnedPtr = NULL;
	}
	else
	{
		pszReturnedPtr = pszReturnedPtr + ((pszTempPtr - pszDupStr) / sizeof(CHAR));
	}
EXIT:
	if (NULL != pszDupStr)
	{
		free(pszDupStr);
		pszDupStr = NULL;
	}
	if (NULL != pszDupStrSearch)
	{
		free(pszDupStrSearch);
		pszDupStrSearch = NULL;
	}
	return pszReturnedPtr;
}
INT WINAPI RetriveDateInfoFromRespondContent(PCHAR pContent, PCHAR szDateBuff, INT nBuffSize)
{
	PCHAR	pstrStartPtr = NULL;
	PCHAR	pstrEndPtr = NULL;
	PCHAR	pstrMark_1 = "Date:";
	PCHAR	pstrMark_2 = "GMT";
	INT		strLen;
	if (NULL == pContent || NULL == szDateBuff || 0 == nBuffSize)
	{
		return 0;
	}
	//Sensitive.
	pstrStartPtr = StrStrInsensitive(pContent, pstrMark_1);
	if (NULL == pstrStartPtr)
	{
		return 0;
	}
	pstrStartPtr = pstrStartPtr + strlen(pstrMark_1);
	pstrEndPtr = StrStrInsensitive(pContent, pstrMark_2);
	if (NULL == pstrEndPtr)
	{
		return 0;
	}
	pstrEndPtr = pstrEndPtr - 1;
	strLen = (INT)(pstrEndPtr - pstrStartPtr) + 1;
	if (strLen < nBuffSize - 1)
	{
		//< nBuffSize - 1
		strncpy(szDateBuff, pstrStartPtr, strLen);
		szDateBuff[strLen] = '\0';
		return 1;
	}
	return 0;
}
VOID CALLBACK DummyAPCProc(ULONG_PTR dwParam)
{
	return;
}

