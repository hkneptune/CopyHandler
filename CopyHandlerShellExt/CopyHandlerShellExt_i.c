/* this file contains the actual definitions of */
/* the IIDs and CLSIDs */

/* link this file in with the server and any clients */


/* File created by MIDL compiler version 5.01.0164 */
/* at Sun Oct 10 12:09:27 2004
 */
/* Compiler settings for F:\projects\c++\working\Copy Handler\CopyHandlerShellExt\CopyHandlerShellExt.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#ifdef __cplusplus
extern "C"{
#endif 


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

const IID IID_IMenuExt = {0x413AA618,0xE769,0x4E6E,{0xA6,0x10,0x7B,0xDC,0x8A,0x18,0x9F,0xB2}};


const IID IID_IDropMenuExt = {0x4AEAD637,0x8A55,0x47B9,{0xAA,0x1A,0xDA,0xCE,0xA3,0xDE,0x9B,0x71}};


const IID LIBID_COPYHANDLERSHELLEXTLib = {0x68FAFC14,0x8EB8,0x4DA1,{0x90,0xEB,0x6B,0x3D,0x22,0x01,0x05,0x05}};


const CLSID CLSID_MenuExt = {0xE7A4C2DA,0xF3AF,0x4145,{0xAC,0x19,0xE3,0xB2,0x15,0x30,0x6A,0x54}};


const CLSID CLSID_DropMenuExt = {0xB46F8244,0x86E6,0x43CF,{0xB8,0xAB,0x8C,0x3A,0x89,0x92,0x8A,0x48}};


#ifdef __cplusplus
}
#endif

