#define _MAINCONFIGINFODATAENTRY_GV
#include "MainConfigInfoDataEntry.h"

BOOL WINAPI InitializeMainConfigInfoDataEntry()
{
	//Reserve.
	RtlZeroMemory(&g_MainConfigInfoDataEntry,sizeof(MAIN_CONFIG_INFO_DATAENTRY));
	GetConfigurationFromRegister();
	AddDbgPrintStream(DBGFMTMSG(L"MainConfigInfoDataEntry initialized finished!"), CURTID, USERMODE, FUNCNAME(L"InitializeMainConfigInfoDataEntry"), DBG_TIPS);
	return TRUE;
}

BOOL WINAPI DestroyMainConfigInfoDataEntry()
{
	//Reserve.
	SaveConfigurationToRegister();
	return TRUE;
}

VOID WINAPI UpdateMainConfigInfoDataEntry()
{
	//Reserve.
	return;
}

HKEY	WINAPI RetrieveRegKey(PWCHAR	pszRegKeyPath)
{
	LONG	lResult;
	HKEY	hSubKey;
	DWORD	dwDisp;
	if (NULL == pszRegKeyPath || pszRegKeyPath[0] == L'\0')
	{
		return NULL;
	}
	hSubKey = NULL;
	dwDisp = 0;
	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, pszRegKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_CREATE_SUB_KEY | KEY_READ | KEY_SET_VALUE, NULL, &hSubKey, &dwDisp);
	if (lResult != ERROR_SUCCESS)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Opened configuration registered handler failed!"), CURTID, USERMODE, FUNCNAME(L"SaveConfigurationToRegister"), DBG_ERROR);
		return NULL;
	}
	return hSubKey;
}

BOOL	WINAPI SaveConfigurationToRegister()
{
	HKEY	hSubKey = NULL;
	LONG	lResult = ERROR_SUCCESS;
	DWORD	dwSavedSize = sizeof(MAIN_CONFIG_INFO_DATAENTRY);
	BOOL	bIsOK = FALSE;
	MAIN_CONFIG_INFO_DATAENTRY stMCID;
	hSubKey = RetrieveRegKey(REG_LOG_CONFIG_PATH);
	if ( NULL == hSubKey)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Retrieve configuration registered handler failed!"), CURTID, USERMODE, FUNCNAME(L"SaveConfigurationToRegister"), DBG_ERROR);
		goto ERRORPATH;
	}
	RequestAccessDataEntryToken();
	stMCID = g_MainConfigInfoDataEntry;
	ReleaseAccessDataEntryToken();
	lResult = RegSetValueEx(hSubKey, REG_KEY_NAME, 0, REG_BINARY, (LPBYTE)(&stMCID), dwSavedSize);
	if (lResult != ERROR_SUCCESS)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Save configuration to register failed!"), CURTID, USERMODE, FUNCNAME(L"SaveConfigurationToRegister"), DBG_ERROR);
		bIsOK = FALSE;
	}
	else
	{
		AddDbgPrintStream(DBGFMTMSG(L"Save configuration to register successfully!"), CURTID, USERMODE, FUNCNAME(L"SaveConfigurationToRegister"), DBG_TIPS);
		bIsOK = TRUE;
	}
ERRORPATH:
	if (NULL != hSubKey)
	{
		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}
	return bIsOK;
}
BOOL	WINAPI GetConfigurationFromRegister()
{
	HKEY	hSubKey;
	LONG	lResult = ERROR_SUCCESS;
	DWORD	dwRecvSize = sizeof(MAIN_CONFIG_INFO_DATAENTRY);
	DWORD	dwDataType = REG_BINARY;
	BOOL    bIsDefault = FALSE;
	BOOL	bIsOK = FALSE;
	MAIN_CONFIG_INFO_DATAENTRY stMCID;
	hSubKey = RetrieveRegKey(REG_LOG_CONFIG_PATH);
	if (NULL == hSubKey)
	{
		goto ERRORPATH;
	}
	lResult = RegQueryValueEx(hSubKey, REG_KEY_NAME, NULL, &dwDataType, (LPBYTE)&stMCID, &dwRecvSize);
	if (lResult != ERROR_SUCCESS || dwRecvSize != sizeof(MAIN_CONFIG_INFO_DATAENTRY))
	{
		stMCID.dwIsStartupRunning = 1;
		stMCID.dwIsSWWorkTimeOnTaskbar = 1;
		stMCID.dwIsTimeRing = 1;
		stMCID.dwIsAdjustLTToNTAuto = 1;
		bIsDefault = TRUE;
		bIsOK = TRUE;
		AddDbgPrintStream(DBGFMTMSG(L"Retrieved configuration information exception,returned default information!"), CURTID, USERMODE, FUNCNAME(L"GetConfigurationFromRegister"), DBG_WARNING);
	}
	else
	{
		bIsDefault = FALSE;
		bIsOK = TRUE;
		AddDbgPrintStream(DBGFMTMSG(L"Retrieved configuration information successfully!"), CURTID, USERMODE, FUNCNAME(L"GetConfigurationFromRegister"), DBG_TIPS);
	}
	RequestAccessDataEntryToken();
	g_MainConfigInfoDataEntry = stMCID;
	ReleaseAccessDataEntryToken();
	if (TRUE == bIsDefault)
	{
		SaveConfigurationToRegister();
	}
ERRORPATH:
	if (NULL != hSubKey)
	{
		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}
	return bIsOK;
}

