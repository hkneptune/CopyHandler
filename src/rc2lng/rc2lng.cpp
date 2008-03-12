// CHExe2Lng.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "rc2lng.h"
#include "conio.h"

#pragma warning(disable : 4786)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CWinApp theApp;

int GetCommasCount(const CString& str)
{
	int cnt=0;
	bool bInside=false;
	for (int i=0;i<str.GetLength();i++)
	{
		if (str[i] == '\"')
			bInside=!bInside;

		if (!bInside && str[i] == ',')
			cnt++;
	}

	return cnt;
}

bool ReadResourceIDs(PCTSTR pszFile, map<CString, UINT>* pIDs)
{
	try
	{
		CFile file(pszFile, CFile::modeRead);
		CArchive ar(&file, CArchive::load);
		CString str, str2;
		while(ar.ReadString(str))
		{
			if (str.Left(7) == _T("#define"))
			{
				str=str.Mid(8);
				int iPos=str.FindOneOf(" \t");
				str2=str.Left(iPos);
				str=str.Mid(iPos);
				str.TrimLeft(_T(" \t"));
				str.TrimRight(_T(" \t\r\n"));

				int iID;
				if (str.Find("x") != -1)
				{
					// hex2dec
					_stscanf(str, "%lx", &iID);
				}
				else
					iID=_ttoi(str);
				
				pIDs->insert(map<CString, UINT>::value_type(str2, iID));
			}
		}
		
		ar.Close();
		file.Close();

		pIDs->insert(map<CString, UINT>::value_type(CString("IDOK"), 1));
		pIDs->insert(map<CString, UINT>::value_type(CString("IDCANCEL"), 2));
	}
	catch(...)
	{
		return false;
	}

/*	map<CString, UINT>::iterator it=pIDs->begin();
	while (it != pIDs->end())
	{
		cout<<(PCTSTR)(CString)(it->first)<<" = "<<(UINT)(it->second)<<endl;
		it++;
	}*/

	return true;
}

bool UpdateLngHeader(PCTSTR pszFile, vector<CString>* pvData)
{
	try
	{
		CFile file(pszFile, CFile::modeRead);
		CArchive ar(&file, CArchive::load);
		
		CString str;
		while (ar.ReadString(str))
			pvData->push_back(str);
		
		ar.Close();
		file.Close();
	}
	catch(...)
	{
		return false;
	}
	return true;
}

bool ReadRCFile(PCTSTR pszFile, vector<CString> *pv)
{
	try
	{
		CFile file(pszFile, CFile::modeRead);
		CArchive ar(&file, CArchive::load);
		
		CString str;
		while (ar.ReadString(str))
			pv->push_back(str);
		
		ar.Close();
		file.Close();
	}
	catch(...)
	{
		return false;
	}
	return true;
}

