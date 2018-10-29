#pragma once
#include <winsock2.h>
//#include <ws2tcpip.h>
#include "DEDeclaration.h"
#include "ProcDbgStream.h"
#include <strsafe.h>

#pragma comment(lib,"ws2_32.lib")

#define	   MAX_SLEEP_MILLISECOND_VALUE_LEVEL1	(3000)
#define	   MAX_SLEEP_MILLISECOND_VALUE_LEVEL2	(5000)
#define	   MAX_SLEEP_MILLISECOND_VALUE_LEVEL3	(10000)
#define	   MAX_SLEEP_MILLISECOND_VALUE_LEVEL4	(15000)
#define	   MAX_SLEEP_MILLISECOND_VALUE			(300000)
#define	   MAX_SLEEP_MILLISECOND_FAILXFACTOR	(5000)
#define	   MAX_SLEEP_MILLISECOND_FAILBASE		(10000)


BOOL	   WINAPI	InitializeNetworkTimeDataEntry();
BOOL	   WINAPI   DestroyNetworkTimeDataEntry();
VOID	   WINAPI	UpdateNetworkTimeDataEntry();

BOOL	   WINAPI   GetGMTNetworkTime(PSYSTEMTIME pSystemTime, PBOOL bIsLocalTime);

BOOL	   WINAPI	GetHostIPAddrByHostName(PCHAR pstrHostName, PCHAR szBuff, INT nBuffSize);
SYSTEMTIME WINAPI	ResolveTimeStringToSysTime(PCHAR pTimeString);
INT		   WINAPI	ResolveStringDate(PCHAR pDateString);
PCHAR	   WINAPI	StrStrInsensitive(PCCH str, PCCH strSearch);
INT		   WINAPI	RetriveDateInfoFromRespondContent(PCHAR pContent, PCHAR szDateBuff, INT nBuffSize);
BOOL	   WINAPI	GetNetworkTimeByHTTPProtocol(PSYSTEMTIME pSystemTime);
DWORD	   WINAPI	WorkThreadProc(LPVOID lpParameter);
VOID	   CALLBACK DummyAPCProc(ULONG_PTR dwParam);

extern	   VOID WINAPI RequestAccessDataEntryToken();
extern	   VOID WINAPI ReleaseAccessDataEntryToken();


#ifdef  _NETWORKTIMEDATAENTRY_GV
HANDLE	g_hNetworkTimeThread = NULL;
BOOL	g_bIsExitThread = FALSE;
PCHAR	g_szServerHostNameList[] =
{
	"www.baidu.com",
	"www.taobao.com",
	"www.jd.com",
	"www.qq.com"
};

#pragma data_seg(".dATaPOL")
NETWORK_TIME_DATAENTRY	g_NetworkTimeDataEntry = { 0 };
#pragma data_seg()

#endif

