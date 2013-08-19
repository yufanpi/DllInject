// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "stdafx.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	wchar_t wszOutput[20];
	DWORD dwPID = ::GetCurrentProcessId();
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		swprintf_s(wszOutput,_T("PID:%d Injected!!"),dwPID);
		::MessageBox(NULL,wszOutput,NULL,MB_OK);
		break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		::MessageBox(NULL,_T("Uninjected!!"),NULL,MB_OK);
		break;
	}
	
	return TRUE;
}

