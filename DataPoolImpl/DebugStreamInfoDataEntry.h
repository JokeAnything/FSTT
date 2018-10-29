#pragma once
#include <Windows.h>
#include "DEDeclaration.h"
#include "ProcDbgStream.h"
#include "strsafe.h"

#define  LOGGING_FILE_NAME						(L"LogDbgRecord.txt")
#define  DEBUG_STREAM_INFO_EVENT_NAME_SD		(L"IwORkTImE_DAtAPOoL_DbgTKN_SD")
#define  DEBUG_STREAM_INFO_EVENT_NAME_RD		(L"IwORkTImE_DAtAPOoL_DbgTKN_RD")
#define  TEXT_NEW_LINE                          (L"\r\n")
#define  MAX_WAIT_MILLISECONDS					(6500)

#define	 MAX_WAIT_DETECT_DEADLOCK_MILLISECONDS	(15000)

typedef VOID(WINAPI *FN_GETNATIVESYSTEMINFO)(LPSYSTEM_INFO lpSysInfo);
typedef BOOL(WINAPI *FN_GETVERSIONEX)(LPOSVERSIONINFO lpVersionInfo);



BOOL	 WINAPI InitializeDebugStreamInfoDataEntry();
BOOL	 WINAPI DestroyDebugStreamInfoDataEntry();
VOID	 WINAPI UpdateDebugStreamInfoDataEntry();
DWORD	 WINAPI LoggingDbgStreamThreadProc(LPVOID lpParameter);
VOID	 WINAPI LoggingDebugStream(PWCHAR pszDbgString, DWORD dwStringLen);
VOID     CALLBACK DummyLoggingDbgStreamThreadAPCProc(ULONG_PTR dwParam);
VOID     WINAPI LoggingDebugStreamHeader();
BOOL     WINAPI GetModuleFileDirectory(HMODULE hMod, PWCHAR pszBuff, DWORD dwMaxCharCount);

extern	 VOID WINAPI RequestAccessDataEntryToken();
extern	 VOID WINAPI ReleaseAccessDataEntryToken();


#ifdef  _DEBUGSTREAMINFODATAENTRY_GV

HANDLE  g_hDbgStreamTokenSendData = NULL;
HANDLE  g_hDbgStreamTokenRecvData = NULL;
HANDLE  g_hLoggingDbgStreamThread = NULL;
BOOL	g_bIsExitLoggingDbgStreamThd = FALSE;

#pragma data_seg(".dATaPOL")
DEBUG_STREAM_INFO_DATAENTRY g_DebugStreamInfoDataEntry = { 0 };
BOOL    g_bIsDbgStreamHeaderProcessed = FALSE;
#pragma data_seg()

#endif