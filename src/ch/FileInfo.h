/***************************************************************************
*   Copyright (C) 2001-2008 by Józef Starosczyk                           *
*   ixen@copyhandler.com                                                  *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU Library General Public License          *
*   (version 2) as published by the Free Software Foundation;             *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU Library General Public     *
*   License along with this program; if not, write to the                 *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
/**
 * @doc FILEINFO
 * @module FileInfo.h 1.3 - Interface for the CFileInfo and CFileInfoArray classes |
 * The classes contained in this file allow to gather recursively file information
 * through directories.
 *
 * <cp> Codeguru & friends
 * Coded by Antonio Tejada Lacaci. 1999<nl>
 * atejada@espanet.com<nl>
 * CRC32 code by Floor A.C. Naaijkens 
 * 
 * Updates (aaaa-mm-dd):<nl>
 *  MANY CHANGES by Ixen Gerthannes...
 *  1999-9-23 ATL: Opensource works! Again, Mr. Szucs (rszucs@cygron.hu) gets another bug:<nl>
 *                Missing "4-(dwRead & 0x3)" in the same lines as before, when calc'ing the padding mask.
 *  1999-9-16 ATL: Corrected bug in GetCRC and GetChecksum as suggested by Róbert Szucs (rszucs@cygron.hu):<nl>
 *                There was a buffer overflow and checksum and crc for last dword +1 was calc'ed instead 
 *                of the ones for last dword. Instead accessing buffer[dwRead +3...] it ought to access 
 *                  buffer[dwRead...] (shame on me! :'().
 *  1999-9-2 ATL: Corrected bug in AddFile(CString, LPARAM) as suggested by Nhycoh (Nhycoh44@yahoo.com):<nl>
 *                There was some weird stuff at CFileInfo::Create(strFilePath) <nl>
 *                stating strFilePath.GetLength()-nBarPos instead of nBarPos+1
 *                (I'm quite sure I left my head on my pillow the day I did that %-#).
 *  1999-6-27 ATL: Updated GetCRC & GetChecksum to avoid some bug cases<nl>
 *  1999-4-7 ATL: Updated source code doc to conform Autoduck 2.0 standard<nl>
 *  1999-4-7 ATL: Corrected bug in AddDir as suggested by Zhuang Yuyao (zhuangyy@koal.com):<nl>
 *                bIncludeDirs wasn't used if bRecurse was false.
 *
 * Keep this comment if you redistribute this file. And credit where credit's due!
 */

#ifndef __FILEINFO_H__
#define __FILEINFO_H__

#include <afxtempl.h>
#include "afxdisp.h"
#include "DestPath.h"

void FindFreeSubstituteName(CString strSrcPath, CString strDstPath, CString* pstrResult);
extern void GetDriveData(LPCTSTR lpszPath, int *piDrvNum, UINT *puiDrvType);

// CFileInfo flags
// flag stating that file has been processed (used to determine if file can be deleted at the end of copying)     
#define FIF_PROCESSED		0x00000001

class CFiltersArray;
/////////////////////////////////////////////////////////////////////////////
// CClipboardEntry
class CClipboardEntry
{
public:
	CClipboardEntry() { m_bMove=true; m_iDriveNumber=-1; m_uiDriveType=static_cast<UINT>(-1); m_iBufferIndex=0; };
	CClipboardEntry(const CClipboardEntry& rEntry) { m_strPath=rEntry.m_strPath; m_bMove=rEntry.m_bMove; m_iDriveNumber=rEntry.m_iDriveNumber; m_uiDriveType=rEntry.m_uiDriveType; m_astrDstPaths.Copy(rEntry.m_astrDstPaths); };
	
	void SetPath(const CString& strPath);
	void CalcBufferIndex(const CDestPath& dpDestPath);
	const CString& GetPath() const { return m_strPath; };

	void SetMove(bool bValue) { m_bMove=bValue; }; 
	bool GetMove() { return m_bMove; };

	int GetDriveNumber() const { return m_iDriveNumber; };
	UINT GetDriveType() const { return m_uiDriveType; };

	int GetBufferIndex() const { return m_iBufferIndex; };

	void Serialize(icpf::archive& ar, bool bData);

private:
	CString m_strPath;				// path (ie. c:\\windows\\) - always with ending '\\'
	bool m_bMove;					// specifies if we can use MoveFile (if will be moved)

