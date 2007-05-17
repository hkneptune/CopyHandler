#ifndef __TESTBASE_H__
#define __TESTBASE_H__

#include <assert.h>
#include "gen_types.h"

class TestBase
{
public:
	TestBase();
	virtual ~TestBase();

	virtual void Run();

	void Report(const tchar_t* pszFmt, ...);
	void ReportS(const tchar_t* pszFmt);
};

#endif
