#include "test-base.h"
#include <stdio.h>
#include <stdarg.h>

#define MAX_BUF	4096

TestBase::TestBase()
{
}

TestBase::~TestBase()
{
}

void TestBase::Run()
{
	assert(false);
}

void TestBase::Report(const tchar_t* pszFmt, ...)
{
	va_list va;
	va_start(va, pszFmt);

	tchar_t szBuf[MAX_BUF];
	_vsntprintf(szBuf, MAX_BUF, pszFmt, va);		// user passed stuff
	ReportS(szBuf);

	va_end(va);
}

void TestBase::ReportS(const tchar_t* pszStr)
{
	_tprintf(TSTRFMT, pszStr);
}
