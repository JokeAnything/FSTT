#define	 _INSIDEWINCLOCK_G
#include "InsideWinClock.h"

BOOL WINAPI MgrWinClockCoreModuleAction(BOOL	bIsInstall, HWND	hHostWnd)
{
	HMODULE			hDllModule;
	HWND			hClockWnd;
	UINT			uCommunicationMsg;
	BOOL			bIsOK = FALSE;

	FN_LOADCOREMODULEFORWINCLOCK	lpFNLoadCoreModuleForWinClock;
	FN_UNLOADCOREMODULEFROMWINCLOCK	lpFNUnloadCoreModuleFromTaskbar;
	hDllModule = LoadLibrary(INSIDEWINCLOCKCORE_DLL_PATH);
	if (NULL == hDllModule || NULL == hHostWnd)
	{
		return FALSE;
	}
	lpFNLoadCoreModuleForWinClock = (FN_LOADCOREMODULEFORWINCLOCK)GetProcAddress(hDllModule,"LoadCoreModuleForWinClock");
	lpFNUnloadCoreModuleFromTaskbar = (FN_UNLOADCOREMODULEFROMWINCLOCK)GetProcAddress(hDllModule,"UnloadCoreModuleFromWinClock");

	if (NULL == lpFNLoadCoreModuleForWinClock || NULL == lpFNUnloadCoreModuleFromTaskbar)
	{
		goto	EXIT;
	}
	hClockWnd = GetTrayNotifyClockWnd();
	if (TRUE == bIsInstall)
	{
		if (FALSE == bIsInsideWinClockCoreRunning)
		{
			bIsOK = lpFNLoadCoreModuleForWinClock(hHostWnd);
			uCommunicationMsg = RegisterWindowMessage(INSTALL_HOOK_COM_MSG);
			SendMessage(hClockWnd, uCommunicationMsg, 0, 0);
		}
	}
	else
	{
		if (TRUE == bIsInsideWinClockCoreRunning)
		{
			uCommunicationMsg = RegisterWindowMessage(UNINSTALL_HOOK_COM_MSG);
			SendMessage(hClockWnd, uCommunicationMsg, 0, 0);
			bIsOK = lpFNUnloadCoreModuleFromTaskbar();
		}
	}
	if (TRUE == bIsOK)
	{
		bIsInsideWinClockCoreRunning == FALSE ? (bIsInsideWinClockCoreRunning = TRUE) : (bIsInsideWinClockCoreRunning = FALSE);
		//Done text.
	}
EXIT:
	if (NULL != hDllModule)
	{
		FreeLibrary(hDllModule);
		hDllModule = NULL;
	}
	return bIsOK;

}
VOID WINAPI ReSetInsideWinClockCoreRunningStatus()
{
	bIsInsideWinClockCoreRunning = FALSE;
}
//Called from WM_TIMER message.
VOID WINAPI RefreshWinClockWndAsyn()
{
	static  INT	 nCalledCount = 0;
	HWND	hWinClockWnd;

	if (FALSE == bIsInsideWinClockCoreRunning)
	{
		return;
	}
	hWinClockWnd = GetTrayNotifyClockWnd();
	if (NULL == hWinClockWnd)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Win clock window handle is missing."), CURTID, USERMODE, FUNCNAME(L"RefreshWinClockWndAsyn"), DBG_ERROR);
		return;
	}
	PostMessage(hWinClockWnd, WM_TIMECHANGE, 0, 0);
	PostMessage(hWinClockWnd, WM_PAINT, 0, 0);
//	AddDbgPrintStream(DBGFMTMSG(L"WM_TIMECHANGE & WM_PAINT message have been posted."), CURTID, USERMODE, FUNCNAME(L"RefreshWinClockWndAsyn"), DBG_ERROR);
	nCalledCount++;
	if (nCalledCount >= DETECT_TASKBAR_RESIZE_CYCLE_CNT)
	{
		if (TRUE == DetectReAdjustTaskBarNecessary())
		{
			TryToResizeTaskbar();
		}
		nCalledCount = 0;
	}
	return;
}

