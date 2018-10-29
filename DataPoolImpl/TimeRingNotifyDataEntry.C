#define	 _TIMERINGNOTIFYDATAENTRY_GV
#include "TimeRingNotifyDataEntry.h"

BOOL WINAPI InitializeTimeRingNotifyDataEntry()
{
	RtlZeroMemory(&g_TimeRingNotifyDataEntry, sizeof(TIME_RING_NOTIFY_DATAENTRY));
	g_TimeRingNotifyDataEntry.dwTimeRingNotifyFlag = (DWORD)-1;
	g_hRingNotifyThread = CreateThread(NULL, 0, RingNotifyThreadProc, NULL, 0, NULL);
	if (NULL == g_hRingNotifyThread)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Create time ring thread failed!"), CURTID, USERMODE, FUNCNAME(L"InitTimeRingNotifyDataEntry"), DBG_ERROR);
		return FALSE;
	}
	return TRUE;
}

BOOL WINAPI DestroyTimeRingNotifyDataEntry()
{
	DWORD	dwStatus;
	if (NULL != g_hRingNotifyThread)
	{
		g_bIsExitRingNotifyThd = TRUE;
		QueueUserAPC((PAPCFUNC)DummyRingNotifyThreadAPCProc, g_hRingNotifyThread, (ULONG_PTR)NULL);
		dwStatus = WaitForSingleObject(g_hRingNotifyThread, 4000);
		if (WAIT_TIMEOUT == dwStatus)
		{
			//Asyn to terminate thread,extreme case.
			TerminateThread(g_hRingNotifyThread, 0);
			WaitForSingleObject(g_hRingNotifyThread, 4000);
		}
		AddDbgPrintStream(DBGFMTMSG(L"Time ring thread exit normally!"), CURTID, USERMODE, FUNCNAME(L"DestroyTimeRingNotifyDataEntry"), DBG_TIPS);
		CloseHandle(g_hRingNotifyThread);
		g_hRingNotifyThread = NULL;
	}
	return TRUE;
}

VOID WINAPI UpdateTimeRingNotifyDataEntry()
{
	//Reserve.
	return;
}

BOOL WINAPI GetNextRingGMTTime(PSYSTEMTIME lpSysTime,PDWORD pdwTimeRingNotifyFlag)
{
	BOOL			bIsOK;
	INT				iIndexMinutes = 1;
	SYSTEMTIME		stTempSysTime;
	FILETIME		stFileTime;
	LARGE_INTEGER	stLiTime;
	LARGE_INTEGER	stLiOneMinuteNonsecond;
	if (NULL == lpSysTime || NULL == pdwTimeRingNotifyFlag)
	{
		return FALSE;
	}
	bIsOK = SystemTimeToFileTime(lpSysTime, &stFileTime);
	if (FALSE == bIsOK)
	{
		return FALSE;
	}
	stLiTime.LowPart = stFileTime.dwLowDateTime;
	stLiTime.HighPart = stFileTime.dwHighDateTime;
	stLiOneMinuteNonsecond.QuadPart = (LONGLONG)(1 * 60 * 1000 * 1000 * 10);	//100ns as a unit.
	for (iIndexMinutes = 1; iIndexMinutes <= RING_TIME_PERIOD_MINUTE; iIndexMinutes++)
	{
		stLiTime.QuadPart = stLiTime.QuadPart + stLiOneMinuteNonsecond.QuadPart;
		stFileTime.dwHighDateTime = stLiTime.HighPart;
		stFileTime.dwLowDateTime = stLiTime.LowPart;
		bIsOK = FileTimeToSystemTime(&stFileTime, &stTempSysTime);
		if (FALSE == bIsOK)
		{
			return FALSE;
		}
		if (0 == (stTempSysTime.wMinute % RING_TIME_PERIOD_MINUTE))
		{
			stTempSysTime.wSecond = 0;
			stTempSysTime.wMilliseconds = 0;
			*lpSysTime = stTempSysTime;
			*pdwTimeRingNotifyFlag = stTempSysTime.wMinute / RING_TIME_PERIOD_MINUTE;
			AddDbgPrintStream(DBGFMTMSG(L"Set next time ring at -%02d:%02d:%02d"), CURTID, USERMODE, FUNCNAME(L"GetNextRingGMTTime"), DBG_TIPS,
				stTempSysTime.wHour, stTempSysTime.wMinute, stTempSysTime.wSecond);
			break;
		}
	}
	return TRUE;
}

