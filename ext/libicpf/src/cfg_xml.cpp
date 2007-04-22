#include "cfg_xml.h"
#include <expat.h>
#include "exception.h"
#include <string>
#include <map>
#include <assert.h>

BEGIN_ICPF_NAMESPACE

/// Buffer size for reading xml data from a file
#define XML_BUFFER	65536

/// Definition of a standard string depending on the unicode support
#ifdef _UNICODE
	#define tstring std::wstring
#else
	#define tstring std::string
#endif

// forward declaration
class xml_node;

/// Xml node storage
typedef std::map<tstring, xml_node> xml_storage;
/// String storage (key(s)=>value(s))
typedef std::multimap<tstring, tstring> attr_storage;

/** Class manages a single xml node.
 */
class xml_node
{
public:
/** \name Construction/destruction */
/**@{*/
	/// Standard constructor
	xml_node() : m_mNodes(), m_mAttr(), m_pParentNode(NULL) { };
	/// Constructor defining the parent node
	xml_node(xml_node* pParentNode)  : m_mNodes(), m_mAttr(), m_pParentNode(pParentNode) { };
/**@}*/

public:
	xml_storage m_mNodes;		///< Additional nodes inside of this one
	attr_storage m_mAttr;		///< String pairs belonging to this node
	xml_node* m_pParentNode;	///< Parent node
};

/** State structure - used by expat notifications.
 */
struct XMLSTATE
{
	xml_cfg* pCfg;
	xml_node* pNode;
};

/** Xml find handle structure - used for searching.
 */
struct XMLFINDHANDLE
{
	attr_storage::iterator it;			///< Iterator of currently retrieved string
	attr_storage::iterator itEnd;		///< Iterator of a last string matching the criteria
};

/// Macro for faster access to the xml storage
#define m_pStorage ((xml_storage*)m_hStorage)

/** Constructs the xml_cfg object.
 */
xml_cfg::xml_cfg() :
	m_hStorage((ptr_t)new xml_storage)
{
	
}

/** Destructs the xml config object.
 */
xml_cfg::~xml_cfg()
{
	delete m_pStorage;
}

/** Expat start element handler.
 *
 * \param[in] userData - pointer to user defined parameters
 * \param[in] name - name of the tag being processed
 * \param[in] attrs - array of pointers to strings with attributes and their values
 */
void XMLCALL element_start(void *userData, const tchar_t *name, const tchar_t **attrs)
{
	XMLSTATE* pState=(XMLSTATE*)userData;

	bool bContainer=true;

	for (size_t t=0;attrs[t] != NULL;t+=2)
	{
		if (_tcscmp(attrs[t], _t("value")) == 0)
		{
			// this is the value type tag
			pState->pNode->m_mAttr.insert(attr_storage::value_type(tstring(name), tstring(attrs[t+1])));
			bContainer=false;
		}
	}

	if (bContainer)
	{
		std::pair<xml_storage::iterator, bool> pr=pState->pNode->m_mNodes.insert(xml_storage::value_type(tstring(name), xml_node(pState->pNode)));
		pState->pNode=&((*pr.first).second);
	}
}

/** Expat handler for closing tag.
 *
 * \param[in] userData - user defined parameter
 * \param[in] name - name of the tag being closed
 */
void XMLCALL element_end(void *userData, const XML_Char* /*name*/)
{
	XMLSTATE* pState=(XMLSTATE*)userData;

	// go up one level
	if (pState->pNode)
		pState->pNode=pState->pNode->m_pParentNode;
	else
		THROW(_t("Trying to close non-existent tag."), 0, 0, 0);
}

/*void XMLCALL element_content(void *userData, const XML_Char *s, int len)
{
	XMLSTATE* pState=(XMLSTATE*)userData;

}*/

/** Function reads the contents of the xml file, parses it using expat parser
 *  and then creates xml nodes in memory that could be read using find functions.
 *
 * \param[in] pszPath - path to the file to be read
 */
void xml_cfg::read(const tchar_t* pszPath)
{
	// read the data from file in 64kB portions and feed it to the expat xml parser
	FILE* pFile=_tfopen(pszPath, _t("r"));
	if (pFile == NULL)
		THROW(icpf::exception::format(_t("Cannot open the file ") STRFMT _t("."), pszPath), 0, errno, 0);

	// create the parser
	XML_Parser parser=XML_ParserCreate(NULL);
	XML_SetElementHandler(parser, element_start, element_end);
//	XML_SetCharacterDataHandler(parser, element_content);

	XMLSTATE xs = { this };
	XML_SetUserData(parser, &xs);

	for (;;)
	{
		bool bLast=false;

		// get xml buffer
		void* pBuffer=XML_GetBuffer(parser, XML_BUFFER);

		// read some data to it
		size_t tSize=fread(pBuffer, 1, XML_BUFFER, pFile);
		if (tSize < XML_BUFFER)
		{
			// check for errors
			int iErr=0;
			if ( (iErr=ferror(pFile)) != 0)
				THROW(icpf::exception::format(_t("Error reading from the file ") STRFMT _t("."), pszPath), 0, iErr, 0);
			else
				bLast=true;
		}

		// parse
		if (!XML_ParseBuffer(parser, (int)tSize, bLast))
		{
			// parser error
			THROW(icpf::exception::format(_t("Error encountered while parsing the xml file ") STRFMT _t(" - ") STRFMT _t("."), pszPath, XML_ErrorString(XML_GetErrorCode(parser))), 0, 0, 0);
		}

		// end of processing ?
		if (bLast)
			break;
	}

	// free parser
	XML_ParserFree(parser);

	// close the file
	fclose(pFile);
}

/** Saves the internal xml nodes to the specified xml file.
 *
 * \param[in] pszPath - path to the file the data should be written to
 *
 * \note Function overwrites the contents of a file
 */
