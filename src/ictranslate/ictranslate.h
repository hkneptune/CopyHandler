// ictranslate.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CICTranslateApp:
// See ictranslate.cpp for the implementation of this class
//

class CICTranslateApp : public CWinApp
{
public:
	CICTranslateApp();

// Overrides
	public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CICTranslateApp theApp;