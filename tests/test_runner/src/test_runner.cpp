// libchcore_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	using std::wcout;
	using std::endl;

	if(argc < 2)
	{
		wcout << _T("Usage: TestRunner.exe <executable_name>") << endl;
		return 1;
	}

	PCTSTR pszLibName = argv[1];

	wcout << _T("Executing tests in: ") << pszLibName << endl;

	HMODULE module = LoadLibrary(pszLibName);
	if(module)
	{
		typedef int(__stdcall *pfnRunTests)(int argc, TCHAR* argv[]);

		pfnRunTests pfnFunc = (pfnRunTests)::GetProcAddress(module, "_RunTests@8");
		if(!pfnFunc)
			pfnFunc = (pfnRunTests)::GetProcAddress(module, "RunTests");
		if(pfnFunc)
			return pfnFunc(argc, argv);
		else
		{
			DWORD dwErr = GetLastError();
			wcout << _T("Library ") << pszLibName << _T(" does not have tests embedded. Error: ") << dwErr << endl;
		}
	}
	else
		wcout << _T("Cannot load library ") << pszLibName << endl;
}