	int m_iDriveNumber;		// disk number (-1 - none)
	UINT m_uiDriveType;		// path type

	int m_iBufferIndex;		// buffer number, with which we'll copy this data

public:
	CStringArray m_astrDstPaths;	// dest paths table for this group of paths
};

//////////////////////////////////////////////////////////////////////////
// CClipboardArray

class CClipboardArray : public CArray<CClipboardEntry*, CClipboardEntry*>
{
public:
	~CClipboardArray() { RemoveAll(); };
	
	void Serialize(icpf::archive& ar, bool bData);

	void SetAt(int nIndex, CClipboardEntry* pEntry) { delete [] GetAt(nIndex); SetAt(nIndex, pEntry); };
	void RemoveAt(int nIndex, int nCount = 1) { while (nCount--) { delete GetAt(nIndex); static_cast<CArray<CClipboardEntry*, CClipboardEntry*>*>(this)->RemoveAt(nIndex, 1); } };
	void RemoveAll() { for (int i=0;i<GetSize();i++) delete GetAt(i); static_cast<CArray<CClipboardEntry*, CClipboardEntry*>*>(this)->RemoveAll(); };
};

class CFileInfo
{  
public:
	CFileInfo();
	CFileInfo(const CFileInfo& finf);
	~CFileInfo();

	// static member
	static bool Exist(CString strPath);	// check for file or folder existence
	
	void Create(const WIN32_FIND_DATA* pwfd, LPCTSTR pszFilePath, int iSrcIndex);
	bool Create(CString strFilePath, int iSrcIndex);
	
	DWORD GetLength(void) const { return (DWORD) m_uhFileSize; };
	ULONGLONG GetLength64(void) const { return m_uhFileSize; };
	void SetLength64(ULONGLONG uhSize) { m_uhFileSize=uhSize; };

	// disk - path and disk number (-1 if none - ie. net disk)
	CString GetFileDrive(void) const;		// returns string with src disk
	int GetDriveNumber() const;				// disk number A - 0, b-1, c-2, ...
	UINT GetDriveType() const;				// drive type
	
	CString GetFileDir(void) const;	// @rdesc Returns \WINDOWS\ for C:\WINDOWS\WIN.INI 
	CString GetFileTitle(void) const;	// @cmember returns WIN for C:\WINDOWS\WIN.INI
	CString GetFileExt(void) const;		/** @cmember returns INI for C:\WINDOWS\WIN.INI */
	CString GetFileRoot(void) const;	/** @cmember returns C:\WINDOWS\ for C:\WINDOWS\WIN.INI */
	CString GetFileName(void) const;	/** @cmember returns WIN.INI for C:\WINDOWS\WIN.INI */
	
	const CString& GetFilePath(void) const { return m_strFilePath; }	// returns path with m_strFilePath (probably not full)
	CString GetFullFilePath() const;		/** @cmember returns C:\WINDOWS\WIN.INI for C:\WINDOWS\WIN.INI */
	void SetFilePath(LPCTSTR lpszPath) { m_strFilePath=lpszPath; };
	
	/* Get File times info (equivalent to CFindFile members) */
	const COleDateTime& GetCreationTime(void) const { return m_timCreation; };	/** @cmember returns creation time */
	const COleDateTime& GetLastAccessTime(void) const { return m_timLastAccess; };	/** @cmember returns last access time */
	const COleDateTime& GetLastWriteTime(void) const { return m_timLastWrite; };	/** @cmember returns las write time */
	
