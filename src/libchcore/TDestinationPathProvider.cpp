#include "stdafx.h"
#include "TDestinationPathProvider.h"
#include "TCoreException.h"
#include "ErrorCodes.h"
#include <boost/lexical_cast.hpp>

namespace chcore
{
	TDestinationPathProvider::TDestinationPathProvider(const IFilesystemPtr& spFilesystem,
		const TSmartPath& pathDestinationBase,
		bool bIgnoreFolders,
		bool bForceDirectories,
		const TString& strFirstAltName,
		const TString& strNextAltName) :

		m_spFilesystem(spFilesystem),
		m_pathDestinationBase(pathDestinationBase),
		m_bForceDirectories(bForceDirectories),
		m_bIgnoreFolders(bIgnoreFolders),
		m_strFirstAltName(strFirstAltName),
		m_strNextAltName(strNextAltName)
	{
		if(!spFilesystem)
			throw TCoreException(eErr_InvalidArgument, L"spFilesystem", LOCATION);
	}


	TSmartPath TDestinationPathProvider::CalculateDestinationPath(const TFileInfoPtr& spFileInfo)
	{
		if(!spFileInfo)
			throw TCoreException(eErr_InvalidArgument, L"spFileInfo", LOCATION);

		if(m_bForceDirectories)
		{
			// force create directories
			TSmartPath pathCombined = m_pathDestinationBase + spFileInfo->GetFullFilePath().GetFileDir();

			// force create directory
			m_spFilesystem->CreateDirectory(pathCombined, true);

			return pathCombined + spFileInfo->GetFullFilePath().GetFileName();
		}
		else
		{
			TBasePathDataPtr spPathData = spFileInfo->GetBasePathData();

			if(!m_bIgnoreFolders && spPathData)
			{
				// generate new dest name
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
						TSmartPath pathFilename = spFileInfo->GetFullFilePath().GetFileName();
						pathFilename.StripPath(L":");
						spPathData->SetDestinationPath(pathFilename);
					}
				}

				return m_pathDestinationBase + spPathData->GetDestinationPath() + spFileInfo->GetFilePath();
			}
			else
				return m_pathDestinationBase + spFileInfo->GetFilePath();
		}
	}

	// finds another name for a copy of src file(folder) in dest location
	TSmartPath TDestinationPathProvider::FindFreeSubstituteName(TSmartPath pathSrcPath) const
	{
		// get the name from src path
		pathSrcPath.StripSeparatorAtEnd();

		TSmartPath pathFilename = pathSrcPath.GetFileName();
		pathFilename.StripPath(L":");

		// set the dest path
		TString strCheckPath = m_strFirstAltName;
		strCheckPath.Replace(_T("%name"), pathFilename.ToString());
		TSmartPath pathCheckPath(PathFromWString(strCheckPath));

		// when adding to strDstPath check if the path already exists - if so - try again
		int iCounter = 1;
		TString strFmt = m_strNextAltName;
		while(m_spFilesystem->PathExist(m_pathDestinationBase + pathCheckPath))
		{
			strCheckPath = strFmt;
			strCheckPath.Replace(_T("%name"), pathFilename.ToString());
			strCheckPath.Replace(_T("%count"), boost::lexical_cast<std::wstring>(++iCounter).c_str());
			pathCheckPath.FromString(strCheckPath);
		}

		return pathCheckPath;
	}
}
