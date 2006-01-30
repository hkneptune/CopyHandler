#include "stdafx.h"
#include "AppHelper.h"
#include "shlobj.h"

CAppHelper::CAppHelper()
{
	// read program paths
	RetrievePaths();

	// retrieve VERSION-based info
	RetrieveAppInfo();

	// single-instance protection
	m_bFirstInstance=true;
	m_hMutex=NULL;
	
	// name of the protection mutex
	m_pszMutexName=new TCHAR[_tcslen(m_pszAppName)+sizeof(_T("__ instance"))/sizeof(TCHAR)+1];
	_stprintf(m_pszMutexName, _T("_%s_ instance"), m_pszAppName);
	_tcslwr(m_pszMutexName+2);	// first letter of appname has to be of predefined case
}

CAppHelper::~CAppHelper()
{
	if (m_hMutex)
		ReleaseMutex(m_hMutex);

	delete [] m_pszProgramPath;
	delete [] m_pszProgramName;
	delete [] m_pszAppName;
	delete [] m_pszAppNameVer;
	delete [] m_pszAppVersion;
	delete [] m_pszMutexName;
}

// inits mutex app protection
void CAppHelper::InitProtection()
{
	m_hMutex=CreateMutex(NULL, TRUE, m_pszMutexName);
	m_bFirstInstance=(m_hMutex != NULL && GetLastError() != ERROR_ALREADY_EXISTS);
}

// retrieves application path
void CAppHelper::RetrievePaths()
{
	// try to find '\\' in path to see if this is only exe name or fully qualified path
	TCHAR* pszName=_tcsrchr(__argv[0], _T('\\'));
	if (pszName != NULL)
	{
		// copy name
		m_pszProgramName=new TCHAR[_tcslen(pszName+1)+1];
		_tcscpy(m_pszProgramName, pszName+1);

		// path
		UINT uiSize=(UINT)(pszName-__argv[0]);
        m_pszProgramPath=new TCHAR[uiSize+1];
		_tcsncpy(m_pszProgramPath, __argv[0], uiSize);
		m_pszProgramPath[uiSize]=_T('\0');
	}
	else
	{
		// copy name
		m_pszProgramName=new TCHAR[_tcslen(__argv[0])+1];
		_tcscpy(m_pszProgramName, __argv[0]);

		// path
		TCHAR szPath[_MAX_PATH];
		UINT uiSize=GetCurrentDirectory(_MAX_PATH, szPath);
		_tcscat(szPath, _T("\\"));
		m_pszProgramPath=new TCHAR[uiSize+2];
		_tcsncpy(m_pszProgramPath, szPath, uiSize+2);
	}
}

void CAppHelper::RetrieveAppInfo()
{
	if (m_pszAppName)
	{
		delete [] m_pszAppName;
		m_pszAppName=NULL;
	}
	if (m_pszAppNameVer)
	{
		delete [] m_pszAppNameVer;
		m_pszAppNameVer=NULL;
	}
	if (m_pszAppVersion)
	{
		delete [] m_pszAppVersion;
		m_pszAppVersion=NULL;
	}

	TCHAR *pszPath=new TCHAR[_tcslen(m_pszProgramPath)+_tcslen(m_pszProgramName)+2];
	_tcscpy(pszPath, m_pszProgramPath);
	_tcscat(pszPath, _T("\\"));
	_tcscat(pszPath, m_pszProgramName);

	DWORD dwHandle;
	DWORD dwSize=GetFileVersionInfoSize(pszPath, &dwHandle);
	if (dwSize)
	{
		BYTE *pbyBuffer=new BYTE[dwSize];
		if (GetFileVersionInfo(pszPath, 0, dwSize, pbyBuffer))
		{
			TCHAR* pszProp;
			UINT uiLen;
			// product name with short version number
			if (VerQueryValue(pbyBuffer, _T("\\StringFileInfo\\040904b0\\ProductName"), (LPVOID*)&pszProp, &uiLen))
			{
				m_pszAppNameVer=new TCHAR[uiLen];
				_tcscpy(m_pszAppNameVer, pszProp);
			}

			// product long version
			if (VerQueryValue(pbyBuffer, _T("\\StringFileInfo\\040904b0\\ProductVersion"), (LPVOID*)&pszProp, &uiLen))
			{
				m_pszAppVersion=new TCHAR[uiLen];
				_tcscpy(m_pszAppVersion, pszProp);
			}

			// product name without version number
			if (VerQueryValue(pbyBuffer, _T("\\StringFileInfo\\040904b0\\InternalName"), (LPVOID*)&pszProp, &uiLen))
			{
				m_pszAppName=new TCHAR[uiLen];
				_tcscpy(m_pszAppName, pszProp);
			}
		}

		delete [] pbyBuffer;
	}
	delete [] pszPath;

	if (m_pszAppNameVer == NULL)
	{
		m_pszAppNameVer=new TCHAR[sizeof(_T("Unknown App 1.0"))/sizeof(TCHAR)];
		_tcscpy(m_pszAppNameVer, _T("Unknown App 1.0"));
	}
	if (m_pszAppVersion == NULL)
	{
		m_pszAppVersion=new TCHAR[sizeof(_T("1.0.0.0"))/sizeof(TCHAR)];
		_tcscpy(m_pszAppVersion, _T("1.0.0.0"));
	}
	if (m_pszAppName == NULL)
	{
		m_pszAppName=new TCHAR[sizeof(_T("Unknown App"))/sizeof(TCHAR)];
		_tcscpy(m_pszAppName, _T("Unknown App"));
	}
}

