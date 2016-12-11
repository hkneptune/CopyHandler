#include "stdafx.h"
#include "TStringException.h"

namespace string
{
	TStringException::TStringException(const char* pszMsg) :
		std::exception(pszMsg)
	{
	}
}
