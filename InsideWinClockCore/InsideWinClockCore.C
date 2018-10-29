#include "InsideWinClockCore.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		g_hDllInstance = hinstDLL;
		DisableThreadLibraryCalls(hinstDLL);
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		if (TRUE == g_bIsInstalledHookOK)
		{
			RecoveryIatHookWorkForNormal(GetModuleHandle(NULL));//IAT Unhook
			g_bIsInstalledHookOK = FALSE;
		}
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

BOOL WINAPI	LoadCoreModuleForWinClock(HWND hHostWnd)
{
	HWND		hWinClockWnd;
	DWORD		dwThreadId;
	DWORD		dwProcessId;
	AddDbgPrintStream(DBGFMTMSG(L"Load core module for Win clock."), CURTID,USERMODE, FUNCNAME(L"LoadCoreModuleForWinClock"), DBG_TIPS);
	if (NULL == hHostWnd)
	{
		return FALSE;
	}
	hWinClockWnd = GetTrayNotifyClockWnd();

	if (NULL == hWinClockWnd)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Win clock window handle is missing."), CURTID, USERMODE, FUNCNAME(L"LoadCoreModuleForWinClock"), DBG_ERROR);
		return  FALSE;
	}
	g_hHostWndForIPC = hHostWnd;
	g_uInstall_Hook_Com_Msg = RegisterWindowMessage(INSTALL_HOOK_COM_MSG);
	g_uUninstall_Hook_Com_Msg = RegisterWindowMessage(UNINSTALL_HOOK_COM_MSG);
	dwThreadId = GetWindowThreadProcessId(hWinClockWnd, &dwProcessId);
	g_hCallProcHook = SetWindowsHookEx(WH_CALLWNDPROC, HookSentMsgWndProc, g_hDllInstance, dwThreadId);
	if (NULL == g_hCallProcHook)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Install win clock window message hook failed."), CURTID, USERMODE, FUNCNAME(L"LoadCoreModuleForWinClock"), DBG_ERROR);
		return FALSE;
	}
	//Valid hook proc immediately.
	SendMessage(hWinClockWnd, WM_PAINT, 0, 0);
	AddDbgPrintStream(DBGFMTMSG(L"Load win clock core module successfully."), CURTID, USERMODE, FUNCNAME(L"LoadCoreModuleForWinClock"), DBG_TIPS);
	return TRUE;
}

BOOL WINAPI	UnloadCoreModuleFromWinClock()
{
	BOOL	bIsOK = FALSE;
	if (NULL != g_hCallProcHook)
	{
		bIsOK = UnhookWindowsHookEx(g_hCallProcHook);
		g_hCallProcHook = NULL;
		AddDbgPrintStream(DBGFMTMSG(L"Unload win clock core module successfully."), CURTID, USERMODE, FUNCNAME(L"UnloadCoreModuleFromWinClock"), DBG_TIPS);
	}
	else
	{
		AddDbgPrintStream(DBGFMTMSG(L"Unload win clock core module failed,exception!"), CURTID, USERMODE, FUNCNAME(L"UnloadCoreModuleFromWinClock"), DBG_WARNING);
	}
	return bIsOK;
}

