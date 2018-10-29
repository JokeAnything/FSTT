#pragma once
#include	<windows.h>
#include	<Mmsystem.h>
#include	"..\DataPoolImpl\FNImportDeclaration.h"
#include	"resource.h"
#include	<strsafe.h>

#pragma comment(lib, "Winmm.lib")

#define		MINUTES_RING_VALUE_UNIT		(15)	

VOID WINAPI TimeRingNotificationProc(DWORD	dwNotifyFlag);