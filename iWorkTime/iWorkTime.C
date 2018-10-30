#include "iWorkTime.h"

//#include "system/RCProcessCreator.h"
//#include "system/RCSystemUtils.h"
//#include "web/post/RCWebPost.h"
//#include "utils/RCBrowserInvoker.h"

INT  WINAPI WinMain(HINSTANCE hInstance, HINSTANCE lpPreInstance, LPSTR lpCmdLine, int nCmdShow)
{
	MSG					stMsg;
	BOOL				bIsSingleApp;
	BOOL				bIsWow64 = FALSE;
	BOOL				bIsHideWnd = FALSE;
	HWND				hMainDlg = NULL;
	HWND				hAppFrameWnd = NULL;
	WPARAM				wParam = 0;
	BOOL				bIsOK = FALSE;
	LPTOP_LEVEL_EXCEPTION_FILTER  lpFNExceptionPtr;

    SpawnWebDataSynImplApp();

	bIsWow64 = IsX86RunningOnX64();
	if (TRUE == bIsWow64)
	{
		MessageBox(NULL, L"Please running X64 directory application.", L"iWorkTime", MB_OK | MB_ICONWARNING);
		return 0;
	}
	lpFNExceptionPtr = SetUnhandledExceptionFilter((LPTOP_LEVEL_EXCEPTION_FILTER)ShowExcptInfo);
	//Just single instance.
	bIsSingleApp = CHECK_IS_SINGLE_INSTANCE(TRUE);
	if (FALSE == bIsSingleApp)
	{
		SetUnhandledExceptionFilter(lpFNExceptionPtr);
		return 0;
	}
	bIsOK = InitializeDataPool();
	if (FALSE == bIsOK)
	{
		MessageBox(NULL, L"Initialize data pool failed", L"Error", MB_ICONERROR | MB_OK);
		goto	EXIT;
	}
	bIsHideWnd = QueryIsApplicationAutoRun(lpCmdLine);
	//Create a unused dialog,for owner.
	hAppFrameWnd = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_APPFRAME_DLG), NULL, (DLGPROC)WinProcForAppFrameWnd);
	if (NULL == hAppFrameWnd)
	{
		MessageBox(NULL, L"Can not create assistance dialog", L"Error", MB_ICONERROR | MB_OK);
		goto	EXIT;
	}
	g_hAppFrameWnd = hAppFrameWnd;
	SetWindowPos(hAppFrameWnd,HWND_TOP, 0, 0, 0, 0, SWP_NOZORDER);

	//Create a main dialog.
	hMainDlg = CreateDialog(hInstance, MAKEINTRESOURCE(IDD_IWORKTIME_DLG), hAppFrameWnd,(DLGPROC)WinDlgProc);
	if (hMainDlg == NULL)
	{
		MessageBox(NULL, L"Can not create main dialog.", L"Error", MB_ICONERROR | MB_OK);
		goto	EXIT;
	}
	g_hTalkShowWnd = hMainDlg;
	if (TRUE == bIsHideWnd)
	{
		wParam = BN_CLICKED;
		wParam = (wParam << (sizeof(WORD)*8) ) | IDC_BTN_RUN;
		SendMessage(hMainDlg, WM_COMMAND, wParam, (LPARAM)NULL);
		wParam = SC_MINIMIZE;
		SendMessage(hMainDlg, WM_SYSCOMMAND, wParam, (LPARAM)NULL);
		ShowWindow(hMainDlg, SW_HIDE);
		ShowWindow(hAppFrameWnd, SW_HIDE);
		bIsHideWnd = FALSE;
		AddDbgPrintStream(DBGFMTMSG(L"Application runs by windows."), CURTID, USERMODE, FUNCNAME(L"WinMain"), DBG_TIPS);
	}
	else
	{
		ShowWindow(hAppFrameWnd, SW_SHOW);
		SetWindowPos(hMainDlg, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW|SWP_NOSIZE | SWP_NOMOVE);
		AddDbgPrintStream(DBGFMTMSG(L"Application runs by user."), CURTID, USERMODE, FUNCNAME(L"WinMain"), DBG_TIPS);
	}
	//Great!At this point,everything is ok now.
	while (GetMessage(&stMsg, NULL, 0, 0))
	{
		TranslateMessage(&stMsg);
		DispatchMessage(&stMsg);
	}
