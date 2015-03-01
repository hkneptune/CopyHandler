// ============================================================================
//  Copyright (C) 2001-2015 by Jozef Starosczyk
//  ixen@copyhandler.com
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU Library General Public License
//  (version 2) as published by the Free Software Foundation;
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this program; if not, write to the
//  Free Software Foundation, Inc.,
//  59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// ============================================================================
#include "stdafx.h"
#include "TFeedbackHandlerBase.h"
#include "SerializerDataTypes.h"
#include "ISerializerContainer.h"

BEGIN_CHCORE_NAMESPACE

TFeedbackHandlerBase::TFeedbackHandlerBase() :
	m_eFileError(m_setModifications, EFeedbackResult::eResult_Unknown),
	m_eFileAlreadyExists(m_setModifications, EFeedbackResult::eResult_Unknown),
	m_eNotEnoughSpace(m_setModifications, EFeedbackResult::eResult_Unknown),
	m_eOperationFinished(m_setModifications, EFeedbackResult::eResult_Unknown),
	m_eOperationError(m_setModifications, EFeedbackResult::eResult_Unknown)
{
	m_setModifications[eMod_Added] = true;
}

TFeedbackHandlerBase::~TFeedbackHandlerBase()
{
}

void TFeedbackHandlerBase::Store(const ISerializerContainerPtr& spContainer) const
{
	boost::shared_lock<boost::shared_mutex> lock(m_lock);

	if (m_setModifications.any())
	{
		InitColumns(spContainer);

		bool bAdded = m_setModifications[eMod_Added];
		ISerializerRowData& rRowData = spContainer->GetRow(0, bAdded);

		if (bAdded || m_eFileError.IsModified())
			rRowData.SetValue(_T("file_error"), m_eFileError);

		if (bAdded || m_eFileAlreadyExists.IsModified())
			rRowData.SetValue(_T("file_already_exists"), m_eFileAlreadyExists);

		if (bAdded || m_eNotEnoughSpace.IsModified())
			rRowData.SetValue(_T("not_enough_space"), m_eNotEnoughSpace);

		if (bAdded || m_eOperationFinished.IsModified())
			rRowData.SetValue(_T("operation_finished"), m_eOperationFinished);

		if (bAdded || m_eOperationError.IsModified())
			rRowData.SetValue(_T("operation_error"), m_eOperationError);

		m_setModifications.reset();
	}
}

void TFeedbackHandlerBase::InitColumns(const ISerializerContainerPtr& spContainer)
{
	IColumnsDefinition& rColumnDefs = spContainer->GetColumnsDefinition();
	if (rColumnDefs.IsEmpty())
	{
		rColumnDefs.AddColumn(_T("id"), ColumnType<object_id_t>::value);
		rColumnDefs.AddColumn(_T("file_error"), IColumnsDefinition::eType_int);
		rColumnDefs.AddColumn(_T("file_already_exists"), IColumnsDefinition::eType_int);
		rColumnDefs.AddColumn(_T("not_enough_space"), IColumnsDefinition::eType_int);
		rColumnDefs.AddColumn(_T("operation_finished"), IColumnsDefinition::eType_int);
		rColumnDefs.AddColumn(_T("operation_error"), IColumnsDefinition::eType_int);
	}
}

void TFeedbackHandlerBase::Load(const ISerializerContainerPtr& spContainer)
{
	boost::unique_lock<boost::shared_mutex> lock(m_lock);

	InitColumns(spContainer);
	ISerializerRowReaderPtr spRowReader = spContainer->GetRowReader();
	if (spRowReader->Next())
	{
		int iFeedbackResult = eResult_Unknown;

		spRowReader->GetValue(_T("file_error"), iFeedbackResult);
		m_eFileError = (EFeedbackResult) iFeedbackResult;

		spRowReader->GetValue(_T("file_already_exists"), iFeedbackResult);
		m_eFileAlreadyExists = (EFeedbackResult) iFeedbackResult;
		spRowReader->GetValue(_T("not_enough_space"), iFeedbackResult);
		m_eNotEnoughSpace = (EFeedbackResult) iFeedbackResult;
		spRowReader->GetValue(_T("operation_finished"), iFeedbackResult);
		m_eOperationFinished = (EFeedbackResult) iFeedbackResult;
		spRowReader->GetValue(_T("operation_error"), iFeedbackResult);
		m_eOperationError = (EFeedbackResult) iFeedbackResult;

		m_setModifications.reset();
	}
}

void TFeedbackHandlerBase::RestoreDefaults()
{
	m_eFileError = EFeedbackResult::eResult_Unknown;
	m_eFileAlreadyExists = EFeedbackResult::eResult_Unknown;
	m_eNotEnoughSpace = EFeedbackResult::eResult_Unknown;
	m_eOperationFinished = EFeedbackResult::eResult_Unknown;
	m_eOperationError = EFeedbackResult::eResult_Unknown;
}

chcore::EFeedbackResult TFeedbackHandlerBase::FileError(const TString& /*strSrcPath*/, const TString& /*strDstPath*/, EFileError /*eFileError*/, unsigned long /*ulError*/)
{
	return m_eFileError;
}

chcore::EFeedbackResult TFeedbackHandlerBase::FileAlreadyExists(const TFileInfoPtr& /*spSrcFileInfo*/, const TFileInfoPtr& /*spDstFileInfo*/)
{
	return m_eFileAlreadyExists;
}

chcore::EFeedbackResult TFeedbackHandlerBase::NotEnoughSpace(const TString& /*strSrcPath*/, const TString& /*strDstPath*/, unsigned long long /*ullRequiredSize*/)
{
	return m_eNotEnoughSpace;
}

chcore::EFeedbackResult TFeedbackHandlerBase::OperationFinished()
{
	return m_eOperationFinished;
}

chcore::EFeedbackResult TFeedbackHandlerBase::OperationError()
{
	return m_eOperationError;
}

END_CHCORE_NAMESPACE