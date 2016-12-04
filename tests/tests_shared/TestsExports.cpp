#ifdef TESTING

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <tchar.h>
#include <boost/format.hpp>
#include <boost/algorithm/string/replace.hpp>

class TFailedOutputPrinter : public ::testing::EmptyTestEventListener
{
	virtual void OnTestStart(const ::testing::TestInfo& test_info)
	{
		m_strTestName = str(boost::format("%1%.%2%") % test_info.test_case_name() % test_info.name());
	}

	virtual void OnTestPartResult(const ::testing::TestPartResult& test_part_result)
	{
		if(test_part_result.failed())
		{
			char* pszFailureText = NULL;
			if(test_part_result.fatally_failed())
				pszFailureText = "FATAL";
			else
				pszFailureText = "NON-FATAL";

			std::string strMsg = test_part_result.message() ? test_part_result.message() : "";
			boost::replace_all(strMsg, "\n", "\n    ");

			printf("**[%s FAILURE]** Unit test %s failed\n    %s(%ld) : %s\n",
				pszFailureText,
				m_strTestName.c_str(),
				test_part_result.file_name() ? test_part_result.file_name() : "Unknown location",
				test_part_result.line_number() < 0 ? 0 : test_part_result.line_number(),
				strMsg.c_str());
		}
	}

	virtual void OnTestProgramEnd(const ::testing::UnitTest& unit_test)
	{
		if(unit_test.Failed())
		{
			printf("Unit tests execution FAILED (successful: %ld, failed: %ld, elapsed time: %I64d ms)\n>\n",
				unit_test.successful_test_count(),
				unit_test.failed_test_count(),
				unit_test.elapsed_time());
		}
		else
		{
			printf("Unit tests execution succeeded (successful: %ld, failed: %ld, elapsed time: %I64d ms)\n>\n",
				unit_test.successful_test_count(),
				unit_test.failed_test_count(),
				unit_test.elapsed_time());
		}
	}

private:
	std::string m_strTestName;
};

#ifdef _CONSOLE
int _tmain(int argc, _TCHAR* argv[])
#else
extern "C"
__declspec(dllexport) int __stdcall RunTests(int argc, TCHAR* argv[])
#endif
{
	testing::InitGoogleMock(&argc, argv);
	::testing::FLAGS_gtest_death_test_style = "fast";
	::testing::FLAGS_gtest_print_time = 1;

	bool bUseStdFormat = false;
	for(int iIndex = 1; iIndex < argc; ++iIndex)
	{
		if(_tcscmp(argv[ iIndex ], _T("--stdformat")) == 0)
		{
			bUseStdFormat = true;
			break;
		}
	}

	if(!bUseStdFormat)
	{
		::testing::TestEventListeners& listeners = ::testing::UnitTest::GetInstance()->listeners();
		delete listeners.Release(listeners.default_result_printer());
		listeners.Append(new TFailedOutputPrinter);
	}

	return RUN_ALL_TESTS();
}

#endif
