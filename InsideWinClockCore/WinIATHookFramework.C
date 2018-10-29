#define	 _WINIATHOOKFRAMEWORK_G
#include "WinIATHookFramework.h"
LPVOID WINAPI RetrieveImageDirectoryEntryVAAndSize(ULONG_PTR	uImageBase, PDWORD uEntryBlockSize,DWORD dwImageDirType)
{
	LPVOID				lpResult = NULL;
	PIMAGE_NT_HEADERS	pNtHeader = NULL;
	PIMAGE_DOS_HEADER	pDosHeader = NULL;

	if (uImageBase <= 0x10000 || NULL == uEntryBlockSize)
	{
		return NULL;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)uImageBase;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return NULL;
	}
	pNtHeader = (PIMAGE_NT_HEADERS)(uImageBase + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		return NULL;
	}
	lpResult = (LPVOID)(uImageBase + pNtHeader->OptionalHeader.DataDirectory[dwImageDirType].VirtualAddress);
	*uEntryBlockSize = (DWORD)(uImageBase + pNtHeader->OptionalHeader.DataDirectory[dwImageDirType].Size);
	return lpResult;
}

BOOL WINAPI ReplaceIATEntryBySpecifyModule(PCSTR pszTargetModName,ULONG_PTR pfnCurrent, ULONG_PTR pfnNew, ULONG_PTR uImageBase)
{
	DWORD				dwSize = 0;
	BOOL				bResult = FALSE;
	PSTR				pszModName = NULL;
	DWORD				dwOldProtect = 0;
	ULONG_PTR			uTemp = 0;
	PULONG_PTR			uTempPtr = NULL;
	PIMAGE_THUNK_DATA	pThunk = NULL;
	PIMAGE_IMPORT_DESCRIPTOR	pImportDesc = NULL;
	if (NULL == pszTargetModName || pszTargetModName[0] == L'\0')
	{
		return FALSE;
	}
	if (pfnCurrent <= 0x10000 || pfnNew <= 0x10000 || uImageBase <= 0x10000 )
	{
		return FALSE;
	}
	__try 
	{
		bResult = FALSE;
		pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)RetrieveImageDirectoryEntryVAAndSize(uImageBase, &dwSize, IMAGE_DIRECTORY_ENTRY_IMPORT);
		if (NULL == pImportDesc)
		{
			// This module has no import section or is no longer loaded			
			__leave;  
		}
		for (; pImportDesc->Name != 0; pImportDesc++) 
		{
			pszModName = (PSTR)(uImageBase + pImportDesc->Name);
			if (lstrcmpiA(pszModName, pszTargetModName) != 0)
			{
				continue;
			}
			// Hit module name.
			pThunk = (PIMAGE_THUNK_DATA)(uImageBase + pImportDesc->FirstThunk);
			// Replace current function address with new function address
			for (; pThunk->u1.Function != 0; pThunk++) 
			{
				uTemp = (ULONG_PTR)pThunk->u1.Function;
				uTempPtr = (PULONG_PTR)&pThunk->u1.Function;
				if (uTemp != pfnCurrent)
				{
					continue;
				}
				//uTemp == pfnCurrent 
				if (VirtualProtect(uTempPtr, sizeof(PULONG_PTR),PAGE_WRITECOPY,&dwOldProtect))
				{
					*uTempPtr = pfnNew;
					bResult = TRUE;
					VirtualProtect(uTempPtr, sizeof(PULONG_PTR), dwOldProtect,&dwOldProtect);
					break; // break loop
				}
			}
			if (TRUE == bResult)
			{
				break;	//break loop
			}
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) 
	{
		bResult = FALSE;
	}
	return	bResult;
}

