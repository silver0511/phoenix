// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：InitEx.h
// 创建人： 史云杰
// 文件说明:异常处理
// $_FILEHEADER_END ******************************
#ifndef __INIEX_H
#define __INIEX_H

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include "base0/platform.h"
using namespace std;

class CIniEx
{
	struct STRU_RECORD
	{
		string	mstrComments;
		char	mcCommented;
		string	mstrSection;
		string	mstrKey;
		string	mstrValue;
	};

	enum ENUM_COMMENTCHAR
	{
		Pound = '#',
		SemiColon = ';'
	};

public:
	CIniEx();
	~CIniEx();

private:

	struct STRU_RECORDSECTIONKEYIS : std::unary_function<STRU_RECORD, bool>
	{
		std::string mstrSection;
		std::string mstrKey;

		STRU_RECORDSECTIONKEYIS(const std::string& section, const std::string& key): mstrSection(section),mstrKey(key){}

		bool operator()( const STRU_RECORD& rec ) const
		{//以前的代码在linux下会抛出异常，此代码由YLP做了修改
			if( (!rec.mstrSection.empty()) &&
				(!mstrSection.empty()) &&
				(!rec.mstrKey.empty()) &&
				(!mstrKey.empty())
				)
				return ((rec.mstrSection == mstrSection)&&(rec.mstrKey == mstrKey));
			else
				return false;
		}
	};

	vector<STRU_RECORD> moContent;
	bool GetRecord(const string& astrKeyName, const string& astrSectionName,string & astrValue);

public:
	bool Open(string astrFilename);
	void Close();
	bool GetValue(const string& astrKey, const string& mstrSessionName, int8& aiValue, const int8 aiDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, int16& aiValue, const int16 aiDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, BYTE& abyValue, const BYTE abyDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, WORD& awValue, const WORD awDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, INT& aiValue, const INT aiDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, uint32& aiValue, const uint32 aiDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, LONG& alValue, const LONG alDefaultValue = 0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, double& adbValue, const double adbDefaultValue = 0.0);
	bool GetValue(const string& astrKey, const string& mstrSessionName, string& astrValue, const string& astrDefaultValue = "");
};
#endif //__INIEX_H