BOOL WINAPI SetTimeRingNotifyTimer()
{
	BOOL		bIsOK;
	FILETIME	stFileTime;
	LARGE_INTEGER stLITime;
	SYSTEMTIME	stTempSysTime;
	DWORD		dwTempTimeRingNotifyFlag;
	BOOL		bIsLocalGMTTime = TRUE;

	if (NULL == g_hRingWaiterTimer)
	{
		return FALSE;
	}
//	Use local time to notify.
//	bIsOK = GetGMTNetworkTime(&stTempSysTime, &bIsLocalGMTTime);
	bIsOK = FALSE;
	if (FALSE == bIsOK)
	{
		GetSystemTime(&stTempSysTime);
	}
	bIsOK = GetNextRingGMTTime(&stTempSysTime,&dwTempTimeRingNotifyFlag);
	if (FALSE == bIsOK)
	{
		return FALSE;
	}
	bIsOK = SystemTimeToFileTime(&stTempSysTime, &stFileTime);
	if (FALSE == bIsOK)
	{
		return FALSE;
	}
	stLITime.HighPart = stFileTime.dwHighDateTime;
	stLITime.LowPart = stFileTime.dwLowDateTime;
	bIsOK = SetWaitableTimer(g_hRingWaiterTimer, &stLITime, 0, NULL, NULL, FALSE);
	if (TRUE == bIsOK)
	{
		RequestAccessDataEntryToken();
		g_TimeRingNotifyDataEntry.stGMT_LastSetTime = stTempSysTime;
		g_TimeRingNotifyDataEntry.dwTimeRingNotifyFlag = dwTempTimeRingNotifyFlag;
		ReleaseAccessDataEntryToken();
		AddDbgPrintStream(DBGFMTMSG(L"Set time ring successfully!"), CURTID, USERMODE, FUNCNAME(L"SetTimeRingNotifyTimer"), DBG_TIPS);
	}
	else
	{
		AddDbgPrintStream(DBGFMTMSG(L"Set time ring failed!"), CURTID, USERMODE, FUNCNAME(L"SetTimeRingNotifyTimer"), DBG_ERROR);
	}
	return bIsOK;
}

DWORD WINAPI RingNotifyThreadProc(LPVOID lpParameter)
{
	DWORD			dwWaitStatus;
	BOOL			bIsReset = TRUE;
	if (NULL != g_hRingWaiterTimer)
	{
		return 0;
	}
	g_hRingWaiterTimer = CreateWaitableTimer(NULL, FALSE, RING_WAITTIMER_NAME);
	if (NULL == g_hRingWaiterTimer)
	{
		return 0;
	}
	while (!g_bIsExitRingNotifyThd)
	{
		if (TRUE == bIsReset)
		{
			SetTimeRingNotifyTimer();
		}
		bIsReset = TRUE;
		dwWaitStatus = WaitForSingleObjectEx(g_hRingWaiterTimer,MAX_WAIT_MILLISECONDS_VALUE,TRUE);
		if (WAIT_FAILED == dwWaitStatus)
		{
			break;
		}
		else if (WAIT_OBJECT_0 == dwWaitStatus)
		{
			NotifyTimeRingEvent();
		}
		else if (WAIT_TIMEOUT == dwWaitStatus)
		{
			//For changing system time.
			bIsReset = !IsSetTimeNotificationIntervalCorrect();
		}
	}
	return 1;
}

BOOL WINAPI IsSetTimeNotificationIntervalCorrect()
{
	BOOL		  bIsOK;
	FILETIME	  stCurrentFileTime;
	FILETIME	  stLastSetFileTime;
	LARGE_INTEGER stCurrentLITime;
	LARGE_INTEGER stLastSetLITime;
	LARGE_INTEGER stIntervalLITime;
	LARGE_INTEGER stConstantLITime;

	SYSTEMTIME	  stTempSysTime;
	RequestAccessDataEntryToken();
	stTempSysTime = g_TimeRingNotifyDataEntry.stGMT_LastSetTime;
	ReleaseAccessDataEntryToken();
	bIsOK = SystemTimeToFileTime(&stTempSysTime, &stLastSetFileTime);
	if (FALSE == bIsOK)
	{
		return FALSE;
	}
	stLastSetLITime.HighPart = stLastSetFileTime.dwHighDateTime;
	stLastSetLITime.LowPart = stLastSetFileTime.dwLowDateTime;

	GetSystemTime(&stTempSysTime);
	bIsOK = SystemTimeToFileTime(&stTempSysTime, &stCurrentFileTime);
	if (FALSE == bIsOK)
	{
		return FALSE;
	}
	stCurrentLITime.HighPart = stCurrentFileTime.dwHighDateTime;
	stCurrentLITime.LowPart = stCurrentFileTime.dwLowDateTime;
	stIntervalLITime.QuadPart = stCurrentLITime.QuadPart - stLastSetLITime.QuadPart;
	stConstantLITime.QuadPart = RING_TIME_PERIOD_MINUTE * 60;
	stConstantLITime.QuadPart = stConstantLITime.QuadPart * 1000 * 1000 * 10;
	if (_abs64(stIntervalLITime.QuadPart) > stConstantLITime.QuadPart)
	{
		//Set time ring notification invalid.
		return FALSE;
	}
	return TRUE;
}

