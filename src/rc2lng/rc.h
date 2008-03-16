#ifndef __RC_H__
#define __RC_H__

class CRCFile
{
private:
	typedef std::map<UINT, CString> element_map;
	typedef std::map<UINT, element_map> group_map;

	enum ELineType
	{
		eLine_Group,
		eLine_Translation
	};
public:
	CRCFile();
	~CRCFile();

	void ReadRC(PCTSTR pszFilename);
	void ReadResourceIDs(PCTSTR pszFilename);

	void WriteRC(PCTSTR pszFilename);
	void WriteLang(PCTSTR pszFilename, PCTSTR pszHeaderFile);

	void Clear();

protected:
	// Adds initial values to the maps
	void AddInitialValues();

	static int GetCommasCount(const CString& str);

	static void WriteFile(PCTSTR pszFile, const std::vector<CString>& rLines, bool bUnicode);
	static void ReadFile(PCTSTR pszFile, std::vector<CString>& rLines, bool bUnicode);

	void AddTranslationLine(ELineType eLineType, UINT uiID, PCTSTR pszText, std::vector<CString>& vLines);

	bool ProcessRCFile();
	void ProcessMenu(UINT uiMenuID, std::vector<CString>::iterator *init);
	void ProcessDialog(UINT uiDialogID, std::vector<CString>::iterator *init);
	void ProcessStringTable(UINT uiStringGroupID, std::vector<CString>::iterator *init);

	element_map* GetElementMap(UINT uiGroup);
	UINT GetResourceID(PCTSTR pszID);

protected:
	std::map<CString, UINT> m_mapNameToID;		// resource name to resource id
	std::vector<CString> m_vInRCFile;			// lines from rc file
	std::vector<CString> m_vOutRCFile;			// lines to write to rc file

	group_map m_mapOutputLanguage;	// section_id->element_id->string
//	std::vector<CString> m_vOutputLanguage;		// lines of the language file to be written
	TCHAR* m_pszBuffer;
};

#endif
