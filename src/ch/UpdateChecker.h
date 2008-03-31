#ifndef __UPDATECHECKER_H__
#define __UPDATECHECKER_H__

class CUpdateChecker
{
public:
	enum ECheckResult
	{
		eResult_VersionOlder,
		eResult_VersionCurrent,
		eResult_VersionNewer,
		eResult_Error
	};
public:
	CUpdateChecker() { };
	~CUpdateChecker() { };

	ECheckResult CheckForUpdates(bool bCheckBeta);
	void GetRemoteVersion(CString& rVersion) { rVersion = m_strRemoteVersion; }
	const tchar_t* GetLastError() const { return (const tchar_t*)m_strLastError; }

protected:
	CString m_strLastError;
	CString m_strRemoteVersion;
};

#endif
