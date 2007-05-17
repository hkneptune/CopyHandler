#ifndef __CONFIGTEST_H__
#define __CONFIGTEST_H__

#include "test-base.h"

class ConfigTest : public TestBase
{
public:
	ConfigTest() : TestBase() { };

	virtual void Run();
};

#endif
