#include "stdafx.h"
#include "UpdateChecker.h"
#include <afxinet.h>
#include <atlconv.h>
#include "../common/version.h"

CUpdateChecker::ECheckResult CUpdateChecker::CheckForUpdates(bool bCheckBeta)
{
	try
	{
		CInternetSession inetSession;
		CHttpConnection* pHttpConnection = inetSession.GetHttpConnection(_T("www.copyhandler.com"), INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, 80);
		if(!pHttpConnection)
			return eResult_Error;

		CString strAddr = _T("chver.php?ver=");
		strAddr += bCheckBeta ? _T("beta") : _T("stable");
		CHttpFile* pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strAddr);
		if(!pHttpFile)
			return eResult_Error;

		if(!pHttpFile->SendRequest())
			return eResult_Error;

		char szBuf[512];
		UINT uiRD = pHttpFile->Read(szBuf, 511);
		if(uiRD > 0)
			szBuf[uiRD] = _T('\0');

		// convert text to unicode
		CA2CT a2ct(szBuf);

		CString strVersion = a2ct;
		CString strNumericVersion;
		CString strReadableVersion;
		int iPos = strVersion.Find(_t('|'));
		if(iPos != -1)
		{
			strNumericVersion = strVersion.Mid(0, iPos);
			strReadableVersion = strVersion.Mid(iPos + 1);
		}
		else
			strNumericVersion = strVersion;

		m_strRemoteVersion = strReadableVersion;

		// and compare to current version
		ushort_t usVer[4];
		if(_stscanf(strNumericVersion, _t("%hu.%hu.%hu.%hu"), &usVer[0], &usVer[1], &usVer[2], &usVer[3]) != 4)
		{
			TRACE(_T("Error parsing retrieved version number."));
			return eResult_Error;
		}

		ull_t ullCurrentVersion = ((ull_t)PRODUCT_VERSION1) << 48 | ((ull_t)PRODUCT_VERSION2) << 32 | ((ull_t)PRODUCT_VERSION3) << 16 | ((ull_t)PRODUCT_VERSION4);
		ull_t ullSiteVersion = ((ull_t)usVer[0]) << 48 | ((ull_t)usVer[1]) << 32 | ((ull_t)usVer[2]) << 16 | ((ull_t)usVer[3]);

		if(ullCurrentVersion < ullSiteVersion)
			return eResult_VersionNewer;
		else if(ullCurrentVersion == ullSiteVersion)
			return eResult_VersionCurrent;
		else
			return eResult_VersionOlder;
	}
	catch(CInternetException* e)
	{
		TCHAR* pszBuffer = m_strLastError.GetBufferSetLength(1024);
		e->GetErrorMessage(pszBuffer, 1023);
		pszBuffer[1023] = _T('\0');
		m_strLastError.ReleaseBuffer();

		return eResult_Error;
	}
}