VOID WINAPI LoadImageDiskFileForInitIATHookBuff(PWCHAR	pszImageFullPath)
{
	HANDLE	hFile = NULL;
	INT		nApiNameCount = 0;
	INT		iIndex = 0;
	HANDLE	lpHeapMemoryPtr = NULL;
	DWORD	dwFileSize = 0;
	DWORD	dwReadedCount = 0;
	DWORD	dwReadedTimeCount = 0;
	DWORD	dwNeedReadedCount = 0;
	BOOL	bIsOK = FALSE;
	if (NULL == pszImageFullPath || L'\0' == pszImageFullPath[0])
	{
		return;
	}
	hFile = CreateFile(pszImageFullPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (INVALID_HANDLE_VALUE == hFile)
	{
		return;
	}
	dwFileSize = GetFileSize(hFile, NULL);
	if (INVALID_FILE_SIZE == dwFileSize)
	{
		goto EXIT;
	}
	lpHeapMemoryPtr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwFileSize);
	if (NULL == lpHeapMemoryPtr)
	{
		goto EXIT;
	}
	dwNeedReadedCount = dwFileSize;
	while (dwReadedCount < dwFileSize)
	{
		bIsOK = ReadFile(hFile, (LPVOID)((LPBYTE)lpHeapMemoryPtr + dwReadedCount), dwNeedReadedCount, &dwReadedTimeCount, NULL);
		if (FALSE == bIsOK)
		{
			break;
		}
		dwNeedReadedCount = dwNeedReadedCount - dwReadedTimeCount;
		dwReadedCount = dwReadedCount + dwReadedTimeCount;
	}
	if (dwReadedCount < dwFileSize)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Read image file[%s] failed!"), CURTID, USERMODE, FUNCNAME(L"FindImportApiByWalkImageImportTable"),
			DBG_ERROR, pszImageFullPath);
		goto  EXIT;
	}
	//Find every api name.
	nApiNameCount = sizeof(g_pApiNameInfoArray) / sizeof(PCHAR);
	g_IatHookInfoValidCount = 0;
	for (iIndex = 0; iIndex < nApiNameCount; iIndex++)
	{
		FindImportApiByWalkImageImportTable((ULONG_PTR)lpHeapMemoryPtr, g_pApiNameInfoArray[iIndex], g_pIatHookTransferAddress[iIndex]);
	}
EXIT:
	if (NULL != lpHeapMemoryPtr)
	{
		HeapFree(GetProcessHeap(), 0, lpHeapMemoryPtr);
		lpHeapMemoryPtr = NULL;
	}
	if (NULL != hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;
	}
	return;
}

VOID WINAPI FindImportApiByWalkImageImportTable(ULONG_PTR uMemoryPtr, PCHAR	pszImportApiFunctionName,PVOID	lpTransferProc)
{
	PIMAGE_NT_HEADERS			pNtHeader = NULL;
	PIMAGE_DOS_HEADER			pDosHeader = NULL;
	PIMAGE_SECTION_HEADER		pSectionBeginPtr = NULL;
	DWORD						dwImportTableRVA = 0;
	DWORD						dwImportTableSize = 0;
	DWORD						dwOptionalHeaderSize = 0;
	DWORD						dwImageAlignSize = 0;
	DWORD						dwImageSecNumber = 0;
	ULONG_PTR					uNtHeaderStartRVAPos = 0;
	ULONG_PTR					uImageSectionPos = 0;
	ULONG_PTR					uImportTableMemoryPtr = 0;
	PIMAGE_THUNK_DATA			pThunk = NULL;
	PIMAGE_IMPORT_DESCRIPTOR	pImportDesc = NULL;
	PIMAGE_IMPORT_BY_NAME		pImportApiName = NULL;
	DWORD						dwFOA = 0;
	PSTR						pszModName = NULL;
	if (0 == uMemoryPtr || NULL == pszImportApiFunctionName || NULL == lpTransferProc)
	{
		return;
	}
	pDosHeader = (PIMAGE_DOS_HEADER)uMemoryPtr;
	if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE)
	{
		return;
	}
	uNtHeaderStartRVAPos = pDosHeader->e_lfanew;
	pNtHeader = (PIMAGE_NT_HEADERS)(uMemoryPtr + pDosHeader->e_lfanew);
	if (pNtHeader->Signature != IMAGE_NT_SIGNATURE)
	{
		return;
	}
	dwImportTableRVA = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress;
	dwImportTableSize = pNtHeader->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size;
	dwImageSecNumber = (DWORD)pNtHeader->FileHeader.NumberOfSections;
	dwOptionalHeaderSize =(DWORD)(pNtHeader->FileHeader.SizeOfOptionalHeader);
	dwImageAlignSize = pNtHeader->OptionalHeader.SectionAlignment;
	uImageSectionPos = uMemoryPtr + uNtHeaderStartRVAPos + sizeof(pNtHeader->Signature) + sizeof(pNtHeader->FileHeader) + dwOptionalHeaderSize;
	pSectionBeginPtr = (PIMAGE_SECTION_HEADER)uImageSectionPos;

	dwFOA = ConvertImageRVAToFOA(pSectionBeginPtr, dwImageSecNumber, dwImportTableRVA, dwImageAlignSize);
	if (0 == dwFOA)
	{
		return;
	}
	uImportTableMemoryPtr = uMemoryPtr + dwFOA;
	pImportDesc = (PIMAGE_IMPORT_DESCRIPTOR)uImportTableMemoryPtr;
	for (; pImportDesc->Name != 0; pImportDesc++)
	{
		dwFOA = ConvertImageRVAToFOA(pSectionBeginPtr, dwImageSecNumber, pImportDesc->Name, dwImageAlignSize);
		pszModName = (PSTR)(uMemoryPtr + dwFOA);
		// Hit module name.
		dwFOA = ConvertImageRVAToFOA(pSectionBeginPtr, dwImageSecNumber, pImportDesc->OriginalFirstThunk, dwImageAlignSize);
		pThunk = (PIMAGE_THUNK_DATA)(uMemoryPtr + dwFOA);

		for (; pThunk->u1.Function != 0; pThunk++)
		{
			if ((pThunk->u1.Function & IMAGE_ORDINAL_FLAG) != 0)
			{
				//Non api name import.
				continue;
			}
			dwFOA = ConvertImageRVAToFOA(pSectionBeginPtr, dwImageSecNumber, (DWORD)pThunk->u1.AddressOfData, dwImageAlignSize);
			pImportApiName = (PIMAGE_IMPORT_BY_NAME)(uMemoryPtr + dwFOA);
			if (0 == lstrcmpiA(pImportApiName->Name, pszImportApiFunctionName))
			{
				if (g_IatHookInfoValidCount >= MAX_IATHOOK_INFO_ARRAY_SIZE)
				{
					goto EXIT;
				}
				StringCbCopyA(g_IatHookInfoBuff[g_IatHookInfoValidCount].szModuleName, MAX_MODULE_NAME_LENGTH, pszModName);
				StringCbCopyA(g_IatHookInfoBuff[g_IatHookInfoValidCount].szModuleApiName, MAX_MODULE_API_NAME_LENGTH, pImportApiName->Name);
				g_IatHookInfoBuff[g_IatHookInfoValidCount].pFnTransferNewAddress = (ULONG_PTR)lpTransferProc;
				AddDbgPrintStream(DBGFMTMSG(L"Found module name:%S api name:%S."), CURTID, USERMODE, FUNCNAME(L"FindImportApiByWalkImageImportTable"),
					DBG_TIPS, pszModName, pImportApiName->Name);
				g_IatHookInfoValidCount++;
			}
		}
	}