EXIT:
	AddDbgPrintStream(DBGFMTMSG(L"Starting to exit application UI!"), CURTID, USERMODE, FUNCNAME(L"WinMain"), DBG_TIPS);
	if (hMainDlg != NULL)
	{
		g_hTalkShowWnd = NULL;
		DestroyWindow(hMainDlg);
		hMainDlg = NULL;
	}
	if (hAppFrameWnd != NULL)
	{
		DestroyWindow(hAppFrameWnd);
		hAppFrameWnd = NULL;
	}
	AddDbgPrintStream(DBGFMTMSG(L"Starting to exit application DataPool!"), CURTID, USERMODE, FUNCNAME(L"WinMain"), DBG_TIPS);
	DestroyDataPool();
	CHECK_IS_SINGLE_INSTANCE(FALSE);
	SetUnhandledExceptionFilter(lpFNExceptionPtr);
	return 1;
}

INT_PTR CALLBACK WinProcForAppFrameWnd(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_CLOSE == uMsg)
	{
		PostMessage(g_hTalkShowWnd, WM_CLOSE, 0, 0);
		return FALSE;
	}
	return FALSE;
}

INT_PTR CALLBACK WinDlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT_PTR        nTimerID;
	BOOL			bReturned;
	if (uMsg == WM_TIMER)
	{
		nTimerID = (UINT_PTR)wParam;
		if (NETWORK_TIME_UPDATE_TIMER_ID == nTimerID)
		{
			UpdateLocalRealTime(hwndDlg);
			UpdateNetworkRealTime(hwndDlg);
			UpdateHoldRealTime(hwndDlg);
		}
		else if (TASKBAR_TIME_UPDATE_TIMER_ID == nTimerID)
		{
			RefreshWinClockWndAsyn();
		}
		else if (ADJUST_LT_TO_NT_TIMER_ID == nTimerID)
		{
			AdjustLTToNTAuto(hwndDlg);
		}
		return FALSE;
	}
	else if (uMsg == WM_COMMAND)
	{
		bReturned = Respond_CommandDlgWnd_Msg(hwndDlg, wParam, lParam);
		return bReturned;
	}
	else if (uMsg == WM_MOUSEMOVE)
	{
		Respond_MouseMove_Msg(hwndDlg, wParam, lParam);
		return TRUE;
	}
	else if (uMsg == WM_LBUTTONDOWN)
	{
		Respond_LBTNDown_Msg(hwndDlg, wParam, lParam);
		return TRUE;
	}
	else if (uMsg == UM_TRAY_MSG_NOTIFY)
	{
		if (lParam != WM_LBUTTONUP)
		{
			return TRUE;
		}
		SetWindowPos(hwndDlg, HWND_TOP, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOMOVE);
		ShowWindow(g_hAppFrameWnd, SW_SHOW);
		RemoveAppIconFromSystemTrayIconWnd();
		return	TRUE;
	}
	else if (uMsg == WM_CLOSE)
	{
		if (IDYES == MessageBox(hwndDlg, L"Do you want to exit this application?", L"Exit", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2))
		{
			ExitApplication(hwndDlg);
			PostQuitMessage(0);
		}
		return FALSE;
	}
	else if (uMsg == WM_SYSCOMMAND)
	{
		if (SC_MINIMIZE == wParam)
		{
			//SC_MINIMIZE == wParam
			ShowWindow(hwndDlg, SW_HIDE);
			ShowWindow(g_hAppFrameWnd, SW_HIDE);
			AddAppIconToSystemTrayIconWnd();
			return TRUE;
		}
		return FALSE;
	}
	else if (uMsg == WM_INITDIALOG)
	{
		bReturned = Respond_InitDlgWnd_Msg(hwndDlg, wParam, lParam);
		return bReturned;
	}
	else if (uMsg == WM_ENDSESSION)
	{
		//System will be shutt down or other action cause current app terminated, so save reg info.
		ExitApplication(hwndDlg);
		return FALSE;
	}
	else if (uMsg == g_uDesktopCrashMsg)
	{
		//Explorer application has been restart.
		if (TRUE == DetectWinClockWndIsExistence())
		{
			ReSetInsideWinClockCoreRunningStatus();
			InsideWinClockModuleControl(TRUE, hwndDlg);
		}
		ShowWindow(hwndDlg, SW_HIDE);
		ShowWindow(g_hAppFrameWnd, SW_HIDE);
		AddAppIconToSystemTrayIconWnd();
		return FALSE;
	}
	return FALSE;
}

