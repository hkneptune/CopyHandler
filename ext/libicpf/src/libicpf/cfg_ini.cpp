#include "cfg_ini.h"
#include "exception.h"
#include <string>
#include <map>
#include <assert.h>
#include "str_help.h"

BEGIN_ICPF_NAMESPACE

/// Buffer size for reading xml data from a file
#define INI_BUFFER	65536

// definition of line ending - system dependent
#if defined(_WIN32) || defined(_WIN64)
#define ENDL _t("\r\n")
#else
#define ENDL _t("\n")
#endif

#ifdef _UNICODE
	#define TEOF WEOF
#else
	#define TEOF EOF
#endif

/// String storage (key(s)=>value(s))
typedef std::multimap<tstring_t, tstring_t> attr_storage;
/// Ini node storage
typedef std::map<tstring_t, attr_storage> ini_storage;

/** Xml find handle structure - used for searching.
*/
struct INIFINDHANDLE
{
	attr_storage::iterator it;			///< Iterator of currently retrieved string
	attr_storage::iterator itEnd;		///< Iterator of a last string matching the criteria
};

/// Macro for faster access to the xml storage
#define m_pMainNode ((ini_storage*)m_hMainNode)

/** Constructs the ini_cfg object.
*/
ini_cfg::ini_cfg() :
	m_hMainNode((ptr_t)new ini_storage)
{

}

/** Destructs the xml config object.
*/
ini_cfg::~ini_cfg()
{
	delete m_pMainNode;
}

/** Function reads the contents of the xml file, parses it using expat parser
*  and then creates xml nodes in memory that could be read using find functions.
*
* \param[in] pszPath - path to the file to be read
*/
void ini_cfg::read(const tchar_t* pszPath)
{
	// clear current contents
	clear();

	// read the data from file
	FILE* pFile=_tfopen(pszPath, _t("rb"));
	if(pFile == NULL)
		THROW(icpf::exception::format(_t("Cannot open the file ") TSTRFMT _t(" for reading."), pszPath), 0, errno, 0);

	// prepare buffer for data
	tchar_t* pszBuffer = new tchar_t[INI_BUFFER];
	tchar_t* pszLine = NULL;
	while((pszLine = _fgetts(pszBuffer, INI_BUFFER, pFile)) != NULL)
	{
		// parse line
		parse_line(pszBuffer);
	}
	
	// check if that was eof or error
	if(feof(pFile) == 0)
	{
		fclose(pFile);
		// error while reading file
		THROW(_T("Error while reading ini file."), 0, errno, 0);
	}

	// close the file
	fclose(pFile);
}

/** Saves the internal xml nodes to the specified xml file.
*
* \param[in] pszPath - path to the file the data should be written to
*
* \note Function overwrites the contents of a file
*/
void ini_cfg::save(const tchar_t* pszPath)
{
	FILE* pFile=_tfopen(pszPath, _t("wb"));
	if(pFile == NULL)
		THROW(icpf::exception::format(_t("Cannot open the file ") TSTRFMT _t(" for writing."), pszPath), 0, errno, 0);

	// put BOM into the file
/*
#if(defined(_WIN32) || defined(_WIN64))
	// utf-16le
	const uint_t uiBOM=0x0000feff;
	const uint_t uiCount=2;
#else
	// utf-8
	const uint_t uiBOM=0x00bfbbef;
	const uint_t uiCount=3;
#endif
*/

	try
	{
		// write bom, check if it succeeded
//		if(fwrite(&uiBOM, 1, uiCount, pFile) != uiCount)
//			THROW(_t("Cannot write the BOM to the file '") TSTRFMT _t("'"), 0, errno, 0);

		// and write
		tstring_t strLine;
		for(ini_storage::iterator iterSections = m_pMainNode->begin(); iterSections != m_pMainNode->end(); iterSections++)
		{
			strLine = _t("[") + (*iterSections).first + _t("]") + ENDL;
			if(_fputts(strLine.c_str(), pFile) == TEOF)
				THROW(_t("Cannot put section name"), 0, errno, 0);
			for(attr_storage::iterator iterAttribute = (*iterSections).second.begin(); iterAttribute != (*iterSections).second.end(); iterAttribute++)
			{
				strLine = (*iterAttribute).first + _t("=") + (*iterAttribute).second + ENDL;
				if(_fputts(strLine.c_str(), pFile) == TEOF)
					THROW(_t("Cannot put attribute"), 0, errno, 0);
			}

			if(_fputts(ENDL, pFile) == TEOF)
				THROW(_t("Cannot put end-of-line marker into the file"), 0, errno, 0);
		}
	}
	catch(...)
	{
		fclose(pFile);
		throw;
	}

	// close the file
	fclose(pFile);
}