void ProcessMenu(vector<CString>* pinrc, vector<CString>::iterator *init, map<CString, UINT>* pids, vector<CString>* poutrc, vector<CString>* pol)
{
	CString str;
	map<CString, UINT>::iterator mit;
	for (;(*init) != pinrc->end();(*init)++)
	{
		str=**init;
		str.TrimLeft(" ");
		str.TrimRight(" ");

		// check for exit
		if ( str == "END" )
		{
			// add the line to the outrc wo changes
			poutrc->push_back(**init);
			return;
		}
		else if (str.Left(5) == "POPUP") // if that is the popup string - call the function once more
		{
			// add the line to the outrc with changes - replace string inside "" with P
			str=**init;

			// processing menuitem - find the text
			int iPos=str.Find("\"", 0);
			CString strText;
			if (iPos != -1)
			{
				strText=str.Mid(iPos+1);
				int iPos2=strText.Find("\"");
				if (iPos2 != -1)
					strText=strText.Left(iPos2);
			}

			// now find the | that separates the text from the pseudo-ID
			int iBar=strText.ReverseFind(_T('|'));
			if (iBar != -1)
			{
				// there is a text with an ID
				CString strID=strText.Mid(iBar+1);
				strText=strText.Left(iBar);

				// put the id and text in the translation file
				// find the equiv for the id
				mit=pids->find(strID);
				CString out;
				if (mit != pids->end())
				{
					out.Format("%lu=%s", mit->second, strText);
					pol->push_back(out);

					// put the found ID as output text
					out.Format("\"%lu\"", mit->second);
					str=str.Left(iPos)+out;
				}
				else
				{
					out.Format("%s=%s", strID, strText);
					pol->push_back(out);

					// put the ID as output text
					str=str.Left(iPos)+"\""+strID+"\"";
				}
			}
			else
			{
				// no ID
				str=str.Left(iPos)+"\"P\"";
			}

			poutrc->push_back(str);

			(*init)++;
			ProcessMenu(pinrc, init, pids, poutrc, pol);
		}
		else
		{
			// if the line has MENUITEM
			if (str.Left(8) == "MENUITEM" && str.Right(9) != "SEPARATOR")
			{
				// restore original
				str=**init;

				// check if there is any text after the comma
				int iPos=str.Find(",", 0);
				CString strTest=str.Mid(iPos);
				strTest.TrimLeft(" ,\t\r\n");
				if (strTest.IsEmpty())
				{
					(*init)++;

					CString tmp=**init;
					tmp.Trim(" ,\t\r\n");
					str+=tmp;
				}

				// processing menuitem - find the text
				iPos=str.Find("\"", 0);
				CString strText;
				if (iPos != -1)
				{
					strText=str.Mid(iPos+1);
					int iPos2=strText.Find("\"");
					if (iPos2 != -1)
						strText=strText.Left(iPos2);
				}

				// find the ID
				iPos=str.Find(",", 0);
				CString strID;
				if (iPos != -1)
				{
					strID=str.Mid(iPos+1);
					int iPos2=strID.Find(",", 0);
					if (iPos2 != -1)
						strID=strID.Left(iPos2);
				}
				strID.TrimLeft(" \t");
				strID.TrimRight(" \t");

				// find the equiv for the id
				mit=pids->find(strID);
				CString out;
				if (mit != pids->end())
				{
					out.Format("%lu=%s", mit->second, strText);
					pol->push_back(out);
				}
				else
				{
					out.Format("%s=%s", strID, strText);
					pol->push_back(out);
				}
//				AfxMessageBox(str);
				out=str;
//				out=**init;
				out.Replace("\""+strText+"\"", "\"i\"");
				poutrc->push_back(out);
			}
			else
				poutrc->push_back(**init);
		}
	}
}

void ProcessDialog(vector<CString>* pinrc, vector<CString>::iterator *init, map<CString, UINT>* pids, vector<CString>* poutrc, vector<CString>* pol)
{
	CString str;
	map<CString, UINT>::iterator mit;
	for (;(*init) != pinrc->end();(*init)++)
	{
		str=**init;
		str.TrimLeft(" ");
		str.TrimRight(" ");

		// check for exit
		if ( str == "END" )
		{
			// add the line to the outrc wo changes
			poutrc->push_back(**init);
			return;
		}
		else if ( str.Left(7) == "CAPTION" )
		{
			// read the caption
			CString strText=str.Mid(7);
			strText.TrimLeft(" \t\"");
			strText.TrimRight(" \t\"");

			pol->push_back("0="+strText);

			// save to rc wo title
			str=**init;
			str.Replace("\""+strText+"\"", "\"\"");
			poutrc->push_back(str);
		}
		else if ( str.Left(5) == "LTEXT" || str.Left(5) == "CTEXT" || str.Left(5) == "RTEXT" || str.Left(13) == "DEFPUSHBUTTON" || str.Left(10) == "PUSHBUTTON" || str.Left(7) == "CONTROL" || str.Left(8) == "GROUPBOX" )
		{
			// needed only 2 commas (outside the '\"')
			if ( GetCommasCount(str) < 2 )
				str+=*((*init)+1);

			// the first thing after LTEXT(and other) is the caption
			CString strText;
			
			if (str.Left(5) == "LTEXT" || str.Left(5) == "CTEXT" || str.Left(5) == "RTEXT")
				strText=str.Mid(5);
			else if (str.Left(13) == "DEFPUSHBUTTON")
				strText=str.Mid(13);
			else if (str.Left(10) == "PUSHBUTTON")
				strText=str.Mid(10);
			else if (str.Left(7) == "CONTROL")
				strText=str.Mid(7);
			else if (str.Left(8) == "GROUPBOX")
				strText=str.Mid(8);

			strText=strText.Mid(strText.Find("\"")+1);
			int iPos=strText.Find("\"", 0);
			if (iPos != -1)
				strText=strText.Left(iPos);

			// after the first comma there is an ID
			iPos=str.Find(",", 0);
			CString strID;
			if (iPos != -1)
			{
				strID=str.Mid(iPos+1);
				iPos=strID.Find(",", 0);
				if (iPos != -1)
					strID=strID.Left(iPos);
				strID.TrimLeft(" \t");
				strID.TrimRight(" \t");
			}

			// find id
			mit=pids->find( strID );
			CString out;
			if (mit != pids->end())
			{
				// id found
				if (mit->second != 0)
				{
					out.Format("%lu=%s", mit->second, strText);
					pol->push_back(out);
				}
			}
			else
			{
				out.Format("%s=%s", strID, strText);
				pol->push_back(out);
			}

			// now add the data to rc
			str=**init;
			str.Replace("\""+strText+"\"", "\"\"");

			poutrc->push_back(str);
		}
		else
		{
			poutrc->push_back(**init);
		}
	}
}

