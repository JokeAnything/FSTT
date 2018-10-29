#pragma once
#include	"windows.h"
#include	"winiathookframework.h"
#include	"ProcDbgStream.h"
#include	"..\DataPoolImpl\FNImportDeclaration.h"
#include	"strsafe.h"

#define		TASKBAR_WND_CLASS_NAME						(L"Shell_TrayWnd")
#define		TASKBAR_TRAY_NOTIFY_WND_CLASS_NAME			(L"TrayNotifyWnd")
#define		TASKBAR_TRAY_NOTIFY_CLKWND_CLASS_NAME		(L"TrayClockWClass")

#define		INSTALL_HOOK_COM_MSG						(L"Install_Hook_Com_Msg_IWCLK")
#define		UNINSTALL_HOOK_COM_MSG						(L"Uninstall_Hook_Com_Msg_IWCLK")
#define		SYSTEM_KERNEL32_DLL_NAME					(L"KERNEL32.DLL")

HINSTANCE	g_hDllInstance = NULL;
BOOL		g_bIsInstalledHookOK = FALSE;

#pragma data_seg(".isdWClk")
HHOOK		g_hCallProcHook = NULL;
HWND		g_hHostWndForIPC = NULL;
UINT		g_uInstall_Hook_Com_Msg = 0;
UINT		g_uUninstall_Hook_Com_Msg = 0;
#pragma data_seg()


#pragma comment(linker,"/SECTION:.isdWClk,RWS")


BOOL WINAPI	LoadCoreModuleForWinClock(HWND hHostWnd);
BOOL WINAPI	UnloadCoreModuleFromWinClock();


HWND WINAPI GetTaskBarWnd();
HWND WINAPI GetTrayNotifyWndFromTaskBar();
HWND WINAPI GetTrayNotifyClockWnd();

INT WINAPI	GetTimeFormat_Transfer(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME* lpTime, LPCTSTR lpFormat, LPTSTR lpTimeStr, INT cchTime);
INT WINAPI  GetTimeFormatEx_Transfer(LPCWSTR lpLocaleName, DWORD dwFlags, CONST SYSTEMTIME* lpTime, LPCTSTR lpFormat, LPTSTR lpTimeStr, INT cchTime);

BOOL WINAPI GetMachineRunningTimeToString(PWCHAR pszStrBuff, INT nMaxCharLen);
BOOL WINAPI FormatMachineRunningTimeText(DWORD dwTotalSecHighPart, DWORD dwTotalSecLowPart, PWCHAR pszStrBuff, INT	nMaxCharLen);


LRESULT CALLBACK HookSentMsgWndProc(INT nCode, WPARAM wParam, LPARAM lParam);

typedef INT (WINAPI *FN_GETTIMEFORMATEX)(LPCWSTR lpLocaleName, DWORD dwFlags,CONST SYSTEMTIME* lpTime, LPCWSTR lpFormat, LPWSTR lpTimeStr, INT cchTime);