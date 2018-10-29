#pragma once
#include <Windows.h>
#include "DEDeclaration.h"
#include "ProcDbgStream.h"
#include <strsafe.h>

#define		MAX_REG_PATH_LENGTH			(180)
#define		MAX_TEXT_CONTENT_SIZE		(120)

#define		REG_LOG_CONFIG_PATH			(L"SOFTWARE\\iWorkTime\\Configuration")
#define		REG_KEY_NAME				(L"Configuration")
#define		REG_RUNNING_VALUE_NAME		(L"iWorkTime")
#define		REG_STARTUP_RUNNING_PATH	(L"Software\\Microsoft\\Windows\\CurrentVersion\\Run")
#define		AUTO_RUN_PARAM_HIDEWND		("/WndHide")

BOOL WINAPI InitializeMainConfigInfoDataEntry();
BOOL WINAPI DestroyMainConfigInfoDataEntry();
VOID WINAPI UpdateMainConfigInfoDataEntry();

HKEY WINAPI RetrieveRegKey(PWCHAR pszRegKeyPath);
BOOL WINAPI SaveConfigurationToRegister();
BOOL WINAPI GetConfigurationFromRegister();
BOOL WINAPI ConfigureStartupRunning(BOOL bIsAdded);
BOOL WINAPI QueryIsApplicationAutoRun(LPSTR lpCmdLine);
BOOL WINAPI GetMainConfigInfoDataEntry(PMAIN_CONFIG_INFO_DATAENTRY pMCIDE);
BOOL WINAPI SetMainConfigInfoDataEntry(PMAIN_CONFIG_INFO_DATAENTRY pMCIDE, DWORD dwSetFieldFlag);

#ifdef _MAINCONFIGINFODATAENTRY_GV

#pragma data_seg(".dATaPOL")
MAIN_CONFIG_INFO_DATAENTRY g_MainConfigInfoDataEntry = { 0 };
#pragma data_seg()

#endif

extern	   VOID WINAPI RequestAccessDataEntryToken();
extern	   VOID WINAPI ReleaseAccessDataEntryToken();