BOOL WINAPI InsideWinClockModuleControl(BOOL	bIsStart,HWND	hMainCtrlWnd)
{
	if (FALSE == DetectWinClockWndIsExistence())
	{
		return FALSE;
	}
	if (TRUE == bIsStart)
	{
		if (TRUE == MgrWinClockCoreModuleAction(TRUE, hMainCtrlWnd))
		{
			if (FALSE == g_bUpdateTimerInstalled)
			{
				g_bUpdateTimerInstalled = TRUE;
				SetTimer(hMainCtrlWnd, TASKBAR_TIME_UPDATE_TIMER_ID, 250, NULL);
			}
			RefreshWinClockWndSyn();
			TryToResizeTaskbar();
		}
		else
		{
			return FALSE;
		}
	}
	else
	{
		if (TRUE == MgrWinClockCoreModuleAction(FALSE, hMainCtrlWnd))
		{
			TryToResizeTaskbar();
			RefreshWinClockWndSyn();
			if (TRUE == g_bUpdateTimerInstalled)
			{
				g_bUpdateTimerInstalled = FALSE;
				KillTimer(hMainCtrlWnd, TASKBAR_TIME_UPDATE_TIMER_ID);
			}
		}
		else
		{
			return FALSE;
		}
	}
	return TRUE;
}

VOID WINAPI ExitApplication(HWND hwndDlg)
{
	HWND  hCtrlBmpWnd = NULL; 
	if (NULL == hwndDlg)
	{
		return;
	}
	if ((LONG_PTR)NULL != g_lpSWTASKBARBMPStaticCtrlProc)
	{
		hCtrlBmpWnd = GetDlgItem(hwndDlg, IDC_STATIC_BMP);
		SetWindowLongPtr(hCtrlBmpWnd, GWLP_WNDPROC, (LONG_PTR)g_lpSWTASKBARBMPStaticCtrlProc);
	}
	InsideWinClockModuleControl(FALSE, hwndDlg);
	KillTimer(hwndDlg, ADJUST_LT_TO_NT_TIMER_ID);
	KillTimer(hwndDlg, NETWORK_TIME_UPDATE_TIMER_ID);
//	DestroyDataPool();
	return;
}

BOOL WINAPI Respond_InitDlgWnd_Msg(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL		 bIsOK;
	HICON		 hMainIcon;
	UINT		 uCheckFlag;
	HWND		 hCtrlTempWnd;
	MAIN_CONFIG_INFO_DATAENTRY stMCID;
	//Set main dialog icon.
	hMainIcon = LoadIcon((HINSTANCE)GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON_CLOCK));
	if (hMainIcon != NULL)
	{
		SendMessage(hwndDlg, WM_SETICON, (WPARAM)ICON_SMALL, (LPARAM)hMainIcon);
	}
	g_hDefaultCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
	SetCursor(g_hDefaultCursor);
	g_hHandCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));

	InitSysTrayLib(hwndDlg);

	bIsOK = GetMainConfigInfoDataEntry(&stMCID);
	if (TRUE == bIsOK)
	{
		1 == stMCID.dwIsStartupRunning ? (uCheckFlag = BST_CHECKED) : (uCheckFlag = BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_AUTORUN, uCheckFlag);

		1 == stMCID.dwIsSWWorkTimeOnTaskbar ? (uCheckFlag = BST_CHECKED) : (uCheckFlag = BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_SHOWWORKTIME, uCheckFlag);
		if (BST_CHECKED == uCheckFlag)
		{
			InsideWinClockModuleControl(TRUE, hwndDlg);
		}
		1 == stMCID.dwIsTimeRing ? (uCheckFlag = BST_CHECKED) : (uCheckFlag = BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_TIMERING, uCheckFlag);
		if (BST_CHECKED == uCheckFlag)
		{
			RegisterTimeRingNotifyRoutine((PVOID)TimeRingNotificationProc);
		}
		1 == stMCID.dwIsAdjustLTToNTAuto ? (uCheckFlag = BST_CHECKED) : (uCheckFlag = BST_UNCHECKED);
		CheckDlgButton(hwndDlg, IDC_CHECK_AUTOADJUSTTIME, uCheckFlag);
		if (BST_CHECKED == uCheckFlag)
		{
			SetTimer(hwndDlg, ADJUST_LT_TO_NT_TIMER_ID, ADJUST_LT_TO_NT_UPDATE_CYCLE, NULL);
		}
	}
	SetTimer(hwndDlg, NETWORK_TIME_UPDATE_TIMER_ID, NETWORK_TIME_UPDATE_CYCLE,NULL);
	hCtrlTempWnd = GetDlgItem(hwndDlg, IDC_STATIC_BMP);
	SetWindowPos(hCtrlTempWnd, HWND_TOP, 0, 0, SWTASKBAR_BMP_WIDTH,SWTASKBAR_BMP_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);
	g_lpSWTASKBARBMPStaticCtrlProc = (LONG_PTR)GetWindowLongPtr(hCtrlTempWnd, GWLP_WNDPROC);
	SetWindowLongPtr(hCtrlTempWnd, GWLP_WNDPROC, (LONG_PTR)SWASKBARBMPStaticWndProc);
	SetWindowText(GetDlgItem(hwndDlg, IDC_STATIC_LINKUS), HELP_UPDATE_LINK_INFO);
	SetWindowInCentre(hwndDlg);
	return FALSE;
}

