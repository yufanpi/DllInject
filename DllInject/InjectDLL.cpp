#include "StdAfx.h"
#include "InjectDLL.h"
#include <Windows.h>
#include <string>
#include "tlhelp32.h"


CInjectDLL::CInjectDLL(void)
{
}


CInjectDLL::~CInjectDLL(void)
{
}

BOOL CInjectDLL::Inject(const DWORD dwRemoteProcessID, const LPCTSTR& lpwszRemoteDllFullPath)
{
	std::wstring wstrRemoteDllFullPath = lpwszRemoteDllFullPath;
	
	AdjustProcessTokenPrivilege();
	
	HANDLE hRemoteProgress = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwRemoteProcessID);
	if (hRemoteProgress == NULL)
	{
		//wprintf_s(_T("OpenProcess fail\n"));
		return FALSE;
	}
	DWORD dwMemSize = sizeof(wchar_t)*wstrRemoteDllFullPath.length()+1;
	wchar_t* wszDllPath = reinterpret_cast<wchar_t*>(::VirtualAllocEx(hRemoteProgress, NULL, dwMemSize, MEM_COMMIT, PAGE_READWRITE));
	if (wszDllPath == NULL)
	{
		//wprintf_s(_T("Allocate memory in remote process fail\n"));
		::CloseHandle(hRemoteProgress);
		return FALSE;
	}
	::WriteProcessMemory(hRemoteProgress, wszDllPath, wstrRemoteDllFullPath.c_str(), dwMemSize, NULL);
	FARPROC pfnFunAddr = ::GetProcAddress(::GetModuleHandle(_T("Kernel32")),"LoadLibraryW");
	::CreateRemoteThread(hRemoteProgress, NULL, 0, (LPTHREAD_START_ROUTINE) pfnFunAddr, wszDllPath, 0, NULL);

	::VirtualFreeEx(hRemoteProgress, reinterpret_cast<LPVOID>(wszDllPath), dwMemSize, MEM_COMMIT);
	::CloseHandle(hRemoteProgress);
	return TRUE;
}

BOOL CInjectDLL::Inject(const wchar_t* wszProcessName, const LPCTSTR& lpwszRemoteDllFullPath)
{
	DWORD dwProcID;
	GetProcessID(wszProcessName,dwProcID);
	if (Inject(dwProcID, lpwszRemoteDllFullPath))
		return TRUE;
	else
		return FALSE;
}

BOOL CInjectDLL::Uninject(const DWORD dwRemoteProcessID, const LPCTSTR& lpwszRemoteDllFullPath)
{
	std::wstring wstrRemoteDllFullPath = lpwszRemoteDllFullPath;

	HANDLE hSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwRemoteProcessID);
	MODULEENTRY32 Me32 = {0};
	Me32.dwSize = sizeof(MODULEENTRY32);

	BOOL bRet = ::Module32First(hSnap, &Me32);
	while (bRet)
	{
		if (wcscmp(Me32.szExePath, wstrRemoteDllFullPath.c_str()) == 0)
		{
			break;
		}
		bRet = Module32Next(hSnap, &Me32);
	}
	CloseHandle(hSnap);

	HANDLE hRemoteProgress = ::OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwRemoteProcessID);
	if (hRemoteProgress == NULL)
	{
		//wprintf_s(_T("OpenProcess fail\n"));
		return FALSE;
	}

	FARPROC pfnFunAddr = ::GetProcAddress(::GetModuleHandle(_T("Kernel32")),"FreeLibrary");
	::CreateRemoteThread(hRemoteProgress, NULL, 0, (LPTHREAD_START_ROUTINE) pfnFunAddr, Me32.hModule, 0, NULL);

	::CloseHandle(hRemoteProgress);
	return TRUE;
}

BOOL CInjectDLL::Uninject(const wchar_t* wszProcessName, const LPCTSTR& lpwszRemoteDllFullPath)
{
	DWORD dwProcID;
	GetProcessID(wszProcessName,dwProcID);
	if (Uninject(dwProcID, lpwszRemoteDllFullPath))
		return TRUE;
	else
		return FALSE;
}


BOOL CInjectDLL::AdjustProcessTokenPrivilege()
{
	LUID luidTmp;
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		OutputDebugString(_T("AdjustProcessTokenPrivilege OpenProcessToken Failed ! \n"));

		return FALSE;
	}

	if(!LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luidTmp))
	{
		OutputDebugString(_T("AdjustProcessTokenPrivilege LookupPrivilegeValue Failed ! \n"));

		CloseHandle(hToken);

		return FALSE;
	}

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luidTmp;
	tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

	if(!AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(tkp), NULL, NULL))
	{
		OutputDebugString(_T("AdjustProcessTokenPrivilege AdjustTokenPrivileges Failed ! \n"));

		CloseHandle(hToken);

		return FALSE;
	}
	return TRUE;
}

BOOL CInjectDLL::GetProcessID(const wchar_t* wszProcessName, DWORD& dwProcID)
{
	HANDLE hSnapShot	= ::CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, dwProcID );
	PROCESSENTRY32 pe	= {sizeof(pe)};
	BOOL bOk			= ::Process32First( hSnapShot, &pe );
	while( bOk )
	{
		//wprintf(TEXT("ProcessID : %d, Name : %s\n"), pe.th32ProcessID, pe.szExeFile);
		if (wcsstr(pe.szExeFile, wszProcessName)!=NULL)
		{
			dwProcID = pe.th32ProcessID;
			return TRUE;
		}

		bOk	= ::Process32Next( hSnapShot, &pe );
	}

	::CloseHandle(hSnapShot);

	return FALSE;
}