/** Function starts a search operation. Given the name of the property
*  to be searched for(ie. "ch/program/startup"), funtion searches for
*  it and returns a handle that can be used by subsequent calls to the
*  find_next(). Free the handle using find_close() after finish.
*
* \param[in] pszName - name of the property to search for(in the form of
*						"ch/program/startup"
* \return Handle to the search (NULL if not found).
*/
ptr_t ini_cfg::find(const tchar_t* pszName)
{
	// parse the path
	tstring_t strSection;
	tstring_t strAttr;
	if(!parse_property_name(pszName, strSection, strAttr))
		return NULL;

	ini_storage::iterator iterSection = m_pMainNode->find(strSection);
	if(iterSection == m_pMainNode->end())
		return NULL;

	std::pair<attr_storage::iterator, attr_storage::iterator> pairRange;
	if(strAttr == _t("*"))
	{
		pairRange.first = (*iterSection).second.begin();
		pairRange.second = (*iterSection).second.end();
	}
	else
		pairRange = (*iterSection).second.equal_range(strAttr);
	if(pairRange.first != (*iterSection).second.end())
	{
		INIFINDHANDLE* pHandle = new INIFINDHANDLE;
		pHandle->it = pairRange.first;
		pHandle->itEnd = pairRange.second;

		return pHandle;
	}

	return NULL;
}

/** Finds the next string that belong to a specific key (as defined in
*  a call to find() function.
*
* \param[in] pFindHandle - handle to the search (as returned from find())
* \return Pointer to a next string found, NULL if none.
*/
const tchar_t* ini_cfg::find_next(ptr_t pFindHandle)
{
	INIFINDHANDLE* pfh=(INIFINDHANDLE*)pFindHandle;
	if(pfh->it != pfh->itEnd)
		return (*pfh->it++).second.c_str();
	else
		return NULL;
}

/** Closes the find handle.
*
* \param[in] pFindHandle - handle to the search (as returned from find())
*/
void ini_cfg::find_close(ptr_t pFindHandle)
{
	delete ((INIFINDHANDLE*)pFindHandle);
}

/** Sets the specified value in the given key name. Value can be either added to
*  the current ones (multi-string support) or replace them completely.
*
* \param[in] pszName - key name for which the string should be set at
* \param[in] pszValue - value to set
* \param[in] a - action to take while setting
*/
void ini_cfg::set_value(const tchar_t* pszName, const tchar_t* pszValue, actions a)
{
	// parse the path
	tstring_t strSection;
	tstring_t strAttr;
	if(!parse_property_name(pszName, strSection, strAttr))
		THROW(_t("Property not found"), 0, 0, 0);

	if(strAttr == _t("*"))
		THROW(_t("Wildcards not available in set_value mode"), 0, 0, 0);

	// search
	ini_storage::iterator iterSection = m_pMainNode->find(strSection.c_str());
	if(iterSection == m_pMainNode->end())
	{
		std::pair<ini_storage::iterator, bool> pairSection = m_pMainNode->insert(ini_storage::value_type(strSection, attr_storage()));
		iterSection = pairSection.first;
		if(iterSection == m_pMainNode->end())
			THROW(_t("Problem with creating section"), 0, 0, 0);
	}

	attr_storage& rAttrs = (*iterSection).second;

	// clear if we're replacing
	switch(a)
	{
	case config_base::action_replace:
		{
			std::pair<attr_storage::iterator, attr_storage::iterator> pairRange = (*iterSection).second.equal_range(strAttr);
			rAttrs.erase(pairRange.first, pairRange.second);
			// do not break here - we are about to insert the data
		}
	case config_base::action_add:
		{
			rAttrs.insert(attr_storage::value_type(strAttr, pszValue));
			break;
		}
	default:
		assert(false);
	}
}

