#pragma once
#include <Windows.h>
#include "DEDeclaration.h"
#include "ProcDbgStream.h"

#define	 RING_TIME_PERIOD_MINUTE		(15)
#define	 RING_WAITTIMER_NAME			(L"IwORkTImE_DAtAPOoL_RIngWaItTiMer")
#define	 MAX_WAIT_MILLISECONDS_VALUE	(15000)


typedef	 VOID(WINAPI *FN_TIMERINGNOTIFYCALLBACK)(DWORD	dwNotifyFlag);

BOOL	WINAPI InitializeTimeRingNotifyDataEntry();
BOOL	WINAPI DestroyTimeRingNotifyDataEntry();
VOID	WINAPI UpdateTimeRingNotifyDataEntry();
BOOL	WINAPI RegisterTimeRingNotifyRoutine(PVOID	lpTimeRingNotifyRoutine);
BOOL	WINAPI UnregisterTimeRingNotifyRoutine(PVOID	lpOriginalTimeRingNotifyRoutine);

BOOL	WINAPI GetNextRingGMTTime(PSYSTEMTIME lpSysTime, PDWORD pdwTimeRingNotifyFlag);
BOOL	WINAPI SetTimeRingNotifyTimer();
DWORD	WINAPI RingNotifyThreadProc(LPVOID lpParameter);
VOID	WINAPI NotifyTimeRingEvent();
VOID	CALLBACK DummyRingNotifyThreadAPCProc(ULONG_PTR dwParam);
BOOL	WINAPI IsSetTimeNotificationIntervalCorrect();

extern	   VOID WINAPI RequestAccessDataEntryToken();
extern	   VOID WINAPI ReleaseAccessDataEntryToken();
extern	BOOL WINAPI GetGMTNetworkTime(PSYSTEMTIME pSystemTime, PBOOL bIsLocalTime);


#ifdef  _TIMERINGNOTIFYDATAENTRY_GV
HANDLE	 g_hRingWaiterTimer = NULL;
HANDLE	 g_hRingNotifyThread = NULL;
BOOL	 g_bIsExitRingNotifyThd = FALSE;

#pragma data_seg(".dATaPOL")
TIME_RING_NOTIFY_DATAENTRY g_TimeRingNotifyDataEntry = { 0 };
#pragma data_seg()

#endif