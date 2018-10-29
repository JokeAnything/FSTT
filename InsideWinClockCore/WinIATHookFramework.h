#pragma once
#include	<windows.h>
#include	"ProcDbgStream.h"
#define		MAX_MODULE_NAME_LENGTH		(80)
#define		MAX_MODULE_API_NAME_LENGTH	(80)
#define		MAX_IATHOOK_INFO_ARRAY_SIZE (40)

typedef  struct _IATHOOKCELLINFO
{
	CHAR		szModuleName[MAX_MODULE_NAME_LENGTH];
	CHAR		szModuleApiName[MAX_MODULE_API_NAME_LENGTH];
	ULONG_PTR	pFnCurrentAddress;
	ULONG_PTR	pFnTransferNewAddress;
	BOOL		bIsHookedOK;
}IATHOOKCELLINFO,*PIATHOOKCELLINFO;


extern INT WINAPI  GetTimeFormat_Transfer(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME* lpTime, LPCTSTR lpFormat, LPTSTR lpTimeStr, int cchTime);
extern INT WINAPI  GetTimeFormatEx_Transfer(LPCWSTR lpLocaleName, DWORD dwFlags, CONST SYSTEMTIME* lpTime, LPCTSTR lpFormat, LPTSTR lpTimeStr, INT cchTime);

BOOL WINAPI InitializeIatHookInfoBuff();
VOID WINAPI RecoveryIatHookWorkForNormal(HMODULE	hFixedModule);
BOOL WINAPI FixIatHookWorkForInside(HMODULE	hFixedModule);
BOOL WINAPI ReplaceIATEntryBySpecifyModule(PCSTR pszTargetModName, ULONG_PTR pfnCurrent, ULONG_PTR pfnNew, ULONG_PTR uImageBase);
LPVOID WINAPI RetrieveImageDirectoryEntryVAAndSize(ULONG_PTR	uImageBase, PDWORD uEntryBlockSize, DWORD dwImageDirType);
VOID WINAPI LoadImageDiskFileForInitIATHookBuff(PWCHAR	pszImageFullPath);
VOID WINAPI FindImportApiByWalkImageImportTable(ULONG_PTR uMemoryPtr, PCHAR	pszImportApiFunctionName, PVOID	lpTransferProc);
DWORD WINAPI ConvertImageRVAToFOA(PIMAGE_SECTION_HEADER lpSectionBeginPtr, DWORD	dwImageSectionNum, DWORD dwRVA, DWORD	dwImageAlignSize);



#ifdef _WINIATHOOKFRAMEWORK_G

//Init 2 Apis.
PCHAR	g_pApiNameInfoArray[] =
{
	"GetTimeFormatW",
	"GetTimeFormatEx"
};

PVOID	g_pIatHookTransferAddress[] =
{
	(PVOID)GetTimeFormat_Transfer,
	(PVOID)GetTimeFormatEx_Transfer
};

IATHOOKCELLINFO g_IatHookInfoBuff[MAX_IATHOOK_INFO_ARRAY_SIZE];
INT	g_IatHookInfoValidCount = 0;

PIATHOOKCELLINFO g_lpIatHookInfoBuffPtr = g_IatHookInfoBuff;

#endif // _WINIATHOOKFRAMEWORK_G


