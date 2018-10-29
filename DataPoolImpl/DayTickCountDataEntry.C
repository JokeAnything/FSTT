#define  _DAYTICKCOUNTDATAENTRY_GV
#include "DayTickCountDataEntry.h"
BOOL WINAPI InitializeDayTickCountDataEntry()
{
	BOOL	bIsLocal = FALSE;
	BOOL	bIsOK;
	RtlZeroMemory(&g_DayTickCountDataEntry, sizeof(DAY_TICKCOUNT_DATAENTRY));

	bIsOK = GetGMTNetworkTime(&g_DayTickCountDataEntry.stGMT_StartTime, &bIsLocal);
	if (FALSE == bIsOK)
	{
		GetSystemTime(&g_DayTickCountDataEntry.stGMT_StartTime);
	}
	g_DayTickCountDataEntry.bIsLocalGMTTime = bIsLocal;
	g_DayTickCountDataEntry.bIsPrecise = FALSE;
	GetMachineRunningTime(&g_DayTickCountDataEntry.liLastestTickCount);
	g_DayTickCountDataEntry.dwDayTotalTickCountCnt = 0;
	return TRUE;
}

BOOL WINAPI DestroyDayTickCountDataEntry()
{
	return TRUE;
}

VOID WINAPI UpdateDayTickCountDataEntry()
{
	BOOL	bIsOKLastestTime;
	BOOL	bIsOKNetTime;
	BOOL	bIsRetrieveLocalTime = TRUE;
	DWORD	dwIsLocalGMTTime = FALSE;
	SYSTEMTIME		stNetGMTTime;
	FILETIME		stFTNetGMTTime;
	FILETIME		stFTExistLocalGMTTime;
	FILETIME		stFTNowLocalGMTTime;

	LARGE_INTEGER	stStartTime;
	LARGE_INTEGER	stEndTime;
	LARGE_INTEGER	stIntervalTime;

	LARGE_INTEGER	newLILastestTickCount;

	RequestAccessDataEntryToken();
	dwIsLocalGMTTime = g_DayTickCountDataEntry.bIsLocalGMTTime;
	if (1 == dwIsLocalGMTTime)
	{
		SystemTimeToFileTime(&g_DayTickCountDataEntry.stGMT_StartTime, &stFTExistLocalGMTTime);
	}
	ReleaseAccessDataEntryToken();

	if (1 == dwIsLocalGMTTime)
	{
		bIsOKNetTime= GetGMTNetworkTime(&stNetGMTTime, &bIsRetrieveLocalTime);
		if (TRUE == bIsOKNetTime && FALSE == bIsRetrieveLocalTime)
		{
			bIsOKNetTime = SystemTimeToFileTime(&stNetGMTTime, &stFTNetGMTTime);
			if (TRUE == bIsOKNetTime)
			{
				GetSystemTimeAsFileTime(&stFTNowLocalGMTTime);
				stEndTime.LowPart = stFTNowLocalGMTTime.dwLowDateTime;
				stEndTime.HighPart = stFTNowLocalGMTTime.dwHighDateTime;
				stStartTime.LowPart = stFTExistLocalGMTTime.dwLowDateTime;
				stStartTime.HighPart = stFTExistLocalGMTTime.dwHighDateTime;
				stIntervalTime.QuadPart = stEndTime.QuadPart - stStartTime.QuadPart;

				stStartTime.LowPart = stFTNetGMTTime.dwLowDateTime;
				stStartTime.HighPart = stFTNetGMTTime.dwHighDateTime;
				stStartTime.QuadPart = stStartTime.QuadPart - stIntervalTime.QuadPart;

				stFTNetGMTTime.dwLowDateTime = stStartTime.LowPart;
				stFTNetGMTTime.dwHighDateTime = stStartTime.HighPart;
				bIsOKNetTime = FileTimeToSystemTime(&stFTNetGMTTime, &stNetGMTTime);
			}
		}
		else
		{
			bIsOKNetTime = FALSE;
		}
	}
	bIsOKLastestTime = GetMachineRunningTime(&newLILastestTickCount);

	RequestAccessDataEntryToken();
	if (TRUE == bIsOKLastestTime)
	{
		g_DayTickCountDataEntry.liLastestTickCount = newLILastestTickCount;
		bIsOKLastestTime = FALSE;
	}
	if (TRUE == bIsOKNetTime && FALSE == bIsRetrieveLocalTime)
	{
		g_DayTickCountDataEntry.stGMT_StartTime = stNetGMTTime;
		g_DayTickCountDataEntry.bIsLocalGMTTime = 0;
		bIsOKNetTime = FALSE;
	}
	ReleaseAccessDataEntryToken();

	return;
}