VOID WINAPI Respond_MouseMove_Msg(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL	bIsHit;
	POINT	ptnCurrentPtn;
	ptnCurrentPtn.x = LOWORD(lParam);
	ptnCurrentPtn.y = HIWORD(lParam);
	bIsHit = IsHitWithinControlArea(hwndDlg, &ptnCurrentPtn, IDC_STATIC_LINKUS);
	if (TRUE == bIsHit)
	{
		SetCursor(g_hHandCursor);
	}
	else
	{
		SetCursor(g_hDefaultCursor);
	}
	return;
}

VOID WINAPI Respond_LBTNDown_Msg(HWND hwndDlg, WPARAM wParam, LPARAM lParam)
{
	BOOL	bIsHit;
	POINT	ptnCurrentPtn;
	ptnCurrentPtn.x = LOWORD(lParam);
	ptnCurrentPtn.y = HIWORD(lParam);

	bIsHit = IsHitWithinControlArea(hwndDlg, &ptnCurrentPtn, IDC_STATIC_LINKUS);
	if (TRUE == bIsHit)
	{
		ShellExecute(hwndDlg, L"open", HELP_UPDATE_LINK_INFO, NULL, NULL, SW_SHOWNORMAL);
		SetCursor(g_hHandCursor);
	}
	return;
}

BOOL WINAPI IsHitWithinControlArea(HWND hwndDlg, LPPOINT pMouseClickClientPtn, UINT uCtrlId)
{
	POINT	ptnCurrentPtn;
	POINT	ptnTargetClientPtn;
	SIZE	stSZ;
	HWND	hFocusWnd;
	RECT	stRect;

	if (NULL == hwndDlg || NULL == pMouseClickClientPtn)
	{
		return FALSE;
	}
	ptnCurrentPtn.x = pMouseClickClientPtn->x;
	ptnCurrentPtn.y = pMouseClickClientPtn->y;
	hFocusWnd = GetDlgItem(hwndDlg, uCtrlId);
	GetWindowRect(hFocusWnd, &stRect);
	stSZ.cx = stRect.right - stRect.left;
	stSZ.cy = stRect.bottom - stRect.top;

	ptnTargetClientPtn.x = stRect.left;
	ptnTargetClientPtn.y = stRect.top;
	ScreenToClient(hwndDlg, &ptnTargetClientPtn);
	if ((ptnCurrentPtn.x >= ptnTargetClientPtn.x) && (ptnCurrentPtn.x <= ptnTargetClientPtn.x + stSZ.cx)
		&& (ptnCurrentPtn.y >= ptnTargetClientPtn.y) && (ptnCurrentPtn.y <= ptnTargetClientPtn.y + stSZ.cy))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
	return FALSE;
}


LRESULT CALLBACK SWASKBARBMPStaticWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;
	if (WM_PAINT == uMsg)
	{
		hdc = BeginPaint(hwnd, &ps);
		CoverBitmapToControlWnd(GetParent(hwnd), IDC_STATIC_BMP, MAKEINTRESOURCE(IDB_BITMAP_SWTASKBAR));
		EndPaint(hwnd, &ps);
		return FALSE;
	}
	return CallWindowProc((WNDPROC)g_lpSWTASKBARBMPStaticCtrlProc,hwnd,uMsg,wParam,lParam);
}

