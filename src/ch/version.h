#ifndef __VERSION_H__
#define __VERSION_H__

#ifndef _T
	#define _T(text) text
#endif

// Product name
#define PRODUCT_NAME _T("Copy Handler")
// Version of program
#define PRODUCT_VERSION1 1
#define PRODUCT_VERSION2 30
#define PRODUCT_VERSION3 58
#define PRODUCT_VERSION4 0

#define PRODUCT_VERSION _T("1.30beta-svn58")

// Full product information (concatenation of the three above informations)
#define PRODUCT_FULL_VERSION PRODUCT_NAME _T(" ") PRODUCT_VERSION

#define COPYRIGHT_INFO	_T("Copyright (C) 2001-2008 Józef Starosczyk")
#endif
