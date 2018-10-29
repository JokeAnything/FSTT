#pragma once
#include <WINDOWS.H>
#include <STRSAFE.H>


#define		MAX_REG_PATH_LENGTH			(180)
#define		MAX_TEXT_CONTENT_SIZE		(120)

#define		REG_LOG_CONFIG_PATH			(L"SOFTWARE\\iWorkTime\\Configuration")
#define		REG_KEY_NAME				(L"Configuration")
#define		REG_RUNNING_VALUE_NAME		(L"iWorkTime")
#define		REG_STARTUP_RUNNING_PATH	(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define		AUTO_RUN_PARAM_HIDEWND		("/WndHide")

typedef struct _REG_CONFIGURATION
{	
	DWORD	dwIsStartupRunning;
	DWORD	dwIsTimeRing;
}REG_CONFIGURATION,*PREG_CONFIGURATION;


HKEY	WINAPI RetrieveRegKey(PWCHAR pszRegKeyPath);
BOOL	WINAPI SaveConfigurationToRegister(PREG_CONFIGURATION lpStConfiguration);
BOOL	WINAPI GetConfigurationFromRegister(PREG_CONFIGURATION lpStConfiguration);
BOOL	WINAPI ConfigureStartupRunning(BOOL bIsAdded);
BOOL	WINAPI QueryIsApplicationAutoRun(LPSTR lpCmdLine);


