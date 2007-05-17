#include "config-test.h"
#include "cfg.h"
#include "cfg_xml.h"
#include "exception.h"
#include <vector>

void ConfigTest::Run()
{
	const ll_t llVal=-23524;
	const ull_t ullVal=3445;
	const bool bVal=true;
	const tchar_t* pszVal=_t("£¹ka œwieci jak mo¿e");

	// generate temporary file name
	tchar_t* pszName=_ttmpnam(NULL);
	if (!pszName)
		THROW(_t("Cannot generate the temporary file name"), 0, 0, 0);
	tstring_t strPath(_t("."));
	strPath+=pszName;
	strPath+=_t("cfg");

	// start with testing cfg class
	icpf::xml_cfg cfgXml;
	icpf::config cfg(&cfgXml);

	ReportS(_t("Registering properties...\n"));

	uint_t auiID[4];
	auiID[0]=cfg.register_signed_num(_t("test/TestSignedNum00"), 320, -23524, 640);
	auiID[1]=cfg.register_unsigned_num(_t("test/moo/TestUnsignedNum01"), 0, 0, 9999999);
	auiID[2]=cfg.register_bool(_t("test/TestBool02"), false);
	auiID[3]=cfg.register_string(_t("test/TestString03"), _t("none"));

	ReportS(_t("Setting values...\n"));

	cfg.set_signed_num(auiID[0], llVal);
	cfg.set_unsigned_num(auiID[1], ullVal);
	cfg.set_bool(auiID[2], bVal);
	cfg.set_string(auiID[3], pszVal);

	ReportS(_t("Retrieving and comparing values...\n"));
	if (cfg.get_signed_num(auiID[0]) != llVal)
		THROW(_t("Comparing signed number values failed"), 0, 0, 0);
	if (cfg.get_unsigned_num(auiID[1]) != ullVal)
		THROW(_t("Comparing unsigned number values failed"), 0, 0, 0);
	if (cfg.get_bool(auiID[2]) != bVal)
		THROW(_t("Comparing bool values failed"), 0, 0, 0);
	if (tstring_t(cfg.get_string(auiID[3])) != tstring_t(pszVal))
		THROW(_t("Comparing string values failed"), 0, 0, 0);

	// store values in the file
	Report(_t("Storing properties in the file '") TSTRFMT _t("'"), strPath.c_str());
	cfg.write(strPath.c_str());

}
