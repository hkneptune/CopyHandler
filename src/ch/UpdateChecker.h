#ifndef __UPDATECHECKER_H__
#define __UPDATECHECKER_H__

class CUpdateChecker
{
public:
	enum ECheckResult
	{
		eResult_Undefined,
		eResult_VersionOlder,
		eResult_VersionCurrent,
		eResult_VersionNewer,
		eResult_Error
	};
public:
	CUpdateChecker() : m_eResult(eResult_Undefined) { };
	~CUpdateChecker() { };

	ECheckResult CheckForUpdates(const tchar_t* pszSite, bool bCheckBeta);

	const tchar_t* GetNumericVersion() const { return (const tchar_t*)m_strNumericVersion; }
	const tchar_t* GetReadableVersion() const { return (const tchar_t*)m_strReadableVersion; }
	const tchar_t* GetLastError() const { return (const tchar_t*)m_strLastError; }
	const tchar_t* GetDownloadAddress() const { return m_strDownloadAddress; }

	ECheckResult GetResult() const { return m_eResult; }

protected:
	CString m_strSite;
	CString m_strLastError;
	CString m_strNumericVersion;
	CString m_strReadableVersion;
	CString m_strDownloadAddress;
	ECheckResult m_eResult;
};

#endif
