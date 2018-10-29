#pragma once
#include <WINDOWS.H>
#include "DEDeclaration.h"

//RWS data section.
#pragma comment(linker,"/SECTION:.dATaPOL,RWS")

#define	DATAENTRY_ACCESS_TOKEN	(L"IwORkTImE_DAtAPOoL_TOkEn") 

typedef BOOL(WINAPI *FN_INITIALIZEPROC)();
typedef BOOL(WINAPI *FN_DESTROYPROC)();
typedef VOID(WINAPI *FN_UPDATEDATAENTRYPROC)();


BOOL WINAPI InitializeDataPool();
VOID WINAPI DestroyDataPool();
VOID WINAPI UpdateDataPool();

VOID WINAPI RequestAccessDataEntryToken();
VOID WINAPI ReleaseAccessDataEntryToken();

extern VOID WINAPI InitializeDbgStreamInfo();
extern VOID WINAPI DestroyDbgStreamInfo();

//DebugStreamInfoDataEntry
extern BOOL	 WINAPI InitializeDebugStreamInfoDataEntry(void);
extern BOOL	 WINAPI DestroyDebugStreamInfoDataEntry(void);
extern VOID	 WINAPI UpdateDebugStreamInfoDataEntry(void);

//MainConfigInfoDataEntry
extern BOOL WINAPI InitializeMainConfigInfoDataEntry(void);
extern BOOL WINAPI DestroyMainConfigInfoDataEntry(void);
extern VOID WINAPI UpdateMainConfigInfoDataEntry(void);

//NetworkTimeDataEntry
extern BOOL WINAPI	InitializeNetworkTimeDataEntry(void);
extern BOOL WINAPI  DestroyNetworkTimeDataEntry(void);
extern VOID WINAPI	UpdateNetworkTimeDataEntry(void);

//DayTickCountDataEntry
extern BOOL WINAPI InitializeDayTickCountDataEntry(void);
extern BOOL WINAPI DestroyDayTickCountDataEntry(void);
extern VOID WINAPI UpdateDayTickCountDataEntry(void);

//TimeRingNotifyDataEntry
extern BOOL	WINAPI InitializeTimeRingNotifyDataEntry(void);
extern BOOL	WINAPI DestroyTimeRingNotifyDataEntry(void);
extern VOID	WINAPI UpdateTimeRingNotifyDataEntry(void);

HANDLE					g_hDataEntryAccessToken = NULL;
BOOL                    g_bIsInitialized = FALSE;

FN_INITIALIZEPROC		g_pInitializeProcSet[] = {   (FN_INITIALIZEPROC)InitializeDebugStreamInfoDataEntry,
													 (FN_INITIALIZEPROC)InitializeMainConfigInfoDataEntry,
													 (FN_INITIALIZEPROC)InitializeNetworkTimeDataEntry,
													 (FN_INITIALIZEPROC)InitializeDayTickCountDataEntry,
													 (FN_INITIALIZEPROC)InitializeTimeRingNotifyDataEntry };
FN_DESTROYPROC			g_pDestroyProcSet[] = { (FN_DESTROYPROC)DestroyTimeRingNotifyDataEntry,
												(FN_DESTROYPROC)DestroyDayTickCountDataEntry,
												(FN_DESTROYPROC)DestroyNetworkTimeDataEntry,
												(FN_DESTROYPROC)DestroyMainConfigInfoDataEntry,
												(FN_DESTROYPROC)DestroyDebugStreamInfoDataEntry };
FN_UPDATEDATAENTRYPROC	g_pUpdateDataEntryProcSet[] = { (FN_UPDATEDATAENTRYPROC)UpdateDebugStreamInfoDataEntry,
														(FN_UPDATEDATAENTRYPROC)UpdateMainConfigInfoDataEntry,
														(FN_UPDATEDATAENTRYPROC)UpdateNetworkTimeDataEntry,
														(FN_UPDATEDATAENTRYPROC)UpdateDayTickCountDataEntry,
														(FN_UPDATEDATAENTRYPROC)UpdateTimeRingNotifyDataEntry};