VOID WINAPI NotifyTimeRingEvent()
{
	DWORD		dwTempTimeRingNotifyFlag = (DWORD)-1;
	FN_TIMERINGNOTIFYCALLBACK	lpFnProc = NULL;

	//whether system sleep to wake up status or not.
	if (FALSE == IsSetTimeNotificationIntervalCorrect())
	{
		AddDbgPrintStream(DBGFMTMSG(L"Set time ring notification invalid."), CURTID, USERMODE, FUNCNAME(L"NotifyTimeRingEvent"),
			DBG_WARNING);
		return;
	}
	RequestAccessDataEntryToken();
	lpFnProc = g_TimeRingNotifyDataEntry.lpTimeRingNotifyCallbackProc;
	dwTempTimeRingNotifyFlag = g_TimeRingNotifyDataEntry.dwTimeRingNotifyFlag;
	ReleaseAccessDataEntryToken();
	if (NULL == lpFnProc)
	{
		return;
	}
	AddDbgPrintStream(DBGFMTMSG(L"Call callback function to notify-Music id = %d !"), CURTID, USERMODE, FUNCNAME(L"NotifyTimeRingEvent"),
		DBG_TIPS, dwTempTimeRingNotifyFlag);
	lpFnProc(dwTempTimeRingNotifyFlag);
	return;
}
VOID CALLBACK DummyRingNotifyThreadAPCProc(ULONG_PTR dwParam)
{
	return;
}


//Export.
BOOL WINAPI RegisterTimeRingNotifyRoutine(PVOID	lpTimeRingNotifyRoutine)
{
	BOOL	bIsOK = FALSE;
	if (NULL == lpTimeRingNotifyRoutine)
	{
		return FALSE;
	}
	RequestAccessDataEntryToken();
	if (NULL != g_TimeRingNotifyDataEntry.lpTimeRingNotifyCallbackProc)
	{
		bIsOK = FALSE;
	}
	else
	{
		g_TimeRingNotifyDataEntry.lpTimeRingNotifyCallbackProc = lpTimeRingNotifyRoutine;
		bIsOK = TRUE;
	}
	ReleaseAccessDataEntryToken();
	if (TRUE == bIsOK)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Register time ring notify routine successfully!"), CURTID, USERMODE, FUNCNAME(L"RegisterTimeRingNotifyRoutine"), DBG_TIPS);
	}
	else
	{
		AddDbgPrintStream(DBGFMTMSG(L"Register time ring notify routine failed!"), CURTID, USERMODE, FUNCNAME(L"RegisterTimeRingNotifyRoutine"), DBG_ERROR);
	}
	return bIsOK;
}

//Export.
BOOL WINAPI UnregisterTimeRingNotifyRoutine(PVOID	lpOriginalTimeRingNotifyRoutine)
{
	BOOL	bIsOK = FALSE;
	if (NULL == lpOriginalTimeRingNotifyRoutine)
	{
		return FALSE;
	}
	RequestAccessDataEntryToken();
	if (lpOriginalTimeRingNotifyRoutine != g_TimeRingNotifyDataEntry.lpTimeRingNotifyCallbackProc)
	{
		bIsOK = FALSE;
	}
	else
	{
		g_TimeRingNotifyDataEntry.lpTimeRingNotifyCallbackProc = NULL;
		bIsOK = TRUE;
	}
	ReleaseAccessDataEntryToken();
	if (TRUE == bIsOK)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Unregister time ring notify routine successfully!"), CURTID, USERMODE, FUNCNAME(L"UnregisterTimeRingNotifyRoutine"), DBG_TIPS);
	}
	else
	{
		AddDbgPrintStream(DBGFMTMSG(L"Unregister time ring notify routine failed!"), CURTID, USERMODE, FUNCNAME(L"UnregisterTimeRingNotifyRoutine"), DBG_ERROR);
	}
	return bIsOK;
}

