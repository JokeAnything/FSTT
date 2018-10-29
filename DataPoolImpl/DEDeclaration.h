#pragma once
#include <WINDOWS.H>

//Data entry declaration header.

#define		MAIN_CONFIG_INFO_INDEX			(0)
#define		NETWORK_TIME_DATAENTRY_INDEX	(1)
#define		DAY_TICKCOUNT_DATAENTRY_INDEX	(2)
#define		TIME_RING_NOTIFY_INDEX			(3)
#define		DEBUG_STREAM_INFO_INDEX			(4)


#define		MAIN_CONFIG_INFO_ALL_FIELD			      (0x0)
#define		MAIN_CONFIG_INFO_STARTUPRUNNING_FIELD     (0x1)
#define		MAIN_CONFIG_INFO_TIMERING_FIELD		      (0x2)
#define     MAIN_CONFIG_INFO_SWWORKTIMETSBAR_FIELD    (0x4)

typedef	struct _MAIN_CONFIG_INFO_DATAENTRY
{
	DWORD			dwIsStartupRunning;
	DWORD			dwIsTimeRing;
	DWORD           dwIsSWWorkTimeOnTaskbar;
	ULONG_PTR		ptrReserve;
}MAIN_CONFIG_INFO_DATAENTRY,*PMAIN_CONFIG_INFO_DATAENTRY;

typedef struct _NETWORK_TIME_DATAENTRY
{
	SYSTEMTIME	stGMT_Time;
	DWORD		bIsGMTTimeValid;
	DWORD		dwTickcountStamp;
	DWORD		dwIncrementTickCount;
	ULONG_PTR	ptrReserve;
}NETWORK_TIME_DATAENTRY, *PNETWORK_TIME_DATAENTRY;

typedef struct _DAY_TICKCOUNT_DATAENTRY
{
	SYSTEMTIME		stGMT_StartTime;
	DWORD			bIsLocalGMTTime;//1:Local GMT time,2:Network GMT time.
	LARGE_INTEGER	liLastestTickCount; //100ns as a unit.
	DWORD			dwDayTotalTickCountCnt; //excluding lastest tick count.
	DWORD			bIsPrecise;
	ULONG_PTR		ptrReserve;
}DAY_TICKCOUNT_DATAENTRY,*PDAY_TICKCOUNT_DATAENTRY;

typedef struct _TIME_RING_NOTIFY_DATAENTRY
{
	SYSTEMTIME		stGMT_LastSetTime;
	DWORD			dwSliceLongFlag;
	PVOID			lpTimeRingNotifyCallbackProc;
	DWORD			dwTimeRingNotifyFlag;
	ULONG_PTR		ptrReserve;
}TIME_RING_NOTIFY_DATAENTRY,*PTIME_RING_NOTIFY_DATAENTRY;


#define DEBUG_STREAM_INFO_MAX_CHAR_BUFFER  (260)
typedef struct _DEBUG_STREAM_INFO_DATAENTRY
{
	DWORD			dwDebugStringCount;
	DWORD           bIsLoggingReady;
	WCHAR			szDbgStreamInfoBuff[DEBUG_STREAM_INFO_MAX_CHAR_BUFFER];
	ULONG_PTR		ptrReserve;
}DEBUG_STREAM_INFO_DATAENTRY,*PDEBUG_STREAM_INFO_DATAENTRY;
