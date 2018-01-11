// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: InitEx.h
// 创 建 人: 史云杰
// 文件说明: ini文件
// $_FILEHEADER_END ******************************
#include "stdafx.h"
#include "IniEx.h"

CIniEx::CIniEx()
{
}

CIniEx::~CIniEx()
{
	Close();
}

void Trim(std::string& str, const std::string & ChrsToTrim = " \t\n\r", int TrimDir = 0)
{
	size_t startIndex = str.find_first_not_of(ChrsToTrim);
	if (startIndex == std::string::npos){str.erase(); return;}
	if (TrimDir < 2) str = str.substr(startIndex, str.size()-startIndex);
	if (TrimDir!=1) str = str.substr(0, str.find_last_not_of(ChrsToTrim) + 1);
}

bool CIniEx::GetRecord(const string& astrKeyName, const string& astrSectionName,string & astrValue)
{
	for(vector<STRU_RECORD>::iterator ite =  moContent.begin();
		ite != moContent.end(); ite++)
	{
		STRU_RECORD &loRecord = *ite;
		if((loRecord.mstrSection != astrSectionName)||(loRecord.mstrKey != astrKeyName))
		{
			continue;
		}
		astrValue = loRecord.mstrValue;
		return true;
				
	}
	return false;
}

bool CIniEx::Open(string astrFilename)
{
	//
	string lstrLineData = "";
	string lstrCurrentSection = ""; // Session Name

	ifstream inFile (astrFilename.c_str());
	
	if (!inFile.is_open()) 
		return false;
		
	moContent.clear();
	string lstrComments = ""; // A string to store comments in

	bool lbIniIsEof = false;

	while(!lbIniIsEof)
	{
		lbIniIsEof = std::getline(inFile, lstrLineData).eof();

		Trim(lstrLineData);
		if(!lstrLineData.empty())
		{
			STRU_RECORD r;

			lstrComments = lstrLineData;
			if(lstrLineData.find("#")==string::npos)
			{
				r.mcCommented = ' ';
				if(lstrLineData.find("[")!=string::npos)
				{		
					lstrLineData.erase(lstrLineData.begin());
					lstrLineData.erase(lstrLineData.find("]"));
					r.mstrComments = lstrComments;
					lstrComments = "";
					r.mstrSection = lstrLineData;
					r.mstrKey = "";
					r.mstrValue = "";
					lstrCurrentSection = lstrLineData;
				}

				if(lstrLineData.find("=")!=string::npos)
				{
					r.mstrComments = lstrComments;
					lstrComments = "";
					r.mstrSection = lstrCurrentSection;	
					r.mstrKey = lstrLineData.substr(0,lstrLineData.find('='));
					r.mstrValue = lstrLineData.substr(lstrLineData.find('=')+1);	
				}
				if(lstrComments == "")
					moContent.push_back(r);
			}
		}
	}

	inFile.close();
	return true;
}

void CIniEx::Close()
{
	moContent.clear();
}

bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, string& astrValue, const string& astrDefaultValue)
{
	string lstrValue="";
	
	if(GetRecord(astrKey, mstrSessionName,astrValue))
	{
		return true;
	}
	astrValue = astrDefaultValue;
	return false;
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, 
					  int8& aiValue, const int8 aiDefaultValue  )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		aiValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		aiValue = aiDefaultValue;
		return false;
	}
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, 
					   BYTE& abyValue, const BYTE abyDefaultValue  )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		abyValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		abyValue = abyDefaultValue;
		return false;
	}
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, WORD& awValue, const WORD awDefaultValue )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		awValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		awValue = awDefaultValue;
		return false;
	}
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, INT& aiValue, const INT aiDefaultValue)
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		aiValue = atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		aiValue = aiDefaultValue;
		return false;
	}
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, uint32& aiValue, const uint32 aiDefaultValue )
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		aiValue = (uint32)atoi(lstrValue.c_str());
		return true;
	}
	else
	{
		aiValue = aiDefaultValue;
		return false;
	}
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, 
					  LONG& alValue, const LONG alDefaultValue)
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		alValue = atol(lstrValue.c_str());
		return true;
	}
	else
	{
		alValue = alDefaultValue;
		return false;
	}
}
bool CIniEx::GetValue(const string& astrKey, const string& mstrSessionName, 
					  double& adbValue, const double adbDefaultValue)
{
	string lstrValue;
	if(GetValue(astrKey, mstrSessionName, lstrValue))
	{
		adbValue = atof(lstrValue.c_str());
		return true;
	}
	else
	{
		adbValue = adbDefaultValue;
		return false;
	}
}