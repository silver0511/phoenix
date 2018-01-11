#ifndef __SERVER_UTIL_MGR_H
#define __SERVER_UTIL_MGR_H

#include "base0/platform.h"
#include <sstream>

class ServerUtilMgr
{
public:
    static inline UINT64 ChangeStringToUll(std::string str)
	{
		if(str.empty())
		{
			return 0;
		}
		
		return strtoull(str.c_str(), nullptr, 10);
	}

	static inline UINT64 STR2UINT64(const std::string& str)
	{
		if(str.empty())
		{
			return 0;
		}
		
		return strtoull(str.c_str(), nullptr, 10);
	}

	static inline uint8 STR2UINT8(const std::string& szStr)
	{
		uint8 dst = 255;
		std::stringstream ss;
		ss << szStr;
		ss >> dst;

		return dst;
	}

	static unsigned char ToHex(unsigned char x) 
	{
		 return  x > 9 ? x + 55 : x + 48;
	}
	
	static std::string UrlEncode(const std::string& str)
	{
		std::string strTemp = "";  
		size_t length = str.length();  
		for (size_t i = 0; i < length; i++)  
		{  
			if (isalnum((unsigned char)str[i]) ||   
					(str[i] == '-') ||  
					(str[i] == '_') ||   
					(str[i] == '.') ||   
					(str[i] == '~'))
			{
				strTemp += str[i];
			}
			else if (str[i] == ' ')
			{
				strTemp += "+";
			}
			else  
			{  
				strTemp += '%';  
				strTemp += ToHex((unsigned char)str[i] >> 4);  
				strTemp += ToHex((unsigned char)str[i] % 16);  
			}
		}  
		return strTemp;
	}

	//buf是malloc出来的所以要自己free
	static unsigned char* GetData(const string &filename, bool for_string, int &buf_len);
};

#endif
