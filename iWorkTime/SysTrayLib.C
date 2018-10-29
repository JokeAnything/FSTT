#define			_SYSTRAYLIB_G
#include		"SYSTRAYLIB.H"

VOID WINAPI InitSysTrayLib(HWND hMainDlgWnd)
{
	if (0 == g_uDesktopCrashMsg)
	{
		//Must specify 'TaskbarCreated' name
		g_uDesktopCrashMsg = RegisterWindowMessage(L"TaskbarCreated");
	}
	if (g_uTrayIcon == NULL)
	{
		g_uTrayIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_CLOCK));
	}
	RtlZeroMemory(&g_stSTray, sizeof(NOTIFYICONDATA));
	g_stSTray.cbSize = sizeof(g_stSTray);
	g_stSTray.hWnd = hMainDlgWnd;
	g_stSTray.uID = 0;
	g_stSTray.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	g_stSTray.uCallbackMessage = UM_TRAY_MSG_NOTIFY;
	g_stSTray.hIcon = g_uTrayIcon;
	StringCchCopy(g_stSTray.szTip, sizeof(g_stSTray.szTip) / sizeof(WCHAR), g_szAppTipName);
	g_bIsTrayLibInit = TRUE;
	return;
}
VOID WINAPI AddAppIconToSystemTrayIconWnd()
{
	if (FALSE == g_bIsTrayLibInit)
	{
		return;
	}
	Shell_NotifyIcon(NIM_ADD, &g_stSTray);
	return;
}
VOID WINAPI RemoveAppIconFromSystemTrayIconWnd()
{
	if (FALSE == g_bIsTrayLibInit)
	{
		return;
	}
	Shell_NotifyIcon(NIM_DELETE, &g_stSTray);
	return;
}

