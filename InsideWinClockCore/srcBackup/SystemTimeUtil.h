#pragma once
#include <windows.h>
#include <winternl.h>
#include <strsafe.h>

typedef struct _SYSTEM_TIMEOFDAY_INFORMATION_EXTEND
{
	LARGE_INTEGER BootTime;		//8.
	LARGE_INTEGER CurrentTime;	//8.
	LARGE_INTEGER TimeZoneBias;	//8.
	ULONG TimeZoneId;			//4.
	ULONG Reserved;				//4.
	ULONGLONG BootTimeBias;		//8.
	ULONGLONG SleepTimeBias;	//8.
}SYSTEM_TIMEOFDAY_INFORMATION_EXTEND, *PSYSTEM_TIMEOFDAY_INFORMATION_EXTEND;


typedef NTSTATUS (WINAPI *FN_NTQUERYSYSTEMINFORMATION)(
	SYSTEM_INFORMATION_CLASS SystemInformationClass,
	PVOID SystemInformation,
	ULONG SystemInformationLength,
	PULONG ReturnLength
	);
BOOL	FormatSecondsToTimeText(DWORD dwTotalSecHighPart, DWORD dwTotalSecLowPart, PWCHAR pszStrBuff, INT	nMaxCharLen);
BOOL	GetMachineRunningTime_MTD_0(PWCHAR pszStrBuff, INT	nMaxCharLen);
BOOL	GetMachineRunningTime_MTD_1(PWCHAR pszStrBuff, INT	nMaxCharLen);
FN_NTQUERYSYSTEMINFORMATION	GetNtQuerySystemInformationFunctionPtr();