VOID WINAPI CoverBitmapToControlWnd(HWND hwndDlg,UINT uCtrlWndId, PWCHAR lpBitmapName)
{
	HDC		hdcCtrlWnd;
	HDC     hDCCompatible;
	HBITMAP hBMPRsr;
	UINT    uCtrlWndWidth;
	UINT    uCtrlWndHeight;
	RECT    stRect;
	HWND	hCtrlTempWnd;
	if (NULL == hwndDlg || 0 == uCtrlWndId || NULL == lpBitmapName)
	{
		return;
	}
	hCtrlTempWnd = GetDlgItem(hwndDlg, uCtrlWndId);
	hdcCtrlWnd = GetDC(hCtrlTempWnd);
	GetWindowRect(hCtrlTempWnd, &stRect);
	uCtrlWndWidth = stRect.right - stRect.left;
	uCtrlWndHeight = stRect.bottom - stRect.top;

	hDCCompatible = CreateCompatibleDC(hdcCtrlWnd);
	hBMPRsr = LoadBitmap((HINSTANCE)GetModuleHandle(NULL), lpBitmapName);
	if (NULL == hDCCompatible || NULL == hBMPRsr)
	{
		goto EXIT;
	}
	SelectObject(hDCCompatible,(HGDIOBJ)hBMPRsr);
	StretchBlt(hdcCtrlWnd,0,0,uCtrlWndWidth, uCtrlWndHeight,hDCCompatible,
		0, 0,uCtrlWndWidth, uCtrlWndHeight,SRCCOPY);
EXIT:
	if (NULL != hBMPRsr)
	{
		DeleteObject((HGDIOBJ)hBMPRsr);
		hBMPRsr = NULL;
	}
	if (NULL != hDCCompatible)
	{
		DeleteObject((HGDIOBJ)hDCCompatible);
		hDCCompatible = NULL;
	}
	if (NULL != hdcCtrlWnd)
	{
		DeleteObject((HGDIOBJ)hdcCtrlWnd);
		hdcCtrlWnd = NULL;
	}
	return;
}
BOOL WINAPI Respond_CommandDlgWnd_Msg(HWND	hwndDlg, WPARAM	wParam, LPARAM lParam)
{
	UINT	uBtnState;
	BOOL	bIsSetOK;
	MAIN_CONFIG_INFO_DATAENTRY stMCID;
	if (LOWORD(wParam) == IDC_CHECK_SHOWWORKTIME && HIWORD(wParam) == BN_CLICKED)
	{
		uBtnState = IsDlgButtonChecked(hwndDlg, IDC_CHECK_SHOWWORKTIME);
		if (BST_CHECKED == uBtnState)
		{
			bIsSetOK = InsideWinClockModuleControl(TRUE, hwndDlg);
			stMCID.dwIsSWWorkTimeOnTaskbar = bIsSetOK;
		}
		else
		{
			bIsSetOK = InsideWinClockModuleControl(FALSE, hwndDlg);
			stMCID.dwIsSWWorkTimeOnTaskbar = (DWORD)!bIsSetOK;
		}
		SetMainConfigInfoDataEntry(&stMCID, MAIN_CONFIG_INFO_SWWORKTIMETSBAR_FIELD);
	}
	else if (LOWORD(wParam) == IDC_CHECK_AUTORUN && HIWORD(wParam) == BN_CLICKED)
	{
		uBtnState = IsDlgButtonChecked(hwndDlg, IDC_CHECK_AUTORUN);
		if (BST_CHECKED == uBtnState)
		{
			bIsSetOK = ConfigureStartupRunning(TRUE);
			stMCID.dwIsStartupRunning = (DWORD)bIsSetOK;
		}
		else
		{
			//BST_UNCHECKED
			bIsSetOK = ConfigureStartupRunning(FALSE);
			stMCID.dwIsStartupRunning = (DWORD)!bIsSetOK;
		}
		SetMainConfigInfoDataEntry(&stMCID, MAIN_CONFIG_INFO_STARTUPRUNNING_FIELD);
	}
	else if (LOWORD(wParam) == IDC_CHECK_TIMERING && HIWORD(wParam) == BN_CLICKED)
	{
		uBtnState = IsDlgButtonChecked(hwndDlg, IDC_CHECK_TIMERING);
		if (BST_CHECKED == uBtnState)
		{
			bIsSetOK = RegisterTimeRingNotifyRoutine((PVOID)TimeRingNotificationProc);
			stMCID.dwIsTimeRing = (DWORD)bIsSetOK;
		}
		else
		{
			//BST_UNCHECKED
			bIsSetOK = UnregisterTimeRingNotifyRoutine((PVOID)TimeRingNotificationProc);
			stMCID.dwIsTimeRing = (DWORD)!bIsSetOK;
		}
		SetMainConfigInfoDataEntry(&stMCID, MAIN_CONFIG_INFO_TIMERING_FIELD);
	}
	else if (LOWORD(wParam) == IDC_CHECK_AUTOADJUSTTIME && HIWORD(wParam) == BN_CLICKED)
	{
		uBtnState = IsDlgButtonChecked(hwndDlg, IDC_CHECK_AUTOADJUSTTIME);
		if (BST_CHECKED == uBtnState)
		{
			SetTimer(hwndDlg, ADJUST_LT_TO_NT_TIMER_ID, ADJUST_LT_TO_NT_UPDATE_CYCLE, NULL);
			stMCID.dwIsAdjustLTToNTAuto = (DWORD)1;
		}
		else
		{
			//BST_UNCHECKED
			KillTimer(hwndDlg, ADJUST_LT_TO_NT_TIMER_ID);
			stMCID.dwIsAdjustLTToNTAuto = (DWORD)0;
		}
		SetMainConfigInfoDataEntry(&stMCID, MAIN_CONFIG_INFO_ADJUST_LT_TO_NT);
	}
	return FALSE;
}

