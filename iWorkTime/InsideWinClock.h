#pragma once
#include <windows.h>
#include <shlwapi.h>
#include "ProcDbgStream.h"

#pragma comment(lib,"shlwapi.lib")

#define		INSIDEWINCLOCKCORE_DLL_PATH				(L"InsideWinClockCore.dll")

#define		TASKBAR_WND_CLASS_NAME					(L"Shell_TrayWnd")
#define		TASKBAR_TRAY_NOTIFY_WND_CLASS_NAME		(L"TrayNotifyWnd")
#define		TASKBAR_TRAY_NOTIFY_CLKWND_CLASS_NAME	(L"TrayClockWClass")


#define		INSTALL_HOOK_COM_MSG					(L"Install_Hook_Com_Msg_IWCLK")
#define		UNINSTALL_HOOK_COM_MSG					(L"Uninstall_Hook_Com_Msg_IWCLK")

#define		TASKBAR_PROPERTY_KEY_PATH				(L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced")
#define		TASKBAR_PROPERTY_KEY_NAME				(L"TaskbarSmallIcons")

#define		RETRIEVE_WINCLOCK_WH_MESSAGE			(0x464)
#define		WND_ABOUT_WIDTH_OFFSET					(26)
#define		DETECT_TASKBAR_RESIZE_CYCLE_CNT			(20)

typedef BOOL (WINAPI	*FN_LOADCOREMODULEFORWINCLOCK)(HWND hHostWnd);
typedef BOOL (WINAPI	*FN_UNLOADCOREMODULEFROMWINCLOCK)();


BOOL	WINAPI MgrWinClockCoreModuleAction(BOOL	bIsInstall, HWND	hHostWnd);
VOID	WINAPI RefreshWinClockWndAsyn();
VOID	WINAPI RefreshWinClockWndSyn();
HWND	WINAPI GetTaskBarWnd();
HWND	WINAPI GetTrayNotifyWndFromTaskBar();
HWND	WINAPI GetTrayNotifyClockWnd();
BOOL	WINAPI DetectWinClockWndIsExistence();
VOID	WINAPI TryToResizeTaskbarTHD1();
VOID	WINAPI TryToResizeTaskbarTHD2();
VOID	WINAPI TryToResizeTaskbar();
VOID	WINAPI ReSetInsideWinClockCoreRunningStatus();
BOOL	WINAPI DetectReAdjustTaskBarNecessary();
BOOL	WINAPI IsTaskbarLocatedTopOrBottomEdge();

extern	VOID WINAPI AddAppIconToSystemTrayIconWnd();
extern	VOID WINAPI RemoveAppIconFromSystemTrayIconWnd();


#ifdef  _INSIDEWINCLOCK_G

BOOL	bIsInsideWinClockCoreRunning = FALSE;

#endif





