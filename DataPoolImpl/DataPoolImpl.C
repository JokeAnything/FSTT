#include "DataPoolImpl.h"

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
	// Perform actions based on the reason for calling.
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		// Initialize once for each new process.
		// Return FALSE to fail DLL load.
		InitializeDbgStreamInfo();
		g_hDataEntryAccessToken = CreateMutex(NULL, FALSE,DATAENTRY_ACCESS_TOKEN);
		DisableThreadLibraryCalls(hinstDLL);
		break;

	case DLL_THREAD_ATTACH:
		// Do thread-specific initialization.
		break;

	case DLL_THREAD_DETACH:
		// Do thread-specific cleanup.
		break;

	case DLL_PROCESS_DETACH:
		// Perform any necessary cleanup.
		if (NULL != g_hDataEntryAccessToken)
		{
			CloseHandle(g_hDataEntryAccessToken);
			g_hDataEntryAccessToken = NULL;
		}
		DestroyDbgStreamInfo();
		break;
	}
	return TRUE;  // Successful DLL_PROCESS_ATTACH.
}

BOOL WINAPI InitializeDataPool()
{
	INT		iIndex = 0;
	INT		nArrayCount;
	if (TRUE == g_bIsInitialized)
	{
		return FALSE;
	}
	if (NULL == g_hDataEntryAccessToken)
	{
		return FALSE;
	}
	nArrayCount = sizeof(g_pInitializeProcSet) / sizeof(FN_INITIALIZEPROC);
	for (iIndex = 0; iIndex < nArrayCount; iIndex++)
	{
		g_pInitializeProcSet[iIndex]();
	}
	g_bIsInitialized = TRUE;
	return TRUE;
}


VOID WINAPI DestroyDataPool()
{
	INT		iIndex = 0;
	INT		nArrayCount;
	if (FALSE == g_bIsInitialized)
	{
		return;
	}
	nArrayCount = sizeof(g_pDestroyProcSet) / sizeof(FN_DESTROYPROC);
	for (iIndex = 0; iIndex < nArrayCount; iIndex++)
	{
		g_pDestroyProcSet[iIndex]();
	}
	g_bIsInitialized = FALSE;
	return;
}

VOID WINAPI UpdateDataPool()
{
	INT		iIndex = 0;
	INT		nArrayCount;
	nArrayCount = sizeof(g_pUpdateDataEntryProcSet) / sizeof(FN_UPDATEDATAENTRYPROC);
	for (iIndex = 0; iIndex < nArrayCount; iIndex++)
	{
		g_pUpdateDataEntryProcSet[iIndex]();
	}
	return;
}
VOID WINAPI RequestAccessDataEntryToken()
{
	if (NULL == g_hDataEntryAccessToken)
	{
		//Fatal error ocurrance.
	}
	WaitForSingleObject(g_hDataEntryAccessToken,INFINITE);
	//No such good error processing manner.
}

VOID WINAPI ReleaseAccessDataEntryToken()
{
	if (NULL == g_hDataEntryAccessToken)
	{
		//Fatal error ocurrance.
	}
	ReleaseMutex(g_hDataEntryAccessToken);
	//No such good error processing manner.
}