// internal func - safe getting special folder locations
UINT CAppHelper::GetFolderLocation(int iFolder, PTSTR pszBuffer)
{
	LPITEMIDLIST piid;
	HRESULT h=SHGetSpecialFolderLocation(NULL, iFolder, &piid);
	if (!SUCCEEDED(h))
		return false;

	// get path
	BOOL bRes=SHGetPathFromIDList(piid, pszBuffer);

	// free piid
	LPMALLOC lpm;
	if (!SUCCEEDED(SHGetMalloc(&lpm)))
		return 0;

	lpm->Free((void*)piid);
	lpm->Release();

	// check for error
	if (!bRes)
		return 0;

	// strip the last '\\'
	UINT uiLen=(UINT)_tcslen(pszBuffer);
	if (pszBuffer[uiLen-1] == _T('\\'))
	{
		pszBuffer[uiLen-1]=_T('\0');
		return uiLen-1;
	}
	else
		return uiLen;
}

// expands given path
PTSTR CAppHelper::ExpandPath(PTSTR pszString)
{
	// check if there is need to perform all these checkings
	if (pszString[0] != _T('<'))
		return pszString;

	TCHAR szStr[_MAX_PATH];
	szStr[0]=_T('\0');

	// search for string to replace
	// _T("<WINDOWS>"), _T("<TEMP>"), _T("<SYSTEM>"), _T("<APPDATA>"), _T("<DESKTOP>"), 
	// _T("<PERSONAL>"), _T("<PROGRAM>")
	if (_tcsnicmp(pszString, _T("<PROGRAM>"), 9) == 0)
	{
		// get windows path
		_tcscpy(szStr, m_pszProgramPath);
		_tcscat(szStr, pszString+9);
	}
	else if (_tcsnicmp(pszString, _T("<WINDOWS>"), 9) == 0)
	{
		// get windows path
		UINT uiSize=GetWindowsDirectory(szStr, _MAX_PATH);
		if (szStr[uiSize-1] == _T('\\'))
			szStr[uiSize-1]=_T('\0');
		_tcscat(szStr, pszString+9);
	}
	else if (_tcsnicmp(pszString, _T("<TEMP>"), 6) == 0)	// temp dir
	{
		// get windows path
		UINT uiSize=GetTempPath(_MAX_PATH, szStr);
		if (szStr[uiSize-1] == _T('\\'))
			szStr[uiSize-1]=_T('\0');
		_tcscat(szStr, pszString+6);
	}
	else if (_tcsnicmp(pszString, _T("<SYSTEM>"), 8) == 0)	// system
	{
		// get windows path
		UINT uiSize=GetSystemDirectory(szStr, _MAX_PATH);
		if (szStr[uiSize-1] == _T('\\'))
			szStr[uiSize-1]=_T('\0');
		_tcscat(szStr, pszString+8);
	}
	else if (_tcsnicmp(pszString, _T("<APPDATA>"), 9) == 0)	// app data
	{
		// get windows path
		UINT uiSize=GetFolderLocation(CSIDL_APPDATA, szStr);
		if (szStr[uiSize-1] == _T('\\'))
			szStr[uiSize-1]=_T('\0');
		_tcscat(szStr, pszString+9);
	}
	else if (_tcsnicmp(pszString, _T("<DESKTOP>"), 9) == 0)	// desktop
	{
		// get windows path
		UINT uiSize=GetFolderLocation(CSIDL_DESKTOPDIRECTORY, szStr);
		if (szStr[uiSize-1] == _T('\\'))
			szStr[uiSize-1]=_T('\0');
		_tcscat(szStr, pszString+9);
	}
	else if (_tcsnicmp(pszString, _T("<PERSONAL>"), 10) == 0)	// personal...
	{
		// get windows path
		UINT uiSize=GetFolderLocation(CSIDL_PERSONAL, szStr);
		if (szStr[uiSize-1] == _T('\\'))
			szStr[uiSize-1]=_T('\0');
		_tcscat(szStr, pszString+10);
	}

	// copy to src string
	if (szStr[0] != _T('\0'))
		_tcscpy(pszString, szStr);

	return pszString;
}

void CAppHelper::SetAutorun(bool bState)
{
	// storing key in registry
	HKEY hkeyRun;
	if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), 0, KEY_ALL_ACCESS, &hkeyRun) != ERROR_SUCCESS)
		return;
	
	if (bState)
	{
		TCHAR *pszPath=new TCHAR[_tcslen(m_pszProgramPath)+_tcslen(m_pszProgramName)+2];
		_tcscpy(pszPath, m_pszProgramPath);
		_tcscat(pszPath, _T("\\"));
		_tcscat(pszPath, m_pszProgramName);

		RegSetValueEx(hkeyRun, m_pszAppName, 0, REG_SZ, (BYTE*)pszPath, (DWORD)(_tcslen(pszPath)+1)*sizeof(TCHAR));

		delete [] pszPath;
	}
	else
		RegDeleteValue(hkeyRun, m_pszAppName);
	
	RegCloseKey(hkeyRun);
}
