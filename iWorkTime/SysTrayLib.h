#include	<windows.h>
#include	"resource.h"
#include	<strsafe.h>

#define		UM_TRAY_MSG_NOTIFY	(WM_USER+1000)

#ifdef		_SYSTRAYLIB_G
LPCWSTR				g_szAppTipName = L"iWorkTime Application";
LPCWSTR				g_szWndTipName = L"TrayIcon";
UINT				g_uDesktopCrashMsg=0;
HICON				g_uTrayIcon=NULL;
NOTIFYICONDATA		g_stSTray;
BOOL				g_bIsTrayLibInit = FALSE;
#endif

VOID WINAPI InitSysTrayLib(HWND hMainDlgWnd);
VOID WINAPI AddAppIconToSystemTrayIconWnd();
VOID WINAPI RemoveAppIconFromSystemTrayIconWnd();



