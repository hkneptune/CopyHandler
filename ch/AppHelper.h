#ifndef __APPHELPER_H__
#define __APPHELPER_H__

class CAppHelper
{
public:
	CAppHelper();
	~CAppHelper();

	void SetAutorun(bool bState);		// changes state of "run with system" option
	PTSTR ExpandPath(PTSTR pszString);	// expands path string - ie. <windows> into c:\windows

	bool IsFirstInstance() const { return m_bFirstInstance; };

	PCTSTR GetAppName() const { return m_pszAppName; };
	PCTSTR GetAppNameVer() const { return m_pszAppNameVer; };
	PCTSTR GetAppVersion() const { return m_pszAppVersion; };

	PCTSTR GetProgramPath() const { return m_pszProgramPath; };
	PCTSTR GetProgramName() const { return m_pszProgramName; };

protected:
	void InitProtection();		// optional call - protects from running multiple instance
	void RetrievePaths();							// reads program's path and name
	void RetrieveAppInfo();							// reads app name and version from VERSION resource
	UINT GetFolderLocation(int iFolder, PTSTR pszBuffer);

protected:
	HANDLE m_hMutex;
	bool m_bFirstInstance;		// tells if it is first instance(true) or second(or third, ...)
	TCHAR *m_pszMutexName;		// name of the protection mutex

	// program placement
	TCHAR* m_pszProgramPath;	// path from which this program was run
	TCHAR* m_pszProgramName;	// name of this program (ie. CH.exe)

	TCHAR* m_pszAppName;		// app-name string of this app
	TCHAR* m_pszAppNameVer;		// extended app-name-with small version
	TCHAR* m_pszAppVersion;		// app-version string of this app (VERSION based)
};

#endif