EXIT:
	return;
}
DWORD WINAPI ConvertImageRVAToFOA(PIMAGE_SECTION_HEADER lpSectionBeginPtr, DWORD	dwImageSectionNum, DWORD dwRVA, DWORD	dwImageAlignSize)
{
	DWORD					dwIndex = 0;
	DWORD					dwBlockAlignSize;
	DWORD					dwFOAResult = 0;
	PIMAGE_SECTION_HEADER	pSectionTempPtr = lpSectionBeginPtr;

	if (NULL == pSectionTempPtr)
	{
		return 0;
	}
	//[0,dwImageSectionNum)
	for (dwIndex = 0; dwIndex < dwImageSectionNum; dwIndex++)
	{
		//Hex
		dwBlockAlignSize = (pSectionTempPtr->Misc.VirtualSize + (dwImageAlignSize - 1)) & (~(dwImageAlignSize - 1));
		//[pSectionTempPtr->VirtualAddress,pSectionTempPtr->VirtualAddress + dwBlockAlignSize)
		if (dwRVA >= pSectionTempPtr->VirtualAddress && dwRVA < pSectionTempPtr->VirtualAddress + dwBlockAlignSize)
		{
			dwFOAResult = pSectionTempPtr->PointerToRawData + (dwRVA - pSectionTempPtr->VirtualAddress);
			break;
		}
		pSectionTempPtr = pSectionTempPtr + 1;
	}
	return dwFOAResult;
}

BOOL WINAPI InitializeIatHookInfoBuff()
{
	HMODULE	hMod = NULL;
	BOOL	bResult = FALSE;
	INT		iIndex = 0;
	DWORD	dwReturnCnt = 0;
	WCHAR	szCurAppFullPathName[MAX_PATH];
	szCurAppFullPathName[0] = L'\0';

	dwReturnCnt = GetModuleFileName(NULL, szCurAppFullPathName, sizeof(szCurAppFullPathName) / sizeof(WCHAR));
	if (0 == dwReturnCnt)
	{
		AddDbgPrintStream(DBGFMTMSG(L"Get current process executable file full path failed!"), CURTID, USERMODE, FUNCNAME(L"InitializeIatHookInfoBuff"), DBG_ERROR );
		return FALSE;
	}
	LoadImageDiskFileForInitIATHookBuff(szCurAppFullPathName);

	AddDbgPrintStream(DBGFMTMSG(L"Initialize IAT hook count:%d !"), CURTID, USERMODE, FUNCNAME(L"InitializeIatHookInfoBuff"), DBG_TIPS,
		g_IatHookInfoValidCount);

	if (g_IatHookInfoValidCount <= 0)
	{
		return FALSE;
	}
	for (iIndex = 0; iIndex < g_IatHookInfoValidCount; iIndex++)
	{
		hMod = GetModuleHandleA(g_IatHookInfoBuff[iIndex].szModuleName);
		if (NULL == hMod)
		{
			continue;
		}
		g_IatHookInfoBuff[iIndex].pFnCurrentAddress = (ULONG_PTR)GetProcAddress(hMod, g_IatHookInfoBuff[iIndex].szModuleApiName);
	}
	if (iIndex >= g_IatHookInfoValidCount)
	{
		bResult = TRUE;
	}
	return bResult;
}

