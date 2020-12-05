#include "stdafx.h"
#include "TDestinationPathProvider.h"
#include "../libchcore/TCoreException.h"
#include "../libchcore/ErrorCodes.h"
#include <boost/lexical_cast.hpp>

using namespace chcore;
using namespace string;

namespace chengine
{
	TDestinationPathProvider::TDestinationPathProvider(const IFilesystemPtr& spFilesystem,
		const TSmartPath& pathDestinationBase,
		bool bIgnoreFolders,
		bool bForceDirectories,
		const TString& strFirstAltName,
		const TString& strNextAltName) :

		m_spFilesystem(spFilesystem),
		m_pathDestinationBase(pathDestinationBase),
		m_bIgnoreFolders(bIgnoreFolders),
		m_bForceDirectories(bForceDirectories),
		m_strFirstAltName(strFirstAltName),
		m_strNextAltName(strNextAltName)
	{
		if(!spFilesystem)
			throw TCoreException(eErr_InvalidArgument, L"spFilesystem", LOCATION);
	}

	TSmartPath TDestinationPathProvider::CalculateDestinationPath(const TFileInfoPtr& spFileInfo) const
	{
		if(!spFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spFileInfo", LOCATION);

		TSmartPath plannedDestinationPath;
		if (m_bForceDirectories)
			plannedDestinationPath = CalculateForceDirectories(spFileInfo);
		else if (m_bIgnoreFolders)
			plannedDestinationPath = CalculateIgnoreDirectories(spFileInfo);
		else
			plannedDestinationPath = CalculateNormalDestination(spFileInfo);

		// adjust the calculated path with different filename if set previously by renaming
		TSmartPath pathDstReplacement = spFileInfo->GetDstRelativePath();
		if(!pathDstReplacement.IsEmpty())
		{
			plannedDestinationPath.DeleteFileName();
			plannedDestinationPath += pathDstReplacement;
		}

		return plannedDestinationPath;
	}

	chcore::TSmartPath TDestinationPathProvider::CalculateSuggestedDestinationPath(chcore::TSmartPath pathDst) const
	{
		// get the name from src path
		pathDst.StripSeparatorAtEnd();

		TSmartPath pathFilename = pathDst.GetFileName();
		pathFilename.StripPath(L":");

		// get rid of extracted filename to get the parent dir
		pathDst.DeleteFileName();

		// set the dest path
		TString strCheckPath = m_strFirstAltName;
		strCheckPath.Replace(_T("%name"), pathFilename.GetFileTitle().ToString());
		strCheckPath.Replace(_T("%ext"), pathFilename.GetExtension().ToString());

		TSmartPath pathCheck(PathFromWString(strCheckPath));

		// when adding to strDstPath check if the path already exists - if so - try again
		int iCounter = 1;
		TString strFmt = m_strNextAltName;
		while(m_spFilesystem->PathExist(pathDst + pathCheck))
		{
			strCheckPath = strFmt;
			strCheckPath.Replace(_T("%name"), pathFilename.GetFileTitle().ToString());
			strCheckPath.Replace(_T("%ext"), pathFilename.GetExtension().ToString());
			strCheckPath.Replace(_T("%count"), boost::lexical_cast<std::wstring>(++iCounter).c_str());
			pathCheck.FromString(strCheckPath);
		}

		return pathCheck;
	}

	TSmartPath TDestinationPathProvider::CalculateForceDirectories(const TFileInfoPtr& spFileInfo) const
	{
		// force create directories
		TSmartPath tFileRoot = spFileInfo->GetFullFilePath().GetFileRoot();
		tFileRoot.StripPath(L":");

		TSmartPath pathCombined = m_pathDestinationBase + tFileRoot;

		// force create directory
		m_spFilesystem->CreateDirectory(pathCombined, true);

		TSmartPath pathFile = spFileInfo->GetFullFilePath().GetFileName();
		pathFile.StripPath(L":");
		TSmartPath pathResult = pathCombined + pathFile;
		pathResult.StripSeparatorAtEnd();

		return pathResult;
	}

	TSmartPath TDestinationPathProvider::CalculateIgnoreDirectories(const TFileInfoPtr& spFileInfo) const
	{
		TSmartPath pathFilename = spFileInfo->GetFullFilePath();
		pathFilename.StripPath(L":");
		pathFilename.StripSeparatorAtEnd();

		TSmartPath pathResult = m_pathDestinationBase + pathFilename.GetFileName();

		return pathResult;
	}

	chcore::TSmartPath TDestinationPathProvider::CalculateNormalDestination(const TFileInfoPtr& spFileInfo) const
	{
		TBasePathDataPtr spPathData = spFileInfo->GetBasePathData();
		if(!spPathData)
			throw TCoreException(eErr_InvalidArgument, L"FileInfo object does not contain base path data", LOCATION);

		TSmartPath plannedDestinationPath;

		// generate new top-level dest name for base path
		if(!spPathData->IsDestinationPathSet())
		{
			// generate something - if dest folder == src folder - search for copy
			if(m_pathDestinationBase == spFileInfo->GetFullFilePath().GetFileRoot())
			{
				TSmartPath pathSubst = FindFreeSubstituteName(spFileInfo->GetFullFilePath());
				spPathData->SetDestinationPath(pathSubst);
			}
			else
			{
				TSmartPath pathFilename = spPathData->GetSrcPath();
				pathFilename.StripSeparatorAtEnd();
				pathFilename.StripPath(L":");

				spPathData->SetDestinationPath(pathFilename.GetFileName());
			}
		}


		TSmartPath pathResult = m_pathDestinationBase + spPathData->GetDestinationPath() + spFileInfo->GetFilePath();
		pathResult.StripSeparatorAtEnd();
		return pathResult;
	}

	// finds another name for a copy of src file(folder) in dest location; works only for paths that ends up directly in m_pathDestinationBase (without sub-directories)
	TSmartPath TDestinationPathProvider::FindFreeSubstituteName(TSmartPath pathSrcPath) const
	{
		// get the name from src path
		pathSrcPath.StripSeparatorAtEnd();

		TSmartPath pathFilename = pathSrcPath.GetFileName();
		pathFilename.StripPath(L":");

		// set the dest path
		TString strCheckPath = m_strFirstAltName;
		strCheckPath.Replace(_T("%name"), pathFilename.GetFileTitle().ToString());
		strCheckPath.Replace(_T("%ext"), pathFilename.GetExtension().ToString());

		TSmartPath pathCheckPath(PathFromWString(strCheckPath));

		// when adding to strDstPath check if the path already exists - if so - try again
		int iCounter = 1;
		TString strFmt = m_strNextAltName;
		while(m_spFilesystem->PathExist(m_pathDestinationBase + pathCheckPath))
		{
			strCheckPath = strFmt;
			strCheckPath.Replace(_T("%name"), pathFilename.GetFileTitle().ToString());
			strCheckPath.Replace(_T("%ext"), pathFilename.GetExtension().ToString());
			strCheckPath.Replace(_T("%count"), boost::lexical_cast<std::wstring>(++iCounter).c_str());
			pathCheckPath.FromString(strCheckPath);
		}

		return pathCheckPath;
	}
}
