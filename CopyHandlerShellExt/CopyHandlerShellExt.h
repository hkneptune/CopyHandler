

/* this ALWAYS GENERATED file contains the definitions for the interfaces */


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

#pragma warning( disable: 4049 )  /* more than 64k source lines */


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#endif

#include "rpc.h"
#include "rpcndr.h"

#ifndef __RPCNDR_H_VERSION__
#error this stub requires an updated version of <rpcndr.h>
#endif // __RPCNDR_H_VERSION__

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __CopyHandlerShellExt_h__
#define __CopyHandlerShellExt_h__

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

/* Forward Declarations */ 

#ifndef __IMenuExt_FWD_DEFINED__
#define __IMenuExt_FWD_DEFINED__
typedef interface IMenuExt IMenuExt;
#endif 	/* __IMenuExt_FWD_DEFINED__ */


#ifndef __IDropMenuExt_FWD_DEFINED__
#define __IDropMenuExt_FWD_DEFINED__
typedef interface IDropMenuExt IDropMenuExt;
#endif 	/* __IDropMenuExt_FWD_DEFINED__ */


#ifndef __MenuExt_FWD_DEFINED__
#define __MenuExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class MenuExt MenuExt;
#else
typedef struct MenuExt MenuExt;
#endif /* __cplusplus */

#endif 	/* __MenuExt_FWD_DEFINED__ */


#ifndef __DropMenuExt_FWD_DEFINED__
#define __DropMenuExt_FWD_DEFINED__

#ifdef __cplusplus
typedef class DropMenuExt DropMenuExt;
#else
typedef struct DropMenuExt DropMenuExt;
#endif /* __cplusplus */

#endif 	/* __DropMenuExt_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "ocidl.h"

#ifdef __cplusplus
extern "C"{
#endif 

void * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void * ); 

#ifndef __IMenuExt_INTERFACE_DEFINED__
#define __IMenuExt_INTERFACE_DEFINED__

/* interface IMenuExt */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IMenuExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("413AA618-E769-4E6E-A610-7BDC8A189FB2")
    IMenuExt : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IMenuExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IMenuExt * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IMenuExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IMenuExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IMenuExt * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IMenuExt * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IMenuExt * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IMenuExt * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IMenuExtVtbl;

    interface IMenuExt
    {
        CONST_VTBL struct IMenuExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IMenuExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IMenuExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IMenuExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IMenuExt_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IMenuExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IMenuExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IMenuExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IMenuExt_INTERFACE_DEFINED__ */


#ifndef __IDropMenuExt_INTERFACE_DEFINED__
#define __IDropMenuExt_INTERFACE_DEFINED__

/* interface IDropMenuExt */
/* [unique][helpstring][dual][uuid][object] */ 


EXTERN_C const IID IID_IDropMenuExt;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    MIDL_INTERFACE("4AEAD637-8A55-47B9-AA1A-DACEA3DE9B71")
    IDropMenuExt : public IDispatch
    {
    public:
    };
    
#else 	/* C style interface */

    typedef struct IDropMenuExtVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE *QueryInterface )( 
            IDropMenuExt * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void **ppvObject);
        
        ULONG ( STDMETHODCALLTYPE *AddRef )( 
            IDropMenuExt * This);
        
        ULONG ( STDMETHODCALLTYPE *Release )( 
            IDropMenuExt * This);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfoCount )( 
            IDropMenuExt * This,
            /* [out] */ UINT *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetTypeInfo )( 
            IDropMenuExt * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo **ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE *GetIDsOfNames )( 
            IDropMenuExt * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE *Invoke )( 
            IDropMenuExt * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS *pDispParams,
            /* [out] */ VARIANT *pVarResult,
            /* [out] */ EXCEPINFO *pExcepInfo,
            /* [out] */ UINT *puArgErr);
        
        END_INTERFACE
    } IDropMenuExtVtbl;

    interface IDropMenuExt
    {
        CONST_VTBL struct IDropMenuExtVtbl *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IDropMenuExt_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IDropMenuExt_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IDropMenuExt_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IDropMenuExt_GetTypeInfoCount(This,pctinfo)	\
    (This)->lpVtbl -> GetTypeInfoCount(This,pctinfo)

#define IDropMenuExt_GetTypeInfo(This,iTInfo,lcid,ppTInfo)	\
    (This)->lpVtbl -> GetTypeInfo(This,iTInfo,lcid,ppTInfo)

#define IDropMenuExt_GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)	\
    (This)->lpVtbl -> GetIDsOfNames(This,riid,rgszNames,cNames,lcid,rgDispId)

#define IDropMenuExt_Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)	\
    (This)->lpVtbl -> Invoke(This,dispIdMember,riid,lcid,wFlags,pDispParams,pVarResult,pExcepInfo,puArgErr)


#endif /* COBJMACROS */


#endif 	/* C style interface */




#endif 	/* __IDropMenuExt_INTERFACE_DEFINED__ */



#ifndef __COPYHANDLERSHELLEXTLib_LIBRARY_DEFINED__
#define __COPYHANDLERSHELLEXTLib_LIBRARY_DEFINED__

/* library COPYHANDLERSHELLEXTLib */
/* [helpstring][version][uuid] */ 


EXTERN_C const IID LIBID_COPYHANDLERSHELLEXTLib;

EXTERN_C const CLSID CLSID_MenuExt;

#ifdef __cplusplus

class DECLSPEC_UUID("E7A4C2DA-F3AF-4145-AC19-E3B215306A54")
MenuExt;
#endif

EXTERN_C const CLSID CLSID_DropMenuExt;

#ifdef __cplusplus

class DECLSPEC_UUID("B46F8244-86E6-43CF-B8AB-8C3A89928A48")
DropMenuExt;
#endif
#endif /* __COPYHANDLERSHELLEXTLib_LIBRARY_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif


