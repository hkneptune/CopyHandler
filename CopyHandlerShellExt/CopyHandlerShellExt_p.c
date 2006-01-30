/* this ALWAYS GENERATED file contains the proxy stub code */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Oct 10 12:09:27 2004
 */
/* Compiler settings for F:\projects\c++\working\Copy Handler\CopyHandlerShellExt\CopyHandlerShellExt.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )

#define USE_STUBLESS_PROXY


/* verify that the <rpcproxy.h> version is high enough to compile this file*/
#ifndef __REDQ_RPCPROXY_H_VERSION__
#define __REQUIRED_RPCPROXY_H_VERSION__ 440
#endif


#include "rpcproxy.h"
#ifndef __RPCPROXY_H_VERSION__
#error this stub requires an updated version of <rpcproxy.h>
#endif // __RPCPROXY_H_VERSION__


#include "CopyHandlerShellExt.h"

#define TYPE_FORMAT_STRING_SIZE   3                                 
#define PROC_FORMAT_STRING_SIZE   1                                 

typedef struct _MIDL_TYPE_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ TYPE_FORMAT_STRING_SIZE ];
    } MIDL_TYPE_FORMAT_STRING;

typedef struct _MIDL_PROC_FORMAT_STRING
    {
    short          Pad;
    unsigned char  Format[ PROC_FORMAT_STRING_SIZE ];
    } MIDL_PROC_FORMAT_STRING;


extern const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString;
extern const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString;


/* Object interface: IUnknown, ver. 0.0,
   GUID={0x00000000,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IDispatch, ver. 0.0,
   GUID={0x00020400,0x0000,0x0000,{0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x46}} */


/* Object interface: IMenuExt, ver. 0.0,
   GUID={0x413AA618,0xE769,0x4E6E,{0xA6,0x10,0x7B,0xDC,0x8A,0x18,0x9F,0xB2}} */


extern const MIDL_STUB_DESC Object_StubDesc;


#pragma code_seg(".orpc")
CINTERFACE_PROXY_VTABLE(7) _IMenuExtProxyVtbl = 
{
    0,
    &IID_IMenuExt,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *)-1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *)-1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *)-1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


static const PRPC_STUB_FUNCTION IMenuExt_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IMenuExtStubVtbl =
{
    &IID_IMenuExt,
    0,
    7,
    &IMenuExt_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};


/* Object interface: IDropMenuExt, ver. 0.0,
   GUID={0x4AEAD637,0x8A55,0x47B9,{0xAA,0x1A,0xDA,0xCE,0xA3,0xDE,0x9B,0x71}} */


extern const MIDL_STUB_DESC Object_StubDesc;


#pragma code_seg(".orpc")

static const MIDL_STUB_DESC Object_StubDesc = 
    {
    0,
    NdrOleAllocate,
    NdrOleFree,
    0,
    0,
    0,
    0,
    0,
    __MIDL_TypeFormatString.Format,
    1, /* -error bounds_check flag */
    0x20000, /* Ndr library version */
    0,
    0x50100a4, /* MIDL Version 5.1.164 */
    0,
    0,
    0,  /* notify & notify_flag routine table */
    1,  /* Flags */
    0,  /* Reserved3 */
    0,  /* Reserved4 */
    0   /* Reserved5 */
    };

CINTERFACE_PROXY_VTABLE(7) _IDropMenuExtProxyVtbl = 
{
    0,
    &IID_IDropMenuExt,
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy ,
    0 /* (void *)-1 /* IDispatch::GetTypeInfoCount */ ,
    0 /* (void *)-1 /* IDispatch::GetTypeInfo */ ,
    0 /* (void *)-1 /* IDispatch::GetIDsOfNames */ ,
    0 /* IDispatch_Invoke_Proxy */
};


static const PRPC_STUB_FUNCTION IDropMenuExt_table[] =
{
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION,
    STUB_FORWARDING_FUNCTION
};

CInterfaceStubVtbl _IDropMenuExtStubVtbl =
{
    &IID_IDropMenuExt,
    0,
    7,
    &IDropMenuExt_table[-3],
    CStdStubBuffer_DELEGATING_METHODS
};

#pragma data_seg(".rdata")

#if !defined(__RPC_WIN32__)
#error  Invalid build platform for this stub.
#endif

#if !(TARGET_IS_NT40_OR_LATER)
#error You need a Windows NT 4.0 or later to run this stub because it uses these features:
#error   -Oif or -Oicf, more than 32 methods in the interface.
#error However, your C/C++ compilation flags indicate you intend to run this app on earlier systems.
#error This app will die there with the RPC_X_WRONG_STUB_VERSION error.
#endif


static const MIDL_PROC_FORMAT_STRING __MIDL_ProcFormatString =
    {
        0,
        {

			0x0
        }
    };

static const MIDL_TYPE_FORMAT_STRING __MIDL_TypeFormatString =
    {
        0,
        {
			NdrFcShort( 0x0 ),	/* 0 */

			0x0
        }
    };

const CInterfaceProxyVtbl * _CopyHandlerShellExt_ProxyVtblList[] = 
{
    ( CInterfaceProxyVtbl *) &_IMenuExtProxyVtbl,
    ( CInterfaceProxyVtbl *) &_IDropMenuExtProxyVtbl,
    0
};

const CInterfaceStubVtbl * _CopyHandlerShellExt_StubVtblList[] = 
{
    ( CInterfaceStubVtbl *) &_IMenuExtStubVtbl,
    ( CInterfaceStubVtbl *) &_IDropMenuExtStubVtbl,
    0
};

PCInterfaceName const _CopyHandlerShellExt_InterfaceNamesList[] = 
{
    "IMenuExt",
    "IDropMenuExt",
    0
};

const IID *  _CopyHandlerShellExt_BaseIIDList[] = 
{
    &IID_IDispatch,
    &IID_IDispatch,
    0
};


#define _CopyHandlerShellExt_CHECK_IID(n)	IID_GENERIC_CHECK_IID( _CopyHandlerShellExt, pIID, n)

int __stdcall _CopyHandlerShellExt_IID_Lookup( const IID * pIID, int * pIndex )
{
    IID_BS_LOOKUP_SETUP

    IID_BS_LOOKUP_INITIAL_TEST( _CopyHandlerShellExt, 2, 1 )
    IID_BS_LOOKUP_RETURN_RESULT( _CopyHandlerShellExt, 2, *pIndex )
    
}

const ExtendedProxyFileInfo CopyHandlerShellExt_ProxyFileInfo = 
{
    (PCInterfaceProxyVtblList *) & _CopyHandlerShellExt_ProxyVtblList,
    (PCInterfaceStubVtblList *) & _CopyHandlerShellExt_StubVtblList,
    (const PCInterfaceName * ) & _CopyHandlerShellExt_InterfaceNamesList,
    (const IID ** ) & _CopyHandlerShellExt_BaseIIDList,
    & _CopyHandlerShellExt_IID_Lookup, 
    2,
    2,
    0, /* table of [async_uuid] interfaces */
    0, /* Filler1 */
    0, /* Filler2 */
    0  /* Filler3 */
};
