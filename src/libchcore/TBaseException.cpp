#include "stdafx.h"
#include "TBaseException.h"
#include <atltrace.h>

BEGIN_CHCORE_NAMESPACE

TBaseException::TBaseException(EGeneralErrors eErrorCode, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
	m_eErrorCode(eErrorCode),
	m_pszMsg(pszMsg),
	m_bDeleteMsg(false),
	m_pszFile(pszFile),
	m_stLineNumber(stLineNumber),
	m_pszFunction(pszFunction)
{
	ATLTRACE(_T("*** Base Exception is being thrown:\n\tMsg: %s\n\tError code: %ld\n\tFile: %s\n\tLine number: %ld\n\tFunction: %s\n"), pszMsg, eErrorCode, pszFile, stLineNumber, pszFunction);
}

TBaseException::TBaseException(EGeneralErrors eErrorCode, const char* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
	m_eErrorCode(eErrorCode),
	m_pszMsg(NULL),
	m_bDeleteMsg(false),
	m_pszFile(pszFile),
	m_stLineNumber(stLineNumber),
	m_pszFunction(pszFunction)
{
	ATLTRACE(_T("*** Base Exception is being thrown:\n\tMsg: %S\n\tError code: %ld\n\tFile: %s\n\tLine number: %ld\n\tFunction: %s\n"), pszMsg, eErrorCode, pszFile, stLineNumber, pszFunction);
	if(pszMsg)
	{
		size_t stMsgLen = strlen(pszMsg);
		m_pszMsg = new wchar_t[stMsgLen + 1];
		
		size_t stResult = 0;
		mbstowcs_s(&stResult, const_cast<wchar_t*>(m_pszMsg), stMsgLen + 1, pszMsg, _TRUNCATE);
	}
}

TBaseException::~TBaseException()
{
	if(m_bDeleteMsg)
		delete [] m_pszMsg;
}

void TBaseException::GetErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
{
	_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("%s (error code: %ld)"),
		m_pszMsg, m_eErrorCode);
	pszBuffer[stMaxBuffer - 1] = _T('\0');
}

void TBaseException::GetDetailedErrorInfo(wchar_t* pszBuffer, size_t stMaxBuffer) const
{
	_snwprintf_s(pszBuffer, stMaxBuffer, _TRUNCATE, _T("%s\r\nError code: %ld\r\nFile: %s\r\nFunction: %s\r\nLine no: %lu"),
		m_pszMsg, m_eErrorCode, m_pszFile, m_pszFunction, (unsigned long)m_stLineNumber);
	pszBuffer[stMaxBuffer - 1] = _T('\0');
}

END_CHCORE_NAMESPACE
