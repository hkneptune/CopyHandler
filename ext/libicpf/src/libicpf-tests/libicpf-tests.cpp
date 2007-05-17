// libicpf-tests.cpp : Defines the entry point for the console application.
//
#if defined(_WIN32) || defined(_WIN64)
	#include <windows.h>
#endif

#include <conio.h>
#include "gen_types.h"
#include "config-test.h"
#include "exception.h"

#if defined(_WIN32) || defined(_WIN64)
int_t _tmain(int_t argc, tchar_t* argv[])
#else
int main(int argc, char_t* argv[])
#endif
{
	ConfigTest ct;

	try
	{
		ct.Run();
	}
	catch(icpf::exception& e)
	{
		tchar_t szData[4096];
		e.get_info(szData, 4096);
		_tprintf(TSTRFMT, szData);
	}

	_getch();
	return 0;
}
