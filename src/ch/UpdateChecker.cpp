#include "stdafx.h"
#include "UpdateChecker.h"
#include <afxinet.h>
#include <atlconv.h>
#include "../common/version.h"
#include "../libicpf/cfg.h"
#include "../libicpf/exception.h"

CUpdateChecker::ECheckResult CUpdateChecker::CheckForUpdates(const tchar_t* pszSite, bool bCheckBeta)
{
	try
	{
		CString strSite(pszSite);
		strSite.Replace(_T("http://"), _T(""));

		CInternetSession inetSession;
		CHttpConnection* pHttpConnection = inetSession.GetHttpConnection(strSite, INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE, 80);
		if(!pHttpConnection)
		{
			m_eResult = eResult_Error;
			return m_eResult;
		}

		CString strAddr = _T("chver.ini");
		CHttpFile* pHttpFile = pHttpConnection->OpenRequest(CHttpConnection::HTTP_VERB_GET, strAddr);
		if(!pHttpFile)
		{
			m_eResult = eResult_Error;
			return m_eResult;
		}

		if(!pHttpFile->SendRequest())
		{
			m_eResult = eResult_Error;
			return m_eResult;
		}

		const size_t stBufferSize(65536);
		tchar_t* pszBuf = new tchar_t[stBufferSize];
		UINT uiRD = pHttpFile->Read(pszBuf, stBufferSize - 1);
		if(uiRD > 0)
			pszBuf[uiRD] = _T('\0');

		// convert text to unicode
		icpf::config cfg(icpf::config::eIni);
		const uint_t uiVersionNumeric = cfg.register_string(_t("Version/Numeric"), _t(""));
		const uint_t uiVersionReadable = cfg.register_string(_t("Version/Human Readable"), _t(""));
		const uint_t uiDownloadAddress = cfg.register_string(_t("Version/Download Address"), pszSite);
		const uint_t uiBetaVersionNumeric = cfg.register_string(_t("Version/Numeric Beta"), _t(""));
		const uint_t uiBetaVersionReadable = cfg.register_string(_t("Version/Human Readable Beta"), _t(""));
		const uint_t uiBetaDownloadAddress = cfg.register_string(_t("Version/Download Address Beta"), pszSite);
		try
		{
			cfg.read_from_buffer(pszBuf, uiRD);
		}
		catch(icpf::exception& e)
		{
			m_strLastError = e.get_desc();

			delete [] pszBuf;
			m_eResult = eResult_Error;
			return m_eResult;
		}
		delete [] pszBuf;

		if(bCheckBeta)
		{
			m_strReadableVersion = cfg.get_string(uiBetaVersionReadable);
			m_strNumericVersion = cfg.get_string(uiBetaVersionNumeric);
			m_strDownloadAddress = cfg.get_string(uiBetaDownloadAddress);
		}

		if(!bCheckBeta || m_strNumericVersion.IsEmpty())
		{
			m_strNumericVersion = cfg.get_string(uiVersionNumeric);
			m_strReadableVersion = cfg.get_string(uiVersionReadable);
			m_strDownloadAddress = cfg.get_string(uiDownloadAddress);
		}

		// and compare to current version
		ushort_t usVer[4];
		if(_stscanf(m_strNumericVersion, _t("%hu.%hu.%hu.%hu"), &usVer[0], &usVer[1], &usVer[2], &usVer[3]) != 4)
		{
			TRACE(_T("Error parsing retrieved version number."));
			m_eResult = eResult_Error;
			return m_eResult;
		}

		ull_t ullCurrentVersion = ((ull_t)PRODUCT_VERSION1) << 48 | ((ull_t)PRODUCT_VERSION2) << 32 | ((ull_t)PRODUCT_VERSION3) << 16 | ((ull_t)PRODUCT_VERSION4);
		ull_t ullSiteVersion = ((ull_t)usVer[0]) << 48 | ((ull_t)usVer[1]) << 32 | ((ull_t)usVer[2]) << 16 | ((ull_t)usVer[3]);

		if(ullCurrentVersion < ullSiteVersion)
			m_eResult = eResult_VersionNewer;
		else if(ullCurrentVersion == ullSiteVersion)
			m_eResult = eResult_VersionCurrent;
		else
			m_eResult = eResult_VersionOlder;

		return m_eResult;
	}
	catch(CInternetException* e)
	{
		TCHAR* pszBuffer = m_strLastError.GetBufferSetLength(1024);
		e->GetErrorMessage(pszBuffer, 1023);
		pszBuffer[1023] = _T('\0');
		m_strLastError.ReleaseBuffer();
		m_strLastError.TrimRight();

		m_eResult = eResult_Error;
		return m_eResult;
	}
}
