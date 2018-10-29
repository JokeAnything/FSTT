#ifndef		_EXTRAFNIMPL_H
#define		_EXTRAFNIMPL_H

#include	<windows.h>
#include    "ProcDbgStream.h"
#include	"SysTrayLib.h"
#include	<strsafe.h>

#define		IWORKTIME_SINGLE_INSTANCE	(L"ITSBAR_SINGLE_INSTANCE")


#ifdef _EXTRAFNIMPL_GV

#pragma data_seg("iWorkTme")
#pragma comment(linker,"/SECTION:iWorkTme,RWS")
HWND	g_hTalkShowWnd = NULL;
#pragma data_seg()

#endif

BOOL WINAPI JustSingleApplication(BOOL bIsCheck);
LONG WINAPI ShowExcptInfo(PEXCEPTION_POINTERS	pstExcptInfo);
VOID WINAPI ShowPreviewInstanceWnd();
BOOL WINAPI GetModuleBaseFileName(HMODULE hMod, PWCHAR pszBuff, DWORD dwMaxCharLen);

#define		CHECK_IS_SINGLE_INSTANCE(bIsCheck) (JustSingleApplication(bIsCheck))


#endif