LRESULT CALLBACK HookSentMsgWndProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	BOOL	bIsFixIatHookOK = FALSE;
	PCWPSTRUCT	lpCWPS  = (PCWPSTRUCT)lParam;
	if (g_uInstall_Hook_Com_Msg == lpCWPS->message)
	{
		if(TRUE == InitializeIatHookInfoBuff() )
		{
			if (FALSE == g_bIsInstalledHookOK)
			{
				bIsFixIatHookOK = FixIatHookWorkForInside(GetModuleHandle(NULL));	//IAT Hook.
			}
			if (TRUE == bIsFixIatHookOK)
			{
				g_bIsInstalledHookOK = TRUE;
				AddDbgPrintStream(DBGFMTMSG(L"Fix iat successfully!"), CURTID, USERMODE, FUNCNAME(L"HookSentMsgWndProc"), DBG_TIPS);
			}
			else
			{
				AddDbgPrintStream(DBGFMTMSG(L"Fix iat failed!"), CURTID, USERMODE, FUNCNAME(L"HookSentMsgWndProc"), DBG_ERROR);
			}
		}
		else
		{
			AddDbgPrintStream(DBGFMTMSG(L"Iat initializes failed!"), CURTID, USERMODE, FUNCNAME(L"HookSentMsgWndProc"), DBG_ERROR);
		}
		return TRUE;
	}
	else if (g_uUninstall_Hook_Com_Msg == lpCWPS->message)
	{
		if (TRUE == g_bIsInstalledHookOK)
		{
			RecoveryIatHookWorkForNormal(GetModuleHandle(NULL));//IAT Unhook
			g_bIsInstalledHookOK = FALSE;
			AddDbgPrintStream(DBGFMTMSG(L"Iat has been fixed,but can not confirm correct!"), CURTID, USERMODE, FUNCNAME(L"HookSentMsgWndProc"), DBG_TIPS);
		}
		else
		{
			AddDbgPrintStream(DBGFMTMSG(L"Iat has NOT been fixed,but can not confirm correct!"), CURTID, USERMODE, FUNCNAME(L"HookSentMsgWndProc"), DBG_TIPS);
		}
		return TRUE;
	}
	return CallNextHookEx(g_hCallProcHook, nCode, wParam, lParam);
}

