#include "SystemTimeUtil.h"

BOOL	GetMachineRunningTime_MTD_0(PWCHAR pszStrBuff, INT	nMaxCharLen)
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

	dwTotalSeconds = (i64CurrentTime - i64BootTime) / 10000000;		//100 ns as a unit.

	bResult = FormatSecondsToTimeText((DWORD)(dwTotalSeconds >> dwDwordBitCnt), dwTotalSeconds&uMaskValue, pszStrBuff, nMaxCharLen);
	return bResult;
}

//For compatibility,Win32 GetTickCount api instead of GetTickCount64 api
BOOL	GetMachineRunningTime_MTD_1(PWCHAR pszStrBuff, INT	nMaxCharLen)
{
	BOOL		bResult = FALSE;
	DWORD		dwTotalSeconds = 0;
	dwTotalSeconds = GetTickCount() / 1000;
	bResult = FormatSecondsToTimeText(0, dwTotalSeconds, pszStrBuff, nMaxCharLen);
	return bResult;
}


FN_NTQUERYSYSTEMINFORMATION	GetNtQuerySystemInformationFunctionPtr()
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