	/* Get File attributes info (equivalent to CFindFile members) */
	DWORD GetAttributes(void) const { return m_dwAttributes; };	/** @cmember returns file attributes */
	bool IsDirectory(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0; };	/** @cmember returns TRUE if the file is a directory */
	bool IsArchived(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_ARCHIVE) != 0; };	/** @cmember Returns TRUE if the file has archive bit set */
	bool IsReadOnly(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_READONLY) != 0; };	/** @cmember Returns TRUE if the file is read-only */
	bool IsCompressed(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_COMPRESSED) != 0; };	/** @cmember Returns TRUE if the file is compressed */
	bool IsSystem(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_SYSTEM) != 0; };	/** @cmember Returns TRUE if the file is a system file */
	bool IsHidden(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_HIDDEN) != 0; };	/** @cmember Returns TRUE if the file is hidden */
	bool IsTemporary(void) const { return (m_dwAttributes & FILE_ATTRIBUTE_TEMPORARY) != 0; };	/** @cmember Returns TRUE if the file is temporary */
	bool IsNormal(void) const { return m_dwAttributes == 0; };	/** @cmember Returns TRUE if the file is a normal file */

	uint_t GetFlags() const { return m_uiFlags; }
	void SetFlags(uint_t uiFlags, uint_t uiMask = 0xffffffff) { m_uiFlags = (m_uiFlags & ~(uiFlags & uiMask)) | (uiFlags & uiMask); }

	// operations
	void SetClipboard(CClipboardArray *pClipboard) { m_pClipboard=pClipboard; };
	CString GetDestinationPath(CString strPath, unsigned char ucCopyNumber, int iFlags);

	void SetSrcIndex(int iIndex) { m_iSrcIndex=iIndex; };
	int GetSrcIndex() const { return m_iSrcIndex; };

	bool GetMove() { if (m_iSrcIndex != -1) return m_pClipboard->GetAt(m_iSrcIndex)->GetMove(); else return true; };

	int GetBufferIndex() const;

	// operators
	bool operator==(const CFileInfo& rInfo);
	
	// (re)/store data
	void Store(icpf::archive& ar);
	void Load(icpf::archive& ar);
private:
	CString m_strFilePath;	// contains relative path (first path is in CClipboardArray)
	int m_iSrcIndex;		// index in CClipboardArray table (which contains the first part of the path)
	
	DWORD m_dwAttributes;	// attributes
	ULONGLONG m_uhFileSize;	 /** @cmember File of size. (COM states LONGLONG as hyper, so "uh" means unsigned hyper) */
	COleDateTime m_timCreation;    /** @cmember Creation time */
	COleDateTime m_timLastAccess;  /** @cmember Last Access time */
	COleDateTime m_timLastWrite;   /** @cmember Last write time */

	uint_t m_uiFlags;
	// ptrs to elements providing data
	CClipboardArray* m_pClipboard;
}; 

/**
* @class Allows to retrieve <c CFileInfo>s from files/directories in a directory
*/
class CFileInfoArray : public CArray<CFileInfo, CFileInfo&>
{
public:
	/** @access Public members */
	
	/**
    * @cmember Default constructor
    */
	CFileInfoArray(CClipboardArray& A_rClipboardArray) :
		m_rClipboard(A_rClipboardArray)
	{
		
		SetSize(0, 5000);
	}

	/**
    * @cmember Adds a file or all contained in a directory to the CFileInfoArray
    * Only "static" data for CFileInfo is filled (by default CRC and checksum are NOT 
    * calculated when inserting CFileInfos).<nl> Returns the number of <c CFileInfo>s added to the array
    * @parm Name of the directory, ended in backslash.
    * @parm Mask of files to add in case that strDirName is a directory
    * @parm Wether to recurse or not subdirectories
    * @parmopt Parameter to pass to protected member function AddFileInfo
    * @parmopt Wether to add or not CFileInfos for directories
    * @parmopt Pointer to a variable to signal abort of directory retrieval 
    * (multithreaded apps).
    * @parmopt pulCount Pointer to a variable incremented each time a CFileInfo is added to the
    * array (multithreaded apps).
    * @xref <mf CFileInfoArray.AddFile> <mf CFileInfoArray.AddFileInfo> <md CFileInfoArray.AP_NOSORT>
    */
	void AddDir(CString strDirName, const CFiltersArray* pFilters, int iSrcIndex,
		const bool bRecurse, const bool bIncludeDirs, const volatile bool* pbAbort=NULL);
	
	/**
	* @cmember Adds a single file or directory to the CFileInfoArray. In case of directory, files
	* contained in the directory are NOT added to the array.<nl>
	* Returns the position in the array where the <c CFileInfo> was added (-1 if <c CFileInfo>
	* wasn't added)
	* @parm Name of the file or directory to add. NOT ended with backslash.
	* @parm Parameter to pass to protected member function AddFileInfo.
	* @xref <mf CFileInfoArray.AddDir> <mf CFileInfoArray.AddFileInfo>
    */
	int AddFile(CString strFilePath, int iSrcIndex);
	
