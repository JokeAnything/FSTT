#define		_PROCDBGSTREAM_G
#include	"PROCDBGSTREAM.H"

//Must _cdecl called type.
VOID	_cdecl  AddDbgPrintStream(PWCHAR	pszFormat,...)
{
	WCHAR				szText[MAX_PATH];
	va_list				argList;
#ifndef PRINT_DEBUG_INFO
	return;
#endif

	va_start(argList,pszFormat);
	RtlZeroMemory(szText,MAX_PATH*sizeof(WCHAR));
	StringCchVPrintf(szText,MAX_PATH,pszFormat,argList);
	va_end(argList);

	//Output system inner interface.
	OutputDebugString(szText);
	//Output log file.
	LoggingDebugStream(szText, (DWORD)wcslen(szText));
}

