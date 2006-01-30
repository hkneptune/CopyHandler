#ifndef __CHPLUGINCORE_H__
#define __CHPLUGINCORE_H__

#include "PluginCore.h"
//#include "FileObject.h"

// types of CH plugins
//#define PT_STORAGE		2

//#define IMID_LOCALSTORAGE		0x0001020000000001

// error codes
//#define EC_NOERROR		0x00000000
//#define EC_SYSERROR		0x00000001
//#define EC_USERBREAK	0x00000002

// error handling info struct
/*struct _ERRORINFO
{
	char szInfo[256];		// string with error description
	DWORD dwCode;		// code
	DWORD dwError;		// system error if any
};*/

// storage plugins section
/*struct _ENUMOBJECTS
{
	// data passed to the enumobjects func
	CBaseObjectInfo* pBaseObject;	// base object

	// ret values
	CObjectInfo* pObject;			// object that has been found

	// func
	bool(*pfnCallback)(_ENUMOBJECTS* pParam);

	// other
	PVOID pAppParam;				// application defined parameter
	PVOID pPlugParam;				// plugin dependent parameter
};*/

// open object modes
//#define OM_READ			0x80000000
//#define OM_WRITE		0x40000000
//#define OM_CREATE		0x00000001	/* create, if exists - fails */
//#define OM_CREATEALWAYS	0x00000002	/* always creates the file */
//#define OM_OPENEXISTING	0x00000003	/* opens only if exists */
//#define OM_OPENALWAYS	0x00000004	/* always opens the object */

#endif