VOID WINAPI RefreshWinClockWndSyn()
{
	HWND	hWinClockWnd;
	hWinClockWnd = GetTrayNotifyClockWnd();
	if (NULL == hWinClockWnd)
	{
		return;
	}
	SendMessage(hWinClockWnd, WM_TIMECHANGE, 0, 0);
	SendMessage(hWinClockWnd, WM_PAINT, 0, 0);
	return;
}
//This method can not support windows version below 5.1.
VOID WINAPI TryToResizeTaskbarTHD2()
{
	DWORD	dwBuffSize;
	DWORD	dwTryAsynAdjustCnt = 30;
	BOOL	bIsTrySyn = FALSE;
	LSTATUS lStatus;
	DWORD	dwValueType = REG_DWORD;
	DWORD	uOriginalValue;
	DWORD	uReverseValue;
	HWND	hRecvWnd = NULL;
	dwBuffSize = sizeof(DWORD);
	uOriginalValue = 0xFFFF;

	while (1)
	{
		lStatus = SHGetValue(HKEY_CURRENT_USER, TASKBAR_PROPERTY_KEY_PATH, TASKBAR_PROPERTY_KEY_NAME, &dwValueType, &uOriginalValue, &dwBuffSize);
		if (ERROR_SUCCESS != lStatus)
		{
			//Failed.
			AddDbgPrintStream(DBGFMTMSG(L"Retrieve taskbar small icon flag failed."), CURTID, USERMODE, FUNCNAME(L"TryToResizeTaskbarTHD2"), DBG_ERROR);
			break;
		}
		uReverseValue = (uOriginalValue <= 0 ? 1 : 0);
		SHSetValue(HKEY_CURRENT_USER, TASKBAR_PROPERTY_KEY_PATH, TASKBAR_PROPERTY_KEY_NAME, REG_DWORD, &uReverseValue, sizeof(DWORD));
		if (FALSE == bIsTrySyn)
		{
			AddDbgPrintStream(DBGFMTMSG(L"ASYN:Try to adjust taskbar."), CURTID, USERMODE, FUNCNAME(L"TryToResizeTaskbarTHD2"), DBG_WARNING);
			SendNotifyMessage(HWND_BROADCAST, WM_WININICHANGE, 0, 0);
		}
		else if(hRecvWnd != NULL)
		{
			AddDbgPrintStream(DBGFMTMSG(L"SYN:Try to adjust taskbar."), CURTID, USERMODE, FUNCNAME(L"TryToResizeTaskbarTHD2"), DBG_WARNING);
			SendMessage(hRecvWnd, WM_WININICHANGE, 0, 0);
		}
		uReverseValue = uOriginalValue;
		SHSetValue(HKEY_CURRENT_USER, TASKBAR_PROPERTY_KEY_PATH, TASKBAR_PROPERTY_KEY_NAME, REG_DWORD, &uReverseValue, sizeof(DWORD));
		if (FALSE == bIsTrySyn)
		{
			SendNotifyMessage(HWND_BROADCAST, WM_WININICHANGE, 0, 0);
		}
		else if (hRecvWnd != NULL)
		{
			SendMessage(hRecvWnd, WM_WININICHANGE, 0, 0);
			break;
		}
		if (TRUE == DetectReAdjustTaskBarNecessary())
		{
			AddDbgPrintStream(DBGFMTMSG(L"ASYN:Detected!Try to adjust taskbar,remain count:%02d."), CURTID, USERMODE, FUNCNAME(L"TryToResizeTaskbarTHD2"), DBG_WARNING,
				dwTryAsynAdjustCnt);
			dwTryAsynAdjustCnt--;
		}
		else
		{
			break;
		}
		if ( dwTryAsynAdjustCnt <= 0 )
		{
			bIsTrySyn = TRUE;
			hRecvWnd = GetTaskBarWnd();
		}
	}
	return;
}

//This method can not support windows version above 10.0.
VOID WINAPI TryToResizeTaskbarTHD1()
{
	AddAppIconToSystemTrayIconWnd();
	RemoveAppIconFromSystemTrayIconWnd();
}

VOID WINAPI TryToResizeTaskbar()
{
	TryToResizeTaskbarTHD1();
	TryToResizeTaskbarTHD2();
}

BOOL WINAPI DetectReAdjustTaskBarNecessary()
{
	HWND	hWinClockWnd = NULL;
	RECT	stWinClockWndCurRect;
	DWORD	dwTemp = 0;
	DWORD	dwWinClockWidth = 0;
	DWORD	dwWinClockCurWidth = 0;
	static DWORD dwResizeTaskbarCount = 0;

	if (FALSE == IsTaskbarLocatedTopOrBottomEdge())
	{
		return FALSE;
	}
	hWinClockWnd = GetTrayNotifyClockWnd();
	if (NULL == hWinClockWnd)
	{
		return FALSE;
	}
	GetWindowRect(hWinClockWnd, &stWinClockWndCurRect);
	dwWinClockCurWidth = stWinClockWndCurRect.right - stWinClockWndCurRect.left;

	dwTemp = (DWORD)SendMessage(hWinClockWnd, RETRIEVE_WINCLOCK_WH_MESSAGE, 0, 0);
	dwWinClockWidth = dwTemp & 0xFFFF;
	if (dwWinClockWidth > dwWinClockCurWidth + 6)
	{
		dwResizeTaskbarCount++;
		AddDbgPrintStream(DBGFMTMSG(L"Detact taskbar needs to be resized,times:%d ."), CURTID, USERMODE, FUNCNAME(L"DetectReAdjustTaskBarNecessary"),
			DBG_TIPS, dwResizeTaskbarCount);
		return TRUE;
	}
	return FALSE;
}

BOOL WINAPI IsTaskbarLocatedTopOrBottomEdge()
{
	HWND	hTaskBarWnd = NULL;
	RECT	stTaskBarRect;
	INT		nWidth;
	INT		nScreenWidth;
	HDC		hScreenDC;
	BOOL	bReturnedValue = FALSE;
	hTaskBarWnd = GetTaskBarWnd();
	if (NULL == hTaskBarWnd)
	{
		return FALSE;
	}
	GetWindowRect(hTaskBarWnd, &stTaskBarRect);
	nWidth = stTaskBarRect.right - stTaskBarRect.left;
	hScreenDC = GetDC(NULL);
	nScreenWidth = GetDeviceCaps(hScreenDC, HORZRES);

	if (abs(nWidth - nScreenWidth) <= WND_ABOUT_WIDTH_OFFSET)
	{
		//Taskbar was located in top or bottom edge.
		bReturnedValue = TRUE;
	}
	else
	{
		bReturnedValue = FALSE;
	}
	if (NULL != hScreenDC)
	{
		ReleaseDC(NULL, hScreenDC);
		hScreenDC = NULL;
	}
	return bReturnedValue;
}


//Maybe this function will be blocked at most 10s.
BOOL WINAPI DetectWinClockWndIsExistence()
{
	HWND	hWinClockWnd = NULL;
	INT		nLoopCount = 60;
	while ( (NULL == hWinClockWnd) && (nLoopCount > 0) )
	{
		hWinClockWnd = GetTrayNotifyClockWnd();
		if (NULL == hWinClockWnd)
		{
			Sleep(250);
		}
		nLoopCount--;
	}
	if (NULL != hWinClockWnd)
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