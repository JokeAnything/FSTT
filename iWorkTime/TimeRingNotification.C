#include "TimeRingNotification.h"

VOID WINAPI TimeRingNotificationProc(DWORD	dwNotifyFlag)
{
	UINT uId;
	UINT uSoundRSRID[] = { IDR_WAVE4,IDR_WAVE1,IDR_WAVE2,IDR_WAVE3};
	uId = uSoundRSRID[dwNotifyFlag];
	PlaySound(NULL, GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	PlaySound(MAKEINTRESOURCE(uId), GetModuleHandle(NULL), SND_RESOURCE | SND_ASYNC);
	return;
}