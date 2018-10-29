#pragma once
#include <windows.h>
#include <winternl.h>
#include "DEDeclaration.h"
#include <strsafe.h>

#define		STR_GETTICKCOUNT64		("GetTickCount64")
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

typedef ULONGLONG (WINAPI *FN_GETTICKCOUNT64)(VOID);

BOOL WINAPI InitializeDayTickCountDataEntry();
BOOL WINAPI DestroyDayTickCountDataEntry();
VOID WINAPI UpdateDayTickCountDataEntry();
BOOL WINAPI GetMachineHoldOnTickCount(PLARGE_INTEGER pLiData);

BOOL WINAPI GetMachineRunningTime(PLARGE_INTEGER pLiData);
BOOL WINAPI GetMachineRunningTime_MTD_0(PLARGE_INTEGER pLiData);
BOOL WINAPI GetMachineRunningTime_MTD_1(PLARGE_INTEGER pLiData);
FN_NTQUERYSYSTEMINFORMATION	WINAPI GetNtQuerySystemInformationFunctionPtr();


extern	   VOID WINAPI RequestAccessDataEntryToken();
extern	   VOID WINAPI ReleaseAccessDataEntryToken();
extern	   BOOL WINAPI GetGMTNetworkTime(PSYSTEMTIME pSystemTime, PBOOL bIsLocalTime);

#ifdef  _DAYTICKCOUNTDATAENTRY_GV

#pragma data_seg(".dATaPOL")
DAY_TICKCOUNT_DATAENTRY	g_DayTickCountDataEntry = { 0 };
#pragma data_seg()

#endif