BOOL WINAPI FixIatHookWorkForInside(HMODULE	hFixedModule)
{
	BOOL		bResult = FALSE;
	INT			nTotalSize = 0;
	INT			iIndex = 0;
	if ((ULONG_PTR)hFixedModule <= 0x10000)
	{
		return FALSE;
	}
	if (g_IatHookInfoValidCount <= 0)
	{
		return FALSE;
	}
	nTotalSize = g_IatHookInfoValidCount;
	for (iIndex = 0; iIndex < nTotalSize; iIndex++)
	{
		bResult = ReplaceIATEntryBySpecifyModule(g_IatHookInfoBuff[iIndex].szModuleName,
			g_IatHookInfoBuff[iIndex].pFnCurrentAddress, 
			g_IatHookInfoBuff[iIndex].pFnTransferNewAddress, 
			(ULONG_PTR)hFixedModule);
		g_IatHookInfoBuff[iIndex].bIsHookedOK = bResult;
		if (FALSE == bResult)
		{
			AddDbgPrintStream(DBGFMTMSG(L"Fix iat item[%S-%S] failed,skipped!"), CURTID, USERMODE, FUNCNAME(L"FixIatHookWorkForInside"), DBG_WARNING,
				g_IatHookInfoBuff[iIndex].szModuleName, g_IatHookInfoBuff[iIndex].szModuleApiName);
		}
		else
		{
			AddDbgPrintStream(DBGFMTMSG(L"Fix iat item[%S-%S] successfully!"), CURTID, USERMODE, FUNCNAME(L"FixIatHookWorkForInside"), DBG_TIPS,
				g_IatHookInfoBuff[iIndex].szModuleName, g_IatHookInfoBuff[iIndex].szModuleApiName);
		}
	}
	return TRUE;

}
VOID WINAPI RecoveryIatHookWorkForNormal(HMODULE	hFixedModule)
{
	BOOL		bResult = FALSE;
	INT			nTotalSize = 0;
	INT			iIndex = 0;
	if ((ULONG_PTR)hFixedModule <= 0x10000)
	{
		return ;
	}
	if (g_IatHookInfoValidCount <= 0)
	{
		return;
	}
	nTotalSize = g_IatHookInfoValidCount;
	for (iIndex = 0; iIndex < nTotalSize; iIndex++)
	{
		if (FALSE == g_IatHookInfoBuff[iIndex].bIsHookedOK)
		{
			continue;
		}
		//There is a extreme exception here.
		//bResult == FALSE ,how to do ?	let current application crash ?
		bResult = ReplaceIATEntryBySpecifyModule(g_IatHookInfoBuff[iIndex].szModuleName,
			g_IatHookInfoBuff[iIndex].pFnTransferNewAddress,
			g_IatHookInfoBuff[iIndex].pFnCurrentAddress,
			(ULONG_PTR)hFixedModule);
		if (TRUE == bResult)
		{
			g_IatHookInfoBuff[iIndex].bIsHookedOK = FALSE;
			AddDbgPrintStream(DBGFMTMSG(L"Recovery iat item[%S-%S] successfully!"),
				CURTID,
				USERMODE,
				FUNCNAME(L"RecoveryIatHookWorkForNormal"),
				DBG_TIPS,
				g_IatHookInfoBuff[iIndex].szModuleName,
				g_IatHookInfoBuff[iIndex].szModuleApiName);
		}
		else
		{
			AddDbgPrintStream(DBGFMTMSG(L"Recovery iat item[%S-%S] failed,skipped!"), 
				CURTID,
				USERMODE,
				FUNCNAME(L"RecoveryIatHookWorkForNormal"),
				DBG_ERROR,
				g_IatHookInfoBuff[iIndex].szModuleName,
				g_IatHookInfoBuff[iIndex].szModuleApiName);
		}
	}
	return;
}