VOID WINAPI UpdateHoldRealTime(HWND hDlgWnd)
{
	BOOL bIsOK;
	WCHAR	szTempStrBuff[100];
	if (NULL == hDlgWnd)
	{
		return;
	}
	bIsOK = GetMachineRunningTimeToString(szTempStrBuff, sizeof(szTempStrBuff) / sizeof(WCHAR));
	if (TRUE == bIsOK)
	{
		SendMessage(GetDlgItem(hDlgWnd, IDC_STATIC_HOLD_TIME), WM_SETTEXT, 0, (LPARAM)szTempStrBuff);
	}
	return;
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
	hResult = StringCchPrintf(pszStrBuff, nMaxCharLen, L"Held time(H:M:S):\n%02d:%02d:%02d", dwHours, dwMinutes, dwSeconds);
	if (SUCCEEDED(hResult))
	{
		return TRUE;
	}
	return FALSE;
}

VOID WINAPI AdjustLTToNTAuto(HWND hDlgWnd)
{
	BOOL		bIsOK;
	BOOL		bIsLocalTime;
	SYSTEMTIME	stSysTime;

	if (NULL == hDlgWnd)
	{
		return;
	}
	bIsOK = GetGMTNetworkTime(&stSysTime, &bIsLocalTime);
	if (FALSE == bIsOK)
	{
		return;
	}
	if (TRUE == bIsLocalTime)
	{
		return;
	}
	SetSystemTime(&stSysTime);
	return;
}