INT WINAPI GetTimeFormatEx_Transfer(LPCWSTR lpLocaleName, DWORD dwFlags, CONST SYSTEMTIME* lpTime, LPCTSTR lpFormat, LPTSTR lpTimeStr, INT cchTime)
{
	INT		nRetValue;
	HMODULE	hKernelModule;
	BOOL	bIsNativeApiOK = FALSE;
	WCHAR	szMachineWorkTime[100];
	static FN_GETTIMEFORMATEX lpGetTimeFormatEx = NULL;
	if (NULL == lpGetTimeFormatEx)
	{
		hKernelModule = GetModuleHandle(SYSTEM_KERNEL32_DLL_NAME);
		if (NULL != hKernelModule)
		{
			lpGetTimeFormatEx = (FN_GETTIMEFORMATEX)GetProcAddress(hKernelModule,"GetTimeFormatEx");
			bIsNativeApiOK = TRUE;
		}
		else
		{
			bIsNativeApiOK = FALSE;
		}
	}
	else
	{
		bIsNativeApiOK = TRUE;
	}
//	AddDbgPrintStream(DBGFMTMSG(L"GetTimeFormat_Transfer called!"), CURTID, USERMODE, FUNCNAME(L"GetTimeFormat_Transfer"), DBG_TIPS);
	if (TRUE == bIsNativeApiOK)
	{
		nRetValue = lpGetTimeFormatEx(lpLocaleName, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
	}
	else
	{
		nRetValue = GetTimeFormat((LCID)0x400, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
	}
	szMachineWorkTime[0] = L'\0';
	GetMachineRunningTimeToString(szMachineWorkTime, sizeof(szMachineWorkTime) / sizeof(WCHAR));
	StringCbCat(lpTimeStr, cchTime, szMachineWorkTime);
	nRetValue = (INT)wcsnlen(lpTimeStr, (size_t)cchTime) + 1;	//including a terminating null character.
//	AddDbgPrintStream(DBGFMTMSG(L"GetTimeFormatEx_Transfer called,Data had been reset!"), CURTID, USERMODE, FUNCNAME(L"GetTimeFormatEx_Transfer"), DBG_TIPS);
	return nRetValue;
}


INT WINAPI GetTimeFormat_Transfer(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME* lpTime, LPCTSTR lpFormat, LPTSTR lpTimeStr, INT cchTime)
{
	INT		nRetValue;
	WCHAR	szMachineWorkTime[100];

//	AddDbgPrintStream(DBGFMTMSG(L"GetTimeFormat_Transfer called!"), CURTID, USERMODE, FUNCNAME(L"GetTimeFormat_Transfer"), DBG_TIPS);

	nRetValue = GetTimeFormat(Locale, dwFlags, lpTime, lpFormat, lpTimeStr, cchTime);
	szMachineWorkTime[0] = L'\0';
	GetMachineRunningTimeToString(szMachineWorkTime, sizeof(szMachineWorkTime) / sizeof(WCHAR));
	StringCbCat(lpTimeStr, cchTime, szMachineWorkTime);
	nRetValue = (INT)wcsnlen(lpTimeStr, (size_t)cchTime) + 1;	//including a terminating null character.
//	AddDbgPrintStream(DBGFMTMSG(L"GetTimeFormat_Transfer called,Data had been reset!"), CURTID, USERMODE, FUNCNAME(L"GetTimeFormat_Transfer"), DBG_TIPS);
	return nRetValue;
}

BOOL WINAPI GetMachineRunningTimeToString(PWCHAR pszStrBuff, INT nMaxCharLen)
{
	BOOL			bIsOK;
	LARGE_INTEGER	stLI;
	LARGE_INTEGER	stLITemp;

	if (NULL == pszStrBuff || 0 == nMaxCharLen)
	{
		return FALSE;
	}
	bIsOK = GetMachineHoldOnTickCount(&stLI);//100ns as an unit.
	if (FALSE == bIsOK)
	{
		return FALSE;
	}
	stLITemp.QuadPart = 1000 * 1000 * 10;
	stLI.QuadPart = stLI.QuadPart / stLITemp.QuadPart;
	return FormatMachineRunningTimeText((DWORD)stLI.HighPart, stLI.LowPart, pszStrBuff, nMaxCharLen);
}
BOOL WINAPI FormatMachineRunningTimeText(DWORD dwTotalSecHighPart, DWORD dwTotalSecLowPart, PWCHAR pszStrBuff, INT	nMaxCharLen)
{
	DWORD		dwHours = 0;
	DWORD		dwMinutes = 0;
	DWORD		dwSeconds = 0;
	HRESULT		hResult;
	INT			dwDwordBitCnt = sizeof(DWORD) * 8;
	INT64		dwTotalSeconds = 0;

	if (NULL == pszStrBuff || nMaxCharLen <= 0)
	{
		return FALSE;
	}
	dwTotalSeconds = dwTotalSecHighPart;
	dwTotalSeconds = (dwTotalSeconds << dwDwordBitCnt) | dwTotalSecLowPart;

	dwSeconds = (DWORD)(dwTotalSeconds % 60);
	dwMinutes = (DWORD)((dwTotalSeconds / 60) % 60);
	dwHours = (DWORD)(dwTotalSeconds / 3600);
	hResult = StringCchPrintf(pszStrBuff, nMaxCharLen, L"  %02d:%02d:%02d ", dwHours, dwMinutes, dwSeconds);
	if (SUCCEEDED(hResult))
	{
		return TRUE;
	}
	return FALSE;
}


HWND WINAPI GetTaskBarWnd()
{
	HWND	hTempWnd;
	hTempWnd = FindWindow(TASKBAR_WND_CLASS_NAME, NULL);
	return hTempWnd;
}
HWND WINAPI GetTrayNotifyWndFromTaskBar()
{
	HWND	hTempWnd;
	hTempWnd = GetTaskBarWnd();
	if (NULL == hTempWnd)
	{
		return NULL;
	}
	hTempWnd = FindWindowEx(hTempWnd, NULL, TASKBAR_TRAY_NOTIFY_WND_CLASS_NAME, NULL);
	return hTempWnd;
}
//TrayClockWClass
HWND WINAPI GetTrayNotifyClockWnd()
{
	HWND	hTempWnd;
	HWND	hTrayNotifyWnd;
	hTrayNotifyWnd = GetTrayNotifyWndFromTaskBar();
	if (NULL == hTrayNotifyWnd)
	{
		return NULL;
	}
	hTempWnd = FindWindowEx(hTrayNotifyWnd, NULL, TASKBAR_TRAY_NOTIFY_CLKWND_CLASS_NAME, NULL);
	return hTempWnd;
}
