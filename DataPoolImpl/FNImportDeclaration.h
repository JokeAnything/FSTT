#pragma once
#include <windows.h>
#include "DEDeclaration.h"

#define		DLL_FN_DESCRIPTOR	__declspec(dllimport)


DLL_FN_DESCRIPTOR BOOL  WINAPI InitializeDataPool();
DLL_FN_DESCRIPTOR VOID  WINAPI DestroyDataPool();
DLL_FN_DESCRIPTOR VOID  WINAPI UpdateDataPool();

DLL_FN_DESCRIPTOR BOOL	WINAPI GetGMTNetworkTime(PSYSTEMTIME pSystemTime, PBOOL bIsLocalTime);
DLL_FN_DESCRIPTOR BOOL  WINAPI GetMachineHoldOnTickCount(PLARGE_INTEGER pLiData);
DLL_FN_DESCRIPTOR BOOL	WINAPI RegisterTimeRingNotifyRoutine(PVOID	lpTimeRingNotifyRoutine);
DLL_FN_DESCRIPTOR BOOL	WINAPI UnregisterTimeRingNotifyRoutine(PVOID	lpOriginalTimeRingNotifyRoutine);


DLL_FN_DESCRIPTOR BOOL WINAPI GetMainConfigInfoDataEntry(PMAIN_CONFIG_INFO_DATAENTRY pMCIDE);
DLL_FN_DESCRIPTOR BOOL WINAPI SetMainConfigInfoDataEntry(PMAIN_CONFIG_INFO_DATAENTRY pMCIDE, DWORD dwSetFieldFlag);
DLL_FN_DESCRIPTOR BOOL WINAPI QueryIsApplicationAutoRun(LPSTR lpCmdLine);
DLL_FN_DESCRIPTOR BOOL WINAPI ConfigureStartupRunning(BOOL bIsAdded);

DLL_FN_DESCRIPTOR VOID WINAPI LoggingDebugStream(PWCHAR pszDbgString, DWORD dwStringLen);


typedef	 VOID(WINAPI *FN_TIMERINGNOTIFYCALLBACK)(DWORD	dwNotifyFlag);