VOID WINAPI UpdateLocalRealTime(HWND hDlgWnd)
{
	SYSTEMTIME	stLocalTime;
	WCHAR	   szFmtBuffer[100];
	if (NULL == hDlgWnd)
	{
		return;
	}
	GetLocalTime(&stLocalTime);
	StringCchPrintf(szFmtBuffer, sizeof(szFmtBuffer) / sizeof(WCHAR), L"%d/%02d/%02d %02d:%02d:%02d",
		stLocalTime.wYear,
		stLocalTime.wMonth,
		stLocalTime.wDay,
		stLocalTime.wHour,
		stLocalTime.wMinute,
		stLocalTime.wSecond);
	SendMessage(GetDlgItem(hDlgWnd, IDC_STATIC_LOCALTIME), WM_SETTEXT, 0, (LPARAM)szFmtBuffer);
	return;
}
VOID WINAPI UpdateNetworkRealTime(HWND hDlgWnd)
{
	BOOL		bIsOK;
	BOOL		bIsLocalTime;
	SYSTEMTIME	stSysTime;
	SYSTEMTIME	stLocalTime;

	WCHAR	   szFmtBuffer[100];
	if (NULL == hDlgWnd)
	{
		return;
	}
	bIsOK = GetGMTNetworkTime(&stSysTime, &bIsLocalTime);
	if (FALSE == bIsOK)
	{
		SendMessage(GetDlgItem(hDlgWnd, IDC_STATIC_NETWORKRTIME), WM_SETTEXT, 0, (LPARAM)NETWORK_DEFAULT_TEXT);
		return;
	}
	if (TRUE == bIsLocalTime)
	{
		SendMessage(GetDlgItem(hDlgWnd, IDC_STATIC_NETWORKRTIME), WM_SETTEXT, 0, (LPARAM)NETWORK_EXCEPTION_TEXT);
		return;
	}
	bIsOK = SystemTimeToTzSpecificLocalTime(NULL, &stSysTime, &stLocalTime);
	if (FALSE == bIsOK)
	{
		SendMessage(GetDlgItem(hDlgWnd, IDC_STATIC_NETWORKRTIME), WM_SETTEXT, 0, (LPARAM)NETWORK_DEFAULT_TEXT);
		return;
	}
	StringCchPrintf(szFmtBuffer, sizeof(szFmtBuffer)/sizeof(WCHAR), L"%d/%02d/%02d %02d:%02d:%02d",
		stLocalTime.wYear,
		stLocalTime.wMonth,
		stLocalTime.wDay,
		stLocalTime.wHour,
		stLocalTime.wMinute,
		stLocalTime.wSecond);
	SendMessage(GetDlgItem(hDlgWnd, IDC_STATIC_NETWORKRTIME), WM_SETTEXT, 0, (LPARAM)szFmtBuffer);
	return;
}
VOID WINAPI SetWindowInCentre(HWND hDlgWnd)
{
	RECT	rect;
	POINT	ptnPos;

	HDC hScreenDC = GetDC(NULL);
	UINT cxScreen = GetDeviceCaps(hScreenDC, HORZRES);
	UINT cyScreen = GetDeviceCaps(hScreenDC, VERTRES);

	GetWindowRect(hDlgWnd, &rect);

	ptnPos.x = (cxScreen / 2) - ((rect.right - rect.left) / 2);
	ptnPos.y = (cyScreen / 2) - ((rect.bottom - rect.top) / 2);

	SetWindowPos(hDlgWnd, HWND_TOP, ptnPos.x, ptnPos.y, 0, 0, SWP_NOSIZE|SWP_NOZORDER);
	ReleaseDC(NULL, hScreenDC);
	return;
}

BOOL WINAPI IsX86RunningOnX64()
{
	BOOL bIsWow64 = FALSE;
	fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress(GetModuleHandle(L"kernel32"), "IsWow64Process");
	if (NULL != fnIsWow64Process)
	{
		if (!fnIsWow64Process(GetCurrentProcess(), &bIsWow64))
		{
			return FALSE;
		}
	}
	return bIsWow64;
}

BOOL WINAPI SpawnWebDataSynImplApp()
{
    BOOL				bIsSpawnPsOK;
    WCHAR				szCmdLine[MAX_PATH];
    STARTUPINFO			stSI;
    PROCESS_INFORMATION stPI;

    //Command line.current direcoty.
    StringCchPrintf(szCmdLine, MAX_PATH, L"%s", SPAWN_PSNAME);
    RtlZeroMemory(&stSI, sizeof(STARTUPINFO));
    stSI.cb = sizeof(STARTUPINFO);
    RtlZeroMemory(&stPI, sizeof(PROCESS_INFORMATION));
    bIsSpawnPsOK = CreateProcess(NULL, szCmdLine, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &stSI, &stPI);
    if (bIsSpawnPsOK == FALSE || stPI.hProcess == NULL)
    {
        //		AddDbgPrintStream(DBGFMTMSG(L"WebDataSynImplApp application started NG."), CURTID, USERMODE, FUNCNAME(L"WinDlgProc"), DBG_ERROR);
        return FALSE;
    }
    if (stPI.hProcess != NULL)
    {
        CloseHandle(stPI.hProcess);
        stPI.hProcess = NULL;
    }
    if (stPI.hThread != NULL)
    {
        CloseHandle(stPI.hThread);
        stPI.hThread = NULL;
    }
    //	AddDbgPrintStream(DBGFMTMSG(L"WebDataSynImplApp application started OK."), CURTID, USERMODE, FUNCNAME(L"WinDlgProc"), DBG_TIPS);
    return TRUE;
}