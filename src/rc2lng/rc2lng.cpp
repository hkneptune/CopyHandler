// rc2lng.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rc2lng.h"
#include "conio.h"
#include "rc.h"
#include "../libicpf/exception.h"

#pragma warning(disable : 4786)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int _tmain(int argc, TCHAR* argv[], TCHAR* /*envp*/[])
{
	// initialize MFC and print and error on failure
	HMODULE hModule = ::GetModuleHandle(NULL);
	if(!hModule)
		return -1;
	if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
	{
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return 1;
	}

	// usage - rc2lng infile.rc resource.h inheader.lng outfile.rc outfile.lng
	if (argc < 6)
	{
		wcerr << _T("Fatal Error: Incorrect numer of params") << endl;
		wcerr << _T("Usage: infile.rc inheader.lng outfile.rc outfile.lng resource.h resource2.h") << endl;
		return -1;
	}

	CRCFile rcFile;

	try
	{
		for (int i=5;i<argc;i++)
		{
			rcFile.ReadResourceIDs(argv[i]);
		}
		
		rcFile.ReadRC(argv[1]);

		rcFile.WriteRC(argv[3]);
		rcFile.WriteLang(argv[4], argv[2]);
	}
	catch(icpf::exception& e)
	{
		wcerr << e.get_desc() << endl;
		return -1;
	}
	return 0;
}
