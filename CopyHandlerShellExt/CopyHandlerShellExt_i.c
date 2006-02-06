

/* this ALWAYS GENERATED file contains the IIDs and CLSIDs */

/* link this file in with the server and any clients */


 /* File created by MIDL compiler version 6.00.0361 */
/* at Mon Feb 06 02:08:15 2006
 */
/* Compiler settings for .\CopyHandlerShellExt.idl:
    Oicf, W1, Zp8, env=Win32 (32b run)
    protocol : dce , ms_ext, c_ext, robust
    error checks: allocation ref bounds_check enum stub_data 
    VC __declspec() decoration level: 
         __declspec(uuid()), __declspec(selectany), __declspec(novtable)
         DECLSPEC_UUID(), MIDL_INTERFACE()
*/
//@@MIDL_FILE_HEADING(  )

#if !defined(_M_IA64) && !defined(_M_AMD64)


#pragma warning( disable: 4049 )  /* more than 64k source lines */


#ifdef __cplusplus
extern "C"{
#endif 


#include <rpc.h>
#include <rpcndr.h>

#ifdef _MIDL_USE_GUIDDEF_

#ifndef INITGUID
#define INITGUID
#include <guiddef.h>
#undef INITGUID
#else
#include <guiddef.h>
#endif

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8)

#else // !_MIDL_USE_GUIDDEF_

#ifndef __IID_DEFINED__
#define __IID_DEFINED__

typedef struct _IID
{
    unsigned long x;
    unsigned short s1;
    unsigned short s2;
    unsigned char  c[8];
} IID;

#endif // __IID_DEFINED__

#ifndef CLSID_DEFINED
#define CLSID_DEFINED
typedef IID CLSID;
#endif // CLSID_DEFINED

#define MIDL_DEFINE_GUID(type,name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) \
        const type name = {l,w1,w2,{b1,b2,b3,b4,b5,b6,b7,b8}}

#endif !_MIDL_USE_GUIDDEF_

MIDL_DEFINE_GUID(IID, IID_IMenuExt,0x413AA618,0xE769,0x4E6E,0xA6,0x10,0x7B,0xDC,0x8A,0x18,0x9F,0xB2);


MIDL_DEFINE_GUID(IID, IID_IDropMenuExt,0x4AEAD637,0x8A55,0x47B9,0xAA,0x1A,0xDA,0xCE,0xA3,0xDE,0x9B,0x71);


MIDL_DEFINE_GUID(IID, LIBID_COPYHANDLERSHELLEXTLib,0x68FAFC14,0x8EB8,0x4DA1,0x90,0xEB,0x6B,0x3D,0x22,0x01,0x05,0x05);


MIDL_DEFINE_GUID(CLSID, CLSID_MenuExt,0xE7A4C2DA,0xF3AF,0x4145,0xAC,0x19,0xE3,0xB2,0x15,0x30,0x6A,0x54);


MIDL_DEFINE_GUID(CLSID, CLSID_DropMenuExt,0xB46F8244,0x86E6,0x43CF,0xB8,0xAB,0x8C,0x3A,0x89,0x92,0x8A,0x48);

#undef MIDL_DEFINE_GUID

#ifdef __cplusplus
}
#endif



#endif /* !defined(_M_IA64) && !defined(_M_AMD64)*/

