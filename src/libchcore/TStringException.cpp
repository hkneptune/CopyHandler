#include "stdafx.h"
#include "TStringException.h"

namespace chcore
{
	TStringException::TStringException(EGeneralErrors eErrorCode, const wchar_t* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
		TBaseException(eErrorCode, pszMsg, pszFile, stLineNumber, pszFunction)
	{
	}

	TStringException::TStringException(EGeneralErrors eErrorCode, const char* pszMsg, const wchar_t* pszFile, size_t stLineNumber, const wchar_t* pszFunction) :
		TBaseException(eErrorCode, pszMsg, pszFile, stLineNumber, pszFunction)
	{
	}
}
