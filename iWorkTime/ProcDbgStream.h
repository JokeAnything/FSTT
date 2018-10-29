#ifndef		_PROCDBGSTREAM_H
#define		_PROCDBGSTREAM_H
#include	<WINDOWS.H>
#include	"..\DataPoolImpl\FNImportDeclaration.h"
#include	<STRSAFE.H>

#define	PRINT_DEBUG_INFO

#define	KRNLMODE					L"KRNL"
#define USERMODE					L"USER"

#define DBG_WARNING					L"WARN"
#define DBG_ERROR					L"EROR"
#define DBG_TIPS					L"TIPS"

#define FUNCNAME(NAME)				L"iWorkTime.EXE-FN:"##NAME

#define DBGFMTMSG(CONTEXT)			L"[%02d:%03d][%04d][%s][%-48.48s]---><%s>:"##CONTEXT
//FUNCNAME(L"main")--->L"FN:main"

#define DBG_TIME_MS					((DWORD)GetTickCount()%1000)
#define DBG_TIME_SD					((DWORD)((GetTickCount()/1000)%60))
#define DBG_THRD_ID					((ULONG)GetCurrentThreadId())
#define CURTID						DBG_TIME_SD,DBG_TIME_MS,DBG_THRD_ID


VOID	_cdecl AddDbgPrintStream(PWCHAR	pszFormat,...);

#endif