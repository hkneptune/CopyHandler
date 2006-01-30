/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Oct 10 12:09:27 2004
 */
/* Compiler settings for F:\projects\c++\working\Copy Handler\CopyHandlerShellExt\CopyHandlerShellExt.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )


/* verify that the <rpcndr.h> version is high enough to compile this file*/
#ifndef __REQUIRED_RPCNDR_H_VERSION__
#define __REQUIRED_RPCNDR_H_VERSION__ 440
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

#ifdef __cplusplus
extern "C"{
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

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

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
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IMenuExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IMenuExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IMenuExt __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IMenuExt __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IMenuExt __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IMenuExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IMenuExt __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IMenuExtVtbl;

    interface IMenuExt
    {
        CONST_VTBL struct IMenuExtVtbl __RPC_FAR *lpVtbl;
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
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IDropMenuExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IDropMenuExt __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IDropMenuExt __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfoCount )( 
            IDropMenuExt __RPC_FAR * This,
            /* [out] */ UINT __RPC_FAR *pctinfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTypeInfo )( 
            IDropMenuExt __RPC_FAR * This,
            /* [in] */ UINT iTInfo,
            /* [in] */ LCID lcid,
            /* [out] */ ITypeInfo __RPC_FAR *__RPC_FAR *ppTInfo);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetIDsOfNames )( 
            IDropMenuExt __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [size_is][in] */ LPOLESTR __RPC_FAR *rgszNames,
            /* [in] */ UINT cNames,
            /* [in] */ LCID lcid,
            /* [size_is][out] */ DISPID __RPC_FAR *rgDispId);
        
        /* [local] */ HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Invoke )( 
            IDropMenuExt __RPC_FAR * This,
            /* [in] */ DISPID dispIdMember,
            /* [in] */ REFIID riid,
            /* [in] */ LCID lcid,
            /* [in] */ WORD wFlags,
            /* [out][in] */ DISPPARAMS __RPC_FAR *pDispParams,
            /* [out] */ VARIANT __RPC_FAR *pVarResult,
            /* [out] */ EXCEPINFO __RPC_FAR *pExcepInfo,
            /* [out] */ UINT __RPC_FAR *puArgErr);
        
        END_INTERFACE
    } IDropMenuExtVtbl;

    interface IDropMenuExt
    {
        CONST_VTBL struct IDropMenuExtVtbl __RPC_FAR *lpVtbl;
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
