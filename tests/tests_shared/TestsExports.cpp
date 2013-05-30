#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <tchar.h>

#ifdef TESTING

#ifdef _CONSOLE
int _tmain(int argc, _TCHAR* argv[])
{
	testing::InitGoogleMock(&argc, argv);
	::testing::FLAGS_gtest_death_test_style = "fast";
	::testing::FLAGS_gtest_print_time = 1;
	return RUN_ALL_TESTS();
}
#else
extern "C" 
__declspec(dllexport) int __stdcall RunTests(int argc, TCHAR* argv[])
{
	testing::InitGoogleMock(&argc, argv);
	::testing::FLAGS_gtest_death_test_style = "fast";
	::testing::FLAGS_gtest_print_time = 1;
	return RUN_ALL_TESTS();
}
#endif
#endif