void ProcessStringTable(vector<CString>* pinrc, vector<CString>::iterator *init, map<CString, UINT>* pids, vector<CString>* poutrc, vector<CString>* ptab)
{
	map<CString, UINT>::iterator mit;
	CString str;
	for (;(*init) != pinrc->end();(*init)++)
	{
		str=**init;
		str.TrimLeft(" ");
		str.TrimRight(" ");

		if ( str == "END" )
			return;
		else if ( str != "BEGIN" )
		{
			// the first stuff is ID, the second is text
			int iPos=str.Find("\"", 0);
			if (iPos == -1)
			{
				(*init)++;
				str+=**init;
				iPos=str.Find("\"", 0);
			}

			if (iPos != -1)
			{
				CString strID=str.Left(iPos);
				strID.TrimRight(" \"\t\n\r");

				CString strText=str.Mid(iPos+1);
				strText.Replace("\"\"", "\"");

				strText=strText.Left(strText.ReverseFind('\"'));

				mit=pids->find(strID);
				CString out;
				if (mit!= pids->end())
					out.Format("%lu=%s", mit->second, strText);
				else
					out.Format("%lu=%s", strID, strText);
				ptab->push_back(out);
				str=**init;
				str.Replace("\""+strText+"\"", "\"\"");
			}
		}
	}
}
CString ProcessLine(PCTSTR psz)
{
	CString str=psz;
	str.Replace("\r", "\\r");
	str.Replace("\n", "\\n");
	str.Replace("\t", "\\t");

	return str;
}

bool ProcessRCFile(PCTSTR pszRCPath, vector<CString>* pinrc, map<CString, UINT>* pids, vector<CString>* poutrc, vector<CString>* pol)
{
	int iPos;
	map<CString, UINT>::iterator mit;
	CString strData;
	vector<CString> vStrTable;
	for (vector<CString>::iterator it=pinrc->begin();it != pinrc->end();it++)
	{
		if ( (iPos=it->Find(" MENU ")) != -1 )
		{
			// retrieve the identifier and add it to the outlng
			mit=pids->find( it->Left(iPos) );
			if (mit != pids->end())
				strData.Format("\r\n# Menu - %s\r\n[%lu]", it->Left(iPos), mit->second);
			else
				strData.Format("\r\n# Menu - %s\r\n[%s]", it->Left(iPos), it->Left(iPos));

			pol->push_back(strData);

			// add the line to the output rc with no change
			poutrc->push_back(*it);

			// begin enumerating items
			it++;
			ProcessMenu(pinrc, &it, pids, poutrc, pol);
		}
		else if ( (iPos=it->Find(" DIALOGEX ")) != -1)
		{
			// find the ID of a dialog box
			mit=pids->find( it->Left(iPos) );
			if (mit != pids->end())
				strData.Format("\r\n# Dialog box - %s\r\n[%lu]", it->Left(iPos), mit->second);
			else
				strData.Format("\r\n# Dialog box - %s\r\n[%s]", it->Left(iPos), it->Left(iPos));

			pol->push_back(strData);

			// add the line to the output rc with no change
			poutrc->push_back(*it);

			// begin processing dialog template
			it++;
			ProcessDialog(pinrc, &it, pids, poutrc, pol);
		}
		else if ( (iPos=it->Find("STRINGTABLE ")) != -1)
		{
			// begin of the string table
			it++;
			ProcessStringTable(pinrc, &it, pids, poutrc, &vStrTable);
		}
		else if ( (iPos=it->Find(" 25 ")) != -1)
		{	//pszRCPath
			CString strID=it->Left(iPos);
			strID.TrimLeft(" \t\n\r\"");
			strID.TrimRight(" \t\n\r\"");

			// file name
			iPos=it->Find("\"");
			CString strName=it->Mid(iPos+1);
			strName.TrimRight(" \t\n\r\"");
			strName.Replace("\\\\", "\\");

			// concat the path
			CString strPath=pszRCPath;
			strPath=strPath.Left(strPath.ReverseFind('\\')+1);

			// read the file
			TCHAR szData[16384];
			CFile file(strPath+strName, CFile::modeRead);
			file.Read(szData, 16384);
			file.Close();
			CString out=ProcessLine(szData);
			
			// add to lang
			mit=pids->find(strID);
			CString strLng;
			if (mit!=pids->end())
				strLng.Format("%lu=%s", mit->second, out);
			else
				strLng.Format("%s=%s", strID, out);
			vStrTable.push_back(strLng);
		}
		else
		{
			poutrc->push_back(*it);
		}
	}

	if (vStrTable.size() > 0)
	{
		// write header
		pol->push_back(CString("\r\n# String table\r\n[0]"));

		// copy data to the out lng
		for (vector<CString>::iterator it=vStrTable.begin();it!=vStrTable.end();it++)
			pol->push_back(*it);
	}

	return true;
}

