/************************************************************************/
/*
Yufan
NOOB

CInjectDLL a class for DLL injection
to inject use Inject()
to uninject use Uninject()

I don't know why it can't wrok in relaese mode, so sorry
*/
/************************************************************************/
#pragma once

class CInjectDLL
{
public:
	CInjectDLL(void);
	BOOL Inject(const DWORD dwRemoteProcessID, const LPCTSTR& lpwszRemoteDllFullPath);
	BOOL Inject(const wchar_t* wszProcessName, const LPCTSTR& lpwszRemoteDllFullPath);
	BOOL Uninject(const DWORD dwRemoteProcessID, const LPCTSTR& lpwszRemoteDllFullPath);
	BOOL Uninject(const wchar_t* wszProcessName, const LPCTSTR& lpwszRemoteDllFullPath);
	~CInjectDLL(void);
private:
	BOOL AdjustProcessTokenPrivilege();
	BOOL GetProcessID(const wchar_t* wszProcessName, DWORD& dwProcID);
};

