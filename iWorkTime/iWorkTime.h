#pragma once
#include <windows.h>
#include "resource.h"
#include "systraylib.h"
#include "extrafnimpl.h"
#include "InsideWinClock.h"
#include "..\DataPoolImpl\FNImportDeclaration.h"
#include <strsafe.h>

#define NETWORK_EXCEPTION_TEXT (L"Network exception!")
#define NETWORK_DEFAULT_TEXT   (L"--:--:--")
#define HELP_UPDATE_LINK_INFO  (L"http://blog.csdn.net/drivewin/article/details/53727720")

#define NETWORK_TIME_UPDATE_TIMER_ID  (0)
#define NETWORK_TIME_UPDATE_CYCLE     (1000)

#define TASKBAR_TIME_UPDATE_TIMER_ID  (1)
#define TASKBAR_TIME_UPDATE_CYCLE     (250)

#define ADJUST_LT_TO_NT_TIMER_ID	  (2)
#define ADJUST_LT_TO_NT_UPDATE_CYCLE  (60000)

#define SWTASKBAR_BMP_WIDTH           (180)
#define SWTASKBAR_BMP_HEIGHT          (29)

#define	SPAWN_PSNAME			 (L"WebDataSynImpl.exe")


HICON	g_hDefaultCursor = NULL;
HICON	g_hHandCursor = NULL;
BOOL	g_bUpdateTimerInstalled = FALSE;
HWND    g_hAppFrameWnd = NULL;

LONG_PTR g_lpSWTASKBARBMPStaticCtrlProc = (LONG_PTR)NULL;

INT_PTR CALLBACK WinProcForAppFrameWnd(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK WinDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI Respond_InitDlgWnd_Msg(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
VOID WINAPI SetWindowInCentre(HWND hDlgWnd);
BOOL WINAPI Respond_CommandDlgWnd_Msg(hwndDlg, wParam, lParam);
BOOL WINAPI InsideWinClockModuleControl(BOOL	bIsStart, HWND	hMainCtrlWnd);
BOOL WINAPI IsX86RunningOnX64();
VOID WINAPI UpdateNetworkRealTime(HWND hDlgWnd);
VOID WINAPI UpdateLocalRealTime(HWND hDlgWnd);
VOID WINAPI AdjustLTToNTAuto(HWND hDlgWnd);
BOOL WINAPI SpawnWebDataSynImplApp();

VOID WINAPI CoverBitmapToControlWnd(HWND hwndDlg, UINT uCtrlWndId, PWCHAR lpBitmapName);
LRESULT CALLBACK SWASKBARBMPStaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
VOID WINAPI ExitApplication(HWND hwndDlg);

VOID WINAPI Respond_MouseMove_Msg(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
VOID WINAPI Respond_LBTNDown_Msg(HWND hwndDlg, WPARAM wParam, LPARAM lParam);
BOOL WINAPI IsHitWithinControlArea(HWND hwndDlg, LPPOINT pMouseClickClientPtn, UINT uCtrlId);

VOID WINAPI UpdateHoldRealTime(HWND hDlgWnd);
BOOL WINAPI GetMachineRunningTimeToString(PWCHAR pszStrBuff, INT nMaxCharLen);
BOOL WINAPI FormatMachineRunningTimeText(DWORD dwTotalSecHighPart, DWORD dwTotalSecLowPart, PWCHAR pszStrBuff, INT	nMaxCharLen);

typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
LPFN_ISWOW64PROCESS fnIsWow64Process;


extern HWND g_hTalkShowWnd;
extern UINT	g_uDesktopCrashMsg;
extern VOID WINAPI TimeRingNotificationProc(DWORD	dwNotifyFlag);

