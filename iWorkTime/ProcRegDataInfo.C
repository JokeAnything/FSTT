#include "PROCREGDATAINFO.H"

HKEY	WINAPI RetrieveRegKey(PWCHAR	pszRegKeyPath)
{
	LONG	lResult;
	HKEY	hSubKey;
	DWORD	dwDisp;
	if(NULL == pszRegKeyPath || pszRegKeyPath[0]==L'\0') 
	{
		return NULL;
	}
	hSubKey=NULL;
	dwDisp=0;
	lResult=RegCreateKeyEx(HKEY_CURRENT_USER,pszRegKeyPath,0,NULL,REG_OPTION_NON_VOLATILE,KEY_CREATE_SUB_KEY |KEY_READ | KEY_SET_VALUE,NULL,&hSubKey,&dwDisp);
	if(lResult!=ERROR_SUCCESS)
	{
		return NULL;
	}
	return hSubKey;
}

BOOL	WINAPI SaveConfigurationToRegister(PREG_CONFIGURATION lpStConfiguration)
{
	HKEY	hSubKey = NULL;
	LONG	lResult = ERROR_SUCCESS;
	DWORD	dwSavedSize = sizeof(REG_CONFIGURATION);
	BOOL	bIsOK=FALSE;
	hSubKey = RetrieveRegKey(REG_LOG_CONFIG_PATH);
	if((NULL == hSubKey) || (NULL == lpStConfiguration))
	{
		goto ERRORPATH;
	}
	lResult=RegSetValueEx(hSubKey,REG_KEY_NAME,0,REG_BINARY,(LPBYTE)(lpStConfiguration),dwSavedSize);	
	if(lResult != ERROR_SUCCESS)
	{
		bIsOK=FALSE;
	}
	else
	{
		bIsOK=TRUE;
	}
ERRORPATH:
	if(NULL != hSubKey)
	{
		RegCloseKey(hSubKey);
		hSubKey = NULL;
	}
	return bIsOK;
}
BOOL	WINAPI GetConfigurationFromRegister(PREG_CONFIGURATION lpStConfiguration)
{
	HKEY	hSubKey;
	LONG	lResult=ERROR_SUCCESS;
	DWORD	dwRecvSize=sizeof(REG_CONFIGURATION);
	DWORD	dwDataType=REG_BINARY;;
	BOOL	bIsOK = FALSE;
	hSubKey = RetrieveRegKey(REG_LOG_CONFIG_PATH);
	if((NULL == hSubKey) || (NULL == lpStConfiguration))
	{
		goto ERRORPATH;
	}
	lResult=RegQueryValueEx(hSubKey,REG_KEY_NAME,NULL,&dwDataType,(LPBYTE)lpStConfiguration,&dwRecvSize);
	if(lResult != ERROR_SUCCESS)
	{
		bIsOK = FALSE;
	}
	else
	{
		bIsOK = TRUE;
	}
ERRORPATH:
	if(NULL != hSubKey)
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
	HKEY	hSubKey = NULL ;
	DWORD	dwDisp;
	DWORD	dwReturnCnt;
	dwDisp = 0;
	lResult = RegCreateKeyEx(HKEY_CURRENT_USER, REG_STARTUP_RUNNING_PATH,0, NULL, REG_OPTION_NON_VOLATILE,KEY_SET_VALUE, NULL, &hSubKey, &dwDisp);
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
		StringCchPrintf(szKeyValue, sizeof(szKeyValue) / sizeof(WCHAR), L"%s %S", szAppFullPath ,AUTO_RUN_PARAM_HIDEWND);
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
BOOL WINAPI QueryIsApplicationAutoRun(LPSTR lpCmdLine)
{
	if (NULL == lpCmdLine || '\0' == lpCmdLine[0])
	{
		return FALSE;
	}
	if ( 0 == lstrcmpiA(lpCmdLine, AUTO_RUN_PARAM_HIDEWND) )
	{
		return TRUE;
	}
	return FALSE;
}