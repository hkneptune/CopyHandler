#ifndef __VERSION_H__
#define __VERSION_H__

// note that this file is also being used by setup compiler;
// in this case the SETUP_COMPILER is defined with value 1
// Product name
#define PRODUCT_NAME "Copy Handler"

// Version of program
#define PRODUCT_VERSION1 1
#define PRODUCT_VERSION2 30
#define PRODUCT_VERSION3 105
#define PRODUCT_VERSION4 0

#define PRODUCT_VERSION "1.30beta-svn105"

// Full product information (concatenation of the three above informations)
#if SETUP_COMPILER != 1
	#define PRODUCT_FULL_VERSION PRODUCT_NAME " " PRODUCT_VERSION
	#define PRODUCT_FULL_VERSION_T _T(PRODUCT_NAME) _T(" ") _T(PRODUCT_VERSION)
#endif

// copyright information
#define COPYRIGHT_INFO	"Copyright (C) 2001-2008 J�zef Starosczyk"

// shell extension
#define SHELLEXT_PRODUCT_NAME "Copy Handler Shell Extension"

#if SETUP_COMPILER != 1
	#define SHELLEXT_PRODUCT_FULL_VERSION SHELLEXT_PRODUCT_NAME " " PRODUCT_VERSION
	#define SHELLEXT_PRODUCT_FULL_VERSION_T _T(SHELLEXT_PRODUCT_NAME) _T(" ") _T(PRODUCT_VERSION)
#endif

#endif