void xml_cfg::save(const tchar_t* /*pszPath*/)
{

}

/** Function starts a search operation. Given the name of the property
 *  to be searched for (ie. "ch/program/startup"), funtion searches for
 *  it and returns a handle that can be used by subsequent calls to the
 *  find_next(). Free the handle using find_close() after finish.
 *
 * \param[in] pszName - name of the property to search for (in the form of
 *						"ch/program/startup" for xml such as this:
 *
 *						<ch>
 *							<program>
 *								<startup value="1"/>
 *							</program>
 *						</ch>
 * \return Handle to the search (NULL if not found).
 */
ptr_t xml_cfg::find(const tchar_t* pszName)
{
	return find(m_pStorage, pszName);
}

/** A find() helper function - recursively searches a specific node
 *  for a given name.
 *
 * \param[in] pNodePtr - pointer to a node to search in
 * \param[in] pszName - name of the property to search for
 * \return Handle to the node or NULL if none.
 */
ptr_t xml_cfg::find(ptr_t pNodePtr, const tchar_t* pszName)
{
	xml_node* pNode=(xml_node*)pNodePtr;

	// parse the name
	const tchar_t* pSign=_tcschr(pszName, _t('/'));
	if (pSign)
	{
		// locate the xml_node associated with the name
		xml_storage::iterator it=pNode->m_mNodes.find(tstring(pszName, pSign-pszName));
		if (it != pNode->m_mNodes.end())
			return find(&(*it).second, pSign+1);
		else
			return NULL;
	}
	else
	{
		XMLFINDHANDLE* pfh=new XMLFINDHANDLE;
		std::pair<attr_storage::iterator, attr_storage::iterator> pr=pNode->m_mAttr.equal_range(pszName);
		pfh->it=pr.first;
		pfh->itEnd=pr.second;

		return pfh;
	}
}

/** Finds the next string that belong to a specific key (as defined in
 *  a call to find() function.
 *
 * \param[in] pFindHandle - handle to the search (as returned from find())
 * \return Pointer to a next string found, NULL if none.
 */
const tchar_t* xml_cfg::find_next(ptr_t pFindHandle)
{
	XMLFINDHANDLE* pfh=(XMLFINDHANDLE*)pFindHandle;
	if (pfh->it != pfh->itEnd)
		return (*pfh->it++).second.c_str();
	else
		return NULL;
}

/** Closes the find handle.
 *
 * \param[in] pFindHandle - handle to the search (as returned from find())
 */
void xml_cfg::find_close(ptr_t pFindHandle)
{
	delete ((XMLFINDHANDLE*)pFindHandle);
}

/** Sets the specified value in the given key name. Value can be either added to
 *  the current ones (multi-string support) or replace them completely.
 *
 * \param[in] pszName - key name for which the string should be set at
 * \param[in] pszValue - value to set
 * \param[in] a - action to take while setting
 */
void xml_cfg::set_value(const tchar_t* pszName, const tchar_t* pszValue, actions a)
{
	// traverse the current tag tree
	set_value(m_pStorage, pszName, pszValue, a);
}

/** Sets the specified value in the given key name - recursive helper function.
 *
 * \param[in] pNodePtr - pointer to the xml node to process
 * \param[in] pszName - key name for which the string should be set at
 * \param[in] pszValue - value to set
 * \param[in] a - action to take while setting
 */
void xml_cfg::set_value(ptr_t pNodePtr, const tchar_t* pszName, const tchar_t* pszValue, actions a)
{
	xml_node* pNode=(xml_node*)pNodePtr;

	const tchar_t* pszSign=_tcschr(pszName, _t('/'));
	if (pszSign != NULL)
	{
		xml_storage::iterator it=pNode->m_mNodes.find(tstring(pszName, pszSign-pszName));
		if (it != pNode->m_mNodes.end())
			set_value(&(*it).second, pszSign+1, pszValue, a);
		else
		{
			std::pair<xml_storage::iterator, bool> pr=pNode->m_mNodes.insert(xml_storage::value_type(tstring(pszName, pszSign-pszName), xml_node(pNode)));
			set_value(&(*pr.first).second, pszSign+1, pszValue, a);
		}
	}
	else
	{
		// clear if we're replacing
		switch(a)
		{
		case config_base::action_replace:
			pNode->m_mAttr.clear();
		case config_base::action_add:
			pNode->m_mAttr.insert(attr_storage::value_type(tstring(pszName), tstring(pszValue)));
			break;
		default:
			assert(false);
		}
	}
}

/** Clear values for a given property name.
 *
 * \param[in] pszName - name of the property to clear the values for
 */
void xml_cfg::clear(const tchar_t* pszName)
{
	clear(m_pStorage, pszName);
}

/** Recursive clear function - searches recursively for a proper node
 *  and finally clears the string map.
 *
 * \param[in] pNodePtr - pointer to a node to be processed
 * \param[in] pszName - name of the property to search for in the given node
 */
void xml_cfg::clear(ptr_t pNodePtr, const tchar_t* pszName)
{
	xml_node* pNode=(xml_node*)pNodePtr;

	// parse the name
	const tchar_t* pSign=_tcschr(pszName, _t('/'));
	if (pSign)
	{
		// locate the xml_node associated with the name
		xml_storage::iterator it=pNode->m_mNodes.find(tstring(pszName, pSign-pszName));
		if (it != pNode->m_mNodes.end())
			clear(&(*it).second, pSign+1);
	}
	else
	{
		std::pair<attr_storage::iterator, attr_storage::iterator> pr=pNode->m_mAttr.equal_range(tstring(pszName));
		pNode->m_mAttr.erase(pr.first, pr.second);
	}
}

END_ICPF_NAMESPACE