BOOL	WINAPI GetMachineRunningTime(PLARGE_INTEGER pLiData)
{
	BOOL	bIsOK;
	if (NULL == pLiData)
	{
		return	FALSE;
	}
	bIsOK = GetMachineRunningTime_MTD_0(pLiData);
	if (FALSE == bIsOK)
	{
		bIsOK = GetMachineRunningTime_MTD_1(pLiData);
	}
	return	bIsOK;
}

BOOL	WINAPI GetMachineRunningTime_MTD_0(PLARGE_INTEGER pLiData)
{
	BOOL		bResult = FALSE;
	INT64		i64BootTime = 0;
	INT64		i64CurrentTime = 0;
	INT64		dwTotalSeconds = 0;
	INT			dwDwordBitCnt = sizeof(DWORD) * 8;
	UINT		uMaskValue = (UINT)-1;
	NTSTATUS	ntStatus = -1;
	DWORD	dwLen = 0;
	SYSTEM_TIMEOFDAY_INFORMATION_EXTEND stSTI;
	FN_NTQUERYSYSTEMINFORMATION lpNtQuerySystemInformationProc = NULL;

	if (NULL == pLiData)
	{
		return	FALSE;
	}
	lpNtQuerySystemInformationProc = GetNtQuerySystemInformationFunctionPtr();
	if (NULL == lpNtQuerySystemInformationProc)
	{
		return FALSE;
	}
	RtlZeroMemory(&stSTI, sizeof(SYSTEM_TIMEOFDAY_INFORMATION_EXTEND));
	ntStatus = lpNtQuerySystemInformationProc(SystemTimeOfDayInformation, &stSTI, sizeof(SYSTEM_TIMEOFDAY_INFORMATION_EXTEND), &dwLen);
	if (ntStatus < 0)
	{
		return FALSE;
	}
	i64CurrentTime = stSTI.CurrentTime.HighPart;
	i64CurrentTime = (i64CurrentTime << dwDwordBitCnt) | stSTI.CurrentTime.LowPart;

	i64BootTime = stSTI.BootTime.HighPart;
	i64BootTime = (i64BootTime << dwDwordBitCnt) | stSTI.BootTime.LowPart;
	pLiData->QuadPart = (LONGLONG)(i64CurrentTime - i64BootTime);		//100ns as a unit.
	return TRUE;
}

//For compatibility,Win32 GetTickCount api instead of GetTickCount64 api
BOOL	WINAPI GetMachineRunningTime_MTD_1(PLARGE_INTEGER pLiData)
{
	BOOL				bResult = FALSE;
	DWORD				dwTotalSeconds = 0;
	static DWORD		dwRetrieveFNCount = 0;
	static FN_GETTICKCOUNT64	lpGetTickCount64FN = NULL;
	if (NULL == lpGetTickCount64FN && (dwRetrieveFNCount < 2) )
	{
		lpGetTickCount64FN = (FN_GETTICKCOUNT64)GetProcAddress(GetModuleHandle(L"KERNEL32.DLL"), STR_GETTICKCOUNT64);
		dwRetrieveFNCount++;
	}
	if (NULL != lpGetTickCount64FN)
	{
		pLiData->QuadPart = (LONGLONG)(lpGetTickCount64FN() * 1000 * 10);//100ns as a unit.
	}
	else
	{
		pLiData->QuadPart = (LONGLONG)(GetTickCount() * 1000 * 10); //100ns as a unit.
	}
	return TRUE;
}
FN_NTQUERYSYSTEMINFORMATION	WINAPI GetNtQuerySystemInformationFunctionPtr()
{
	HMODULE		hMod = NULL;
	FN_NTQUERYSYSTEMINFORMATION	lpProcReturnedPtr = NULL;

	hMod = GetModuleHandle(L"NTDLL.DLL");
	if (NULL == hMod)
	{
		return NULL;
	}
	lpProcReturnedPtr = (FN_NTQUERYSYSTEMINFORMATION)GetProcAddress(hMod, "NtQuerySystemInformation");
	return lpProcReturnedPtr;
}

BOOL	WINAPI GetMachineHoldOnTickCount(PLARGE_INTEGER pLiData)
{
	return GetMachineRunningTime(pLiData);
}