bool WriteFiles(vector<CString>* prc, PCTSTR pszRCFile, vector<CString>* plng, PCTSTR pszLngFile)
{
	vector<CString>::iterator it;
	try
	{
		CFile file1(pszRCFile, CFile::modeWrite | CFile::modeCreate);
		CArchive ar1(&file1, CArchive::store);

		for (it=prc->begin();it != prc->end();it++)
			ar1.WriteString((*it)+_T("\r\n"));

		ar1.Close();
		file1.Close();
		
		CFile file2(pszLngFile, CFile::modeWrite | CFile::modeCreate);
		CArchive ar2(&file2, CArchive::store);
		
		for (it=plng->begin();it != plng->end();it++)
			ar2.WriteString((*it)+_T("\r\n"));

		ar2.Close();
		file2.Close();
	}
	catch(...)
	{
		return false;
	}
	return true;
}

struct CmpStr : public binary_function <CString, CString, bool>
{
      bool operator() (const CString& _Left, const CString& _Right) const
	  {
		  CString l=_Left.Left(_Left.Find("="));
		  CString r=_Right.Left(_Right.Find("="));

		  return (_ttoi(l) <= _ttoi(r));
	  }
};

void SortItems(vector<CString>* pv, int iHdrCnt)
{
	// find the begin and the end of section
	vector<CString>::iterator st=pv->begin()+iHdrCnt, en;
	while (st != pv->end())
	{
		for (;st != pv->end();st++)
		{
			// find section beginning
			if (st->Left(3) == "\r\n#" && st->Find("\r\n[") != -1)
			{
				st++;
				break;
			}
		}
		if (st != pv->end())
		{
			en=st+1;
			for (;en != pv->end();en++)
			{
				if (en->Left(3) == "\r\n#" && en->Find("\r\n[") != -1)
					break;
			}
				
			sort(st, en, CmpStr());
			st=en;
		}
	}
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		return 1;
	}

	// usage - chexe2lng infile.rc resource.h inheader.lng outfile.rc outfile.lng
	if (argc < 6)
	{
		cerr<<_T("Fatal Error: Incorrect numer of params") << endl;
		cerr << _T("Usage: infile.rc inheader.lng outfile.rc outfile.lng resource.h resource2.h") << endl;
		return 2;
	}

	// open the resource.h file and interprete it
	map<CString, UINT> mID;
	for (int i=5;i<argc;i++)
	{
		if (!ReadResourceIDs(argv[i], &mID))
		{
			cerr<<"Fatal Error: Cannot read file with resource ID's"<<endl;
			return 3;
		}
	}

	// for out .lng - data
	vector<CString> vOutLang;
	if (!UpdateLngHeader(argv[2], &vOutLang))
	{
		cerr<<"Fatal Error: Cannot read lang header file"<<endl;
		return 4;
	}
	int iHeaderCount=vOutLang.size();

	// out rc file data
	vector<CString> vRCIn, vRCOut;
	if (!ReadRCFile(argv[1], &vRCIn))
	{
		cerr<<"Fatal Error: Cannot read source RC file"<<endl;
		return 5;
	}

	if (!ProcessRCFile(argv[1], &vRCIn, &mID, &vRCOut, &vOutLang))
	{
		cerr<<"Fatal Error: Cannot process RC file"<<endl;
		return 6;
	}

	SortItems(&vOutLang, iHeaderCount);

	if (!WriteFiles(&vRCOut, argv[3], &vOutLang, argv[4]))
	{
		cerr<<"Fatal Error: Cannot write output files"<<endl;
		return 7;
	}

	return 0;
}