	// store/restore
	void Store(icpf::archive& ar, bool bOnlyFlags)
	{
		INT_PTR iSize = GetSize();
		ar << iSize;
		for (INT_PTR i=0;i<iSize;i++)
		{
			CFileInfo& fi=GetAt(i);
			if(bOnlyFlags)
				ar << fi.GetFlags();
			else
				fi.Store(ar);
		}
	}

	void Load(icpf::archive& ar, bool bOnlyFlags)
	{
		INT_PTR iSize;
		ar>>iSize;
		SetSize(iSize, 5000);
		CFileInfo fi;
		fi.SetClipboard(&m_rClipboard);
		uint_t uiFlags = 0;
		for (INT_PTR i=0;i<iSize;i++)
		{
			if(bOnlyFlags)
			{
				CFileInfo& rInfo = GetAt(i);
				ar >> uiFlags;
				rInfo.SetFlags(uiFlags);
			}
			else
			{
				fi.Load(ar);
				SetAt(i, fi);
			}
		}
	}
	
protected:
	CClipboardArray& m_rClipboard;
};


/**
@ex This code adds all files in root directory and its subdirectories (but not directories themselves) to the array and TRACEs them: |

	CFileInfoArray fia;
  
	fia.AddDir(
	"C:\\",                                     // Directory
	"*.*",                                      // Filemask (all files)
	TRUE,                                       // Recurse subdirs
	fia::AP_SORTBYNAME | fia::AP_SORTASCENDING, // Sort by name and ascending
	FALSE                                       // Do not add array entries for directories (only for files)
	);
	TRACE("Dumping directory contents\n");
	for (int i=0;i<fia.GetSize();i++) TRACE(fia[i].GetFilePath()+"\n");
	
	@ex You can also call AddDir multiple times. The example shows files in root directories (but not subdirectories) of C:\\ and D:\\: |
	  
	CFileInfoArray fia;
		
	// Note both AddDir use the same sorting order and direction
	fia.AddDir("C:\\", "*.*", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTASCENDING, FALSE );
	fia.AddDir("D:\\", "*.*", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTASCENDING, FALSE );
	TRACE("Dumping directory contents for C:\\ and D:\\ \n");
	for (int i=0;i<fia.GetSize();i++) TRACE(fia[i].GetFilePath()+"\n");
		  
			
	@ex Or you can add individual files: |
			  
	CFileInfoArray fin;
				
	// Note both AddDir and AddFile must use the same sorting order and direction
	fia.AddDir("C:\\WINDOWS\\", "*.*", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTDESCENDING, FALSE );
	fia.AddFile("C:\\AUTOEXEC.BAT", fia::AP_SORTBYNAME | fia::SORTDESCENDING);
	TRACE("Dumping directory contents for C:\\WINDOWS\\ and file C:\\AUTOEXEC.BAT\n");
	for (int i=0;i<fia.GetSize();i++) TRACE(fia[i].GetFilePath()+"\n");
				  
	@ex And mix directories with individual files:  |
					
	CFileInfoArray fin;
					  
	// Note both AddDir and AddFile must use the same sorting order and direction
	// Note also the list of filemasks *.EXE and *.COM
	fia.AddDir("C:\\WINDOWS\\", "*.EXE;*.COM", FALSE, fia::AP_SORTBYNAME | fia::AP_SORTDESCENDING, FALSE );
	fia.AddFile("C:\\AUTOEXEC.BAT", fia::AP_SORTBYNAME | fia::SORTDESCENDING);
	// Note no trailing bar for next AddFile (we want to insert an entry for the directory
	// itself, not for the files inside the directory)
	fia.AddFile("C:\\PROGRAM FILES", fia::AP_SORTBYNAME | fia::SORTDESCENDING);
	TRACE("Dumping directory contents for C:\\WINDOWS\\, file C:\\AUTOEXEC.BAT and "
	" directory \"C:\\PROGRAM FILES\" \n");
	for (int i=0;i<fia.GetSize();i++) TRACE(fia[i].GetFilePath+"\n");
						
	*/
						  
#endif