BOOL WINAPI ConfigureStartupRunning(BOOL bIsAdded)
{
	LONG	lResult;
	BOOL	bIsOK = FALSE;
	WCHAR	szAppFullPath[MAX_PATH];
	WCHAR	szKeyValue[MAX_PATH + 16];//Extra 10 characters for command parameter.
	HKEY	hSubKey = NULL;
	DWORD	dwDisp;
	DWORD	dwReturnCnt;
	dwDisp = 0;
	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REG_STARTUP_RUNNING_PATH, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, NULL, &hSubKey, &dwDisp);
	if (lResult != ERROR_SUCCESS)
	{
		return FALSE;
	}
	if (TRUE == bIsAdded)
	{
		dwReturnCnt = GetModuleFileName(NULL, szAppFullPath, sizeof(szAppFullPath) / sizeof(WCHAR));
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			bIsOK = FALSE;
			goto EXIT;
		}
		StringCchPrintf(szKeyValue, sizeof(szKeyValue)/sizeof(WCHAR), L"%s %S", szAppFullPath, AUTO_RUN_PARAM_HIDEWND);
		dwReturnCnt = (DWORD)(wcslen(szKeyValue) + 1);
		lResult = RegSetValueEx(hSubKey, REG_RUNNING_VALUE_NAME, 0, REG_SZ, (LPBYTE)(szKeyValue), dwReturnCnt*sizeof(WCHAR));
		if (ERROR_SUCCESS == lResult)
		{
			bIsOK = TRUE;
		}
		else
		{
			bIsOK = FALSE;
		}
	}
	else
	{
		lResult = RegDeleteValue(hSubKey, REG_RUNNING_VALUE_NAME);
		if (ERROR_SUCCESS == lResult)
		{
			bIsOK = TRUE;
		}
		else
		{
			bIsOK = FALSE;
		}
	}
EXIT:
	if (NULL != hSubKey)
	{
		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}
	return bIsOK;
}

BOOL WINAPI GetMainConfigInfoDataEntry(PMAIN_CONFIG_INFO_DATAENTRY pMCIDE)
{
	if (NULL == pMCIDE)
	{
		return FALSE;
	}
	RequestAccessDataEntryToken();
	*pMCIDE = g_MainConfigInfoDataEntry;
	ReleaseAccessDataEntryToken();
	return TRUE;
}

BOOL WINAPI SetMainConfigInfoDataEntry(PMAIN_CONFIG_INFO_DATAENTRY pMCIDE, DWORD dwSetFieldFlag)
{
	BOOL	bIsOK = TRUE;
	if (NULL == pMCIDE)
	{
		return FALSE;
	}
	RequestAccessDataEntryToken();
	if (MAIN_CONFIG_INFO_ALL_FIELD == dwSetFieldFlag)
	{
		g_MainConfigInfoDataEntry = *pMCIDE;
	}
	else if (0 != (dwSetFieldFlag&MAIN_CONFIG_INFO_STARTUPRUNNING_FIELD) )
	{
		g_MainConfigInfoDataEntry.dwIsStartupRunning = pMCIDE->dwIsStartupRunning;
	}
	else if (0 != (dwSetFieldFlag&MAIN_CONFIG_INFO_TIMERING_FIELD) )
	{
		g_MainConfigInfoDataEntry.dwIsTimeRing = pMCIDE->dwIsTimeRing;
	}
	else if (0 != (dwSetFieldFlag&MAIN_CONFIG_INFO_SWWORKTIMETSBAR_FIELD))
	{
		g_MainConfigInfoDataEntry.dwIsSWWorkTimeOnTaskbar = pMCIDE->dwIsSWWorkTimeOnTaskbar;
	}
	else if (0 != (dwSetFieldFlag&MAIN_CONFIG_INFO_ADJUST_LT_TO_NT))
	{
		g_MainConfigInfoDataEntry.dwIsAdjustLTToNTAuto = pMCIDE->dwIsAdjustLTToNTAuto;
	}
	else
	{
		bIsOK = FALSE;
		//Do nothing here.
	}
	ReleaseAccessDataEntryToken();
	if (TRUE == bIsOK)
	{
		SaveConfigurationToRegister();
	}
	return bIsOK;
}


BOOL WINAPI QueryIsApplicationAutoRun(LPSTR lpCmdLine)
{
	BOOL  bStatus = FALSE;
	if (NULL == lpCmdLine || '\0' == lpCmdLine[0])
	{
		return FALSE;
	}
	if (0 == lstrcmpiA(lpCmdLine, AUTO_RUN_PARAM_HIDEWND))
	{
		bStatus = TRUE;
	}
	else
	{
		bStatus = FALSE;
	}
	AddDbgPrintStream(DBGFMTMSG(L"Automatic running£º%d"), CURTID, USERMODE, FUNCNAME(L"QueryIsApplicationAutoRun"), DBG_TIPS,bStatus);
	return bStatus;
}
