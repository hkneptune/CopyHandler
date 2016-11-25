#ifndef __VERSION_H__
#define __VERSION_H__

// note that this file is also being used by setup compiler;
// in this case the SETUP_COMPILER is defined with value 1
// Product name
#define PRODUCT_NAME "Copy Handler"
// shell extension
#define SHELLEXT_PRODUCT_NAME "Copy Handler Shell Extension"

// Version of program
#define PRODUCT_VERSION1 1
#define PRODUCT_VERSION2 45
#define PRODUCT_VERSION3 0
#define PRODUCT_VERSION4 0

#if SETUP_COMPILER != 1
	#define PRODUCT_FULL_NUMERIC_VERSION ((unsigned long long)PRODUCT_VERSION1) << 48 | ((unsigned long long)PRODUCT_VERSION2) << 32 | ((unsigned long long)PRODUCT_VERSION3) << 16 | ((unsigned long long)PRODUCT_VERSION4)

	#define STRINGIZE2(a) STRINGIZE(a)
	#define STRINGIZE(a) L#a

	#define PRODUCT_NUMERIC_VERSION STRINGIZE2(PRODUCT_VERSION1) L"." STRINGIZE2(PRODUCT_VERSION2) L"." STRINGIZE2(PRODUCT_VERSION3) L"." STRINGIZE2(PRODUCT_VERSION4)
#endif

#define PRODUCT_VERSION "1.45internal-current"

#if SETUP_COMPILER != 1
	#define SHELLEXT_PRODUCT_FULL_VERSION SHELLEXT_PRODUCT_NAME " " PRODUCT_VERSION
	#define SHELLEXT_PRODUCT_FULL_VERSION_T _T(SHELLEXT_PRODUCT_NAME) _T(" ") _T(PRODUCT_VERSION)

	#define PRODUCT_FULL_VERSION PRODUCT_NAME " " PRODUCT_VERSION
	#define PRODUCT_FULL_VERSION_T _T(PRODUCT_NAME) _T(" ") _T(PRODUCT_VERSION)
#endif

// copyright information
#define COPYRIGHT_INFO	"Copyright (C) 2001-2016 Józef Starosczyk"
#define PRODUCT_SITE		"http://www.copyhandler.com"
#define UPDATE_CHECK_LINK_SECURE	"https://www.copyhandler.com/update-check"
#define UPDATE_CHECK_LINK_NONSECURE	"http://www.copyhandler.com/update-check"
#define CONTACT_INFO		"http://www.copyhandler.com/contact"

#endif