/** Clears the contents of this class
*
* \param[in] pszName - name of the property to clear the values for
*/
void ini_cfg::clear()
{
	m_pMainNode->clear();
}

/** Recursive clear function - searches recursively for a proper node
*  and finally clears the string map.
*
* \param[in] pNodePtr - pointer to a node to be processed
* \param[in] pszName - name of the property to search for in the given node
*/
void ini_cfg::clear(const tchar_t* pszName)
{
	if(pszName == NULL || pszName[0] == _t('*'))
		m_pMainNode->clear();
	else
	{
		tstring_t strSection;
		tstring_t strAttr;
		if(!parse_property_name(pszName, strSection, strAttr))
			THROW(_t("Invalid name"), 0, 0, 0);

		ini_storage::iterator iterSection = m_pMainNode->find(strSection);
		if(iterSection != m_pMainNode->end())
		{
			attr_storage& rAttrs = (*iterSection).second;
			std::pair<attr_storage::iterator, attr_storage::iterator> pairRange;

			if(strAttr == _t("*"))
			{
				pairRange.first = rAttrs.begin();
				pairRange.second = rAttrs.end();
			}
			else
				pairRange = (*iterSection).second.equal_range(strAttr);
			rAttrs.erase(pairRange.first, pairRange.second);
		}
	}
}

void ini_cfg::parse_line(const tchar_t* pszLine)
{
	assert(pszLine);
	if(!pszLine)
		THROW(_t("Invalid parameter"), 0, 0, 0);

	tstring_t strLine = pszLine;

	// trim whitespaces
	while(strLine.begin() != strLine.end() && string_tool::is_whitespace(*strLine.begin()))
	{
		strLine.erase(strLine.begin());
	}
	while(strLine.rbegin() != strLine.rend() && string_tool::is_whitespace(*strLine.rbegin()))
	{
		strLine.erase(strLine.end() - 1);
	}

	// detect line type
	if(strLine.begin() == strLine.end())
		return;
	if(strLine[0] == _t('['))
	{
		// trim [ and ]
		strLine.erase(strLine.begin());
		if(strLine.empty() || (*strLine.rbegin()) != _t(']'))
			THROW(_t("Wrong section name"), 0, 0, 0);
		strLine.erase(strLine.end() - 1);

		// a new section
		m_strCurrentSection = strLine;
		m_pMainNode->insert(ini_storage::value_type(strLine, attr_storage()));
	}
	else
	{
		// key=value
		tstring_t::size_type stPos = strLine.find_first_of(_t('='));
		if(stPos != tstring_t::npos)
		{
			ini_storage::iterator iterSection = m_pMainNode->find(m_strCurrentSection);
			if(iterSection == m_pMainNode->end())
				THROW(_t("Internal processing error. Section should already be included."), 0, 0, 0);
			tstring_t strLeft, strRight;
			strLeft.insert(strLeft.begin(), strLine.begin(), strLine.begin() + stPos);
			strRight.insert(strRight.begin(), strLine.begin() + stPos + 1, strLine.end());
			(*iterSection).second.insert(attr_storage::value_type(strLeft, strRight));
		}
	}
}

bool ini_cfg::parse_property_name(const tchar_t* pszName, tstring_t& rstrSection, tstring_t& rstrName)
{
	// parse the path
	tstring_t strPath = pszName;
	tstring_t::size_type stPos = strPath.find_first_of(_t('/'));
	if(stPos == tstring_t::npos)
		return false;
	tstring_t::size_type stPos2 = strPath.find_first_of(_t('/'), stPos + 1);
	if(stPos2 != tstring_t::npos && stPos2 != stPos)
		return false;											// paths with two or more '/' are not supported

	rstrName.clear();
	rstrName.clear();
	rstrSection.insert(rstrSection.begin(), strPath.begin(), strPath.begin() + stPos);
	rstrName.insert(rstrName.begin(), strPath.begin() + stPos + 1, strPath.end());

	return true;
}

END_ICPF_NAMESPACE
