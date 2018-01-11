// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: basefun.cpp
// 创 建 人: 史云杰
// 文件说明: 基本函数集
// $_FILEHEADER_END ******************************
#include "stdafx.h"

#include "common/basefun.h"
#include <stdlib.h>

string GetAppPathA()
{
#ifdef _WIN32
	char strAppPath[255]={0};

	DWORD dwSize = 255;
	dwSize = ::GetModuleFileNameA(NULL, (LPSTR)strAppPath, dwSize);

	char* lpPath = (char*)strAppPath;
	int i=0;
	for(i=dwSize-1; i>0; i--)
	{
		if(lpPath[i] == '\\' || lpPath[i] == ':')
			break;
	}

	char lstrPath[255]={0};
	strncpy(lstrPath, lpPath, i+1);
	lstrPath[i+1] = '\0';

	return (string)lstrPath;

#else	

	char lcAppPath[255+1]={0}; 
	char lcFullPath[255+1]={0};

	sprintf(lcAppPath, "/proc/%d/exe", getpid());
	readlink(lcAppPath, lcFullPath, 255);

	int i=0;
	for(i=strlen(lcFullPath); i>=0; i--)
	{
		if (lcFullPath[i] == '/')
		{
			lcFullPath[i] = '\0';
			break;
		}
	}

	string lstrPath = lcFullPath;
	lstrPath += "/";

	return lstrPath;
#endif
}
#ifdef WIN32
u9_string GetAppPathU9()
{
	U9_CHAR strAppPath[255]={0};
	DWORD dwSize = sizeof(strAppPath) / sizeof(strAppPath[0]);
	dwSize = ::GetModuleFileName(NULL, strAppPath, dwSize);
	u9_strlwr(strAppPath);
	//for (int i = 0; i < dwSize; ++i)
	//{
	//	strAppPath[i] = tolower(strAppPath[i]);
	//}
	U9_CHAR* lpPath = (U9_CHAR*)strAppPath;
	int i=0;
	for(i=dwSize-1; i>0; i--)
	{
		if(lpPath[i] == _T('\\') || lpPath[i] == _T(':'))
			break;
	}

	U9_CHAR lstrPath[255]={0};
	u9_strncpy(lstrPath, lpPath, i+1);
	lstrPath[i+1] = _T('\0');

	return (u9_string)lstrPath;

}
#endif  //WIN32

BOOL IsInRange(INT aiValue, INT aiStart, INT aiEnd)
{
	if (aiValue >= aiStart && aiValue <= aiEnd)
		return TRUE;
	return FALSE;
}


BOOL GetHexNum(char acChar,int32 &aiValue)
{
	acChar = tolower(acChar);
	int32 liValue = '9';
	liValue = liValue-acChar;
	if(liValue>=0 && liValue<=9)
	{
		aiValue = 9-liValue;
		return TRUE;
	}

	liValue = 'f'-acChar;
	if(liValue<0)
		return FALSE;
	if(liValue>5)
		return FALSE;
	aiValue = 15-liValue;
	return TRUE;
}
// $_FUNCTION_BEGIN ******************************
// 方法名: lmahtoi
// 访  问: public 
// 参  数: char * aszStr
// 参  数: int32 &aiValue out
// 返回值: BOOL
// 说  明: 字符串（16进制字符字符串转换成数字）
// $_FUNCTION_END ********************************
BOOL lmhexatoi(char * aszStr,int32 &aiValue)
{
	if(NULL  == aszStr)
		return FALSE;
	int32 liValue = 0;
	int32 liTemp = 0;
	int32 i=0;
	while (aszStr[i] !=0)
	{	
		if(!GetHexNum(aszStr[i],liTemp))
			return FALSE;
		liValue=liValue*16+liTemp;
		i++;
	}
	aiValue=liValue;
	return TRUE;
}
BOOL lmmacatoi(char* aszMac,int64& ai64Mac)
{
	if(NULL == aszMac)
		return FALSE;
	int32 liLen = (int32)strlen(aszMac);
	if(17 != liLen)
		return FALSE;

	char *lszMac = aszMac;
	char lszValue[3]={0};
	
	for(int i=0;i<2;i++)
	{
		lszValue[i]=lszMac[i];
	}
	lszMac+=3;

	int32 liValue = 0;
	if(!lmhexatoi(lszValue,liValue))
		return FALSE;
	int64 li64Temp =liValue;
	ai64Mac = 0;

	ai64Mac += li64Temp<<40;

	
	for(int i=0;i<2;i++)
	{
		lszValue[i]=lszMac[i];
	}
	lszMac+=3;
	if(!lmhexatoi(lszValue,liValue))
		return FALSE;
	li64Temp =liValue;
	ai64Mac += li64Temp<<32;

	for(int i=0;i<2;i++)
	{
		lszValue[i]=lszMac[i];
	}
	lszMac+=3;
	if(!lmhexatoi(lszValue,liValue))
		return FALSE;
	li64Temp =liValue;
	ai64Mac += li64Temp<<24;

	for(int i=0;i<2;i++)
	{
		lszValue[i]=lszMac[i];
	}
	lszMac+=3;
	if(!lmhexatoi(lszValue,liValue))
		return FALSE;
	li64Temp =liValue;
	ai64Mac += li64Temp<<16;

	for(int i=0;i<2;i++)
	{
		lszValue[i]=lszMac[i];
	}
	lszMac+=3;
	if(!lmhexatoi(lszValue,liValue))
		return FALSE;
	li64Temp =liValue;
	ai64Mac += li64Temp<<8;

	for(int i=0;i<2;i++)
	{
		lszValue[i]=lszMac[i];
	}
	if(!lmhexatoi(lszValue,liValue))
		return FALSE;
	ai64Mac +=liValue;
	return TRUE;
}

#ifdef WIN32
#include <Nb30.h>
#pragma comment(lib,"Netapi32.lib")

INT GetLocalMacStr(char* apMac, INT aiBufLen) 
{ 
	NCB loNcb; 

	typedef struct _ASTAT_ 
	{ 
		ADAPTER_STATUS adapt; 
		NAME_BUFFER NameBuff [30]; 
	} ASTAT, * PASTAT; 
	ASTAT loAdapter;

	typedef struct _LANA_ENUM 
	{ // le 
		UCHAR length; 
		UCHAR lana[MAX_LANA]; 
	} LANA_ENUM ; 
	LANA_ENUM loLanaEnum; 

	UCHAR luRetCode; 
	memset( &loNcb, 0, sizeof(loNcb) ); 
	memset( &loLanaEnum, 0, sizeof(loLanaEnum)); 

	loNcb.ncb_command = NCBENUM; 
	loNcb.ncb_buffer = (unsigned char *) &loLanaEnum; 
	loNcb.ncb_length = sizeof(LANA_ENUM); 
	luRetCode = Netbios( &loNcb ); 
	if( luRetCode != NRC_GOODRET ) 
		return luRetCode ; 

	for( INT lana=0; lana<loLanaEnum.length; lana++ ) 
	{ 
		loNcb.ncb_command = NCBRESET; 
		loNcb.ncb_lana_num = loLanaEnum.lana[lana]; 
		luRetCode = Netbios( &loNcb ); 
		if( luRetCode == NRC_GOODRET ) 
			break ; 
	} 
	if( luRetCode != NRC_GOODRET ) 
		return luRetCode; 

	memset( &loNcb, 0, sizeof(loNcb) ); 
	loNcb.ncb_command = NCBASTAT; 
	loNcb.ncb_lana_num = loLanaEnum.lana[0]; 
	strcpy( (char* )loNcb.ncb_callname, "*" ); 
	loNcb.ncb_buffer = (unsigned char *) &loAdapter; 
	loNcb.ncb_length = sizeof(loAdapter); 
	luRetCode = Netbios( &loNcb ); 
	if( luRetCode != NRC_GOODRET ) 
		return luRetCode ; 
	_snprintf(apMac, aiBufLen-1, "%02X-%02X-%02X-%02X-%02X-%02X", 
		loAdapter.adapt.adapter_address[0], 
		loAdapter.adapt.adapter_address[1], 
		loAdapter.adapt.adapter_address[2], 
		loAdapter.adapt.adapter_address[3], 
		loAdapter.adapt.adapter_address[4], 
		loAdapter.adapt.adapter_address[5] ); 
	return 0; 
}

INT GetLocalMacINT64(INT64& ai64Mac)
{
	NCB loNcb; 

	typedef struct _ASTAT_ 
	{ 
		ADAPTER_STATUS adapt; 
		NAME_BUFFER NameBuff [30]; 
	} ASTAT, * PASTAT; 
	ASTAT loAdapter;

	typedef struct _LANA_ENUM 
	{ // le 
		UCHAR length; 
		UCHAR lana[MAX_LANA]; 
	} LANA_ENUM ; 
	LANA_ENUM loLanaEnum; 

	UCHAR luRetCode; 
	memset( &loNcb, 0, sizeof(loNcb) ); 
	memset( &loLanaEnum, 0, sizeof(loLanaEnum)); 

	loNcb.ncb_command = NCBENUM; 
	loNcb.ncb_buffer = (unsigned char *) &loLanaEnum; 
	loNcb.ncb_length = sizeof(LANA_ENUM); 
	luRetCode = Netbios( &loNcb ); 
	if( luRetCode != NRC_GOODRET ) 
		return luRetCode ; 

	for( INT lana=0; lana<loLanaEnum.length; lana++ ) 
	{ 
		loNcb.ncb_command = NCBRESET; 
		loNcb.ncb_lana_num = loLanaEnum.lana[lana]; 
		luRetCode = Netbios( &loNcb ); 
		if( luRetCode == NRC_GOODRET ) 
			break ; 
	} 
	if( luRetCode != NRC_GOODRET ) 
		return luRetCode; 

	memset( &loNcb, 0, sizeof(loNcb) ); 
	loNcb.ncb_command = NCBASTAT; 
	loNcb.ncb_lana_num = loLanaEnum.lana[0]; 
	strcpy( (char* )loNcb.ncb_callname, "*" ); 
	loNcb.ncb_buffer = (unsigned char *) &loAdapter; 
	loNcb.ncb_length = sizeof(loAdapter); 
	luRetCode = Netbios( &loNcb ); 
	if( luRetCode != NRC_GOODRET ) 
		return luRetCode ; 

	INT64 li64Temp = loAdapter.adapt.adapter_address[0];
	ai64Mac = li64Temp<<40;
	li64Temp = loAdapter.adapt.adapter_address[1];
	ai64Mac += li64Temp<<32;
	li64Temp = loAdapter.adapt.adapter_address[2];
	ai64Mac += li64Temp<<24;
	li64Temp = loAdapter.adapt.adapter_address[3];
	ai64Mac += li64Temp<<16;
	li64Temp = loAdapter.adapt.adapter_address[4];
	ai64Mac += li64Temp<<8;
	li64Temp = loAdapter.adapt.adapter_address[5];
	ai64Mac += li64Temp;
	return 0; 
}

INT64 GetLocalMac(BOOL abRandom)
{
	INT64 li64Ret = 0;
	if (abRandom)
	{
		li64Ret = (rand() + GetTickCount()) % 0xFFFFFFFF;
	}
	else
	{
		GetLocalMacINT64(li64Ret);
	}
	return li64Ret;
}

#else
//#include <string>
//#include <iostream>
//using namespace std;
////命令行输出缓冲大小
//const long MAX_COMMAND_SIZE = 10000;
//
////获取MAC命令行
//char szFetCmd[] = "ifconfig /all";
////网卡MAC地址的前导信息
//const string str4Search = "Physical Address. . . . . . . . . : ";
//
//////////////////////////////////////////////////////////////////////////////
//// 函数名： GetMacByCmd(char *lpszMac)
//// 参数：
////      输入： void
////      输出： lpszMac,返回的MAC地址串
//// 返回值：
////      TRUE:  获得MAC地址。
////      FALSE: 获取MAC地址失败。
//// 过程：
////      1. 创建一个无名管道。
////      2. 创建一个IPCONFIG 的进程，并将输出重定向到管道。
////      3. 从管道获取命令行返回的所有信息放入缓冲区lpszBuffer。
////      4. 从缓冲区lpszBuffer中获得抽取出MAC串。
////
////  提示：可以方便的由此程序获得IP地址等其他信息。
////        对于其他的可以通过其他命令方式得到的信息只需改变strFetCmd 和
////        str4Search的内容即可。
/////////////////////////////////////////////////////////////////////////////
//INT GetLocalMacStr( char* apMac, INT aiBufLen) 
//{
//	//初始化返回MAC地址缓冲区
//	memset(apMac, 0x00, sizeof(apMac));
//	BOOL bret; 
//
//	SECURITY_ATTRIBUTES sa;
//	HANDLE hReadPipe,hWritePipe;
//
//	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
//	sa.lpSecurityDescriptor = NULL;
//	sa.bInheritHandle = TRUE;
//
//	//创建管道
//	bret = CreatePipe(&hReadPipe, &hWritePipe, &sa, 0);
//	if(!bret)
//	{
//		return -1;
//	}
//
//	//控制命令行窗口信息
//	STARTUPINFO si;
//	//返回进程信息
//	PROCESS_INFORMATION pi;
//
//	si.cb = sizeof(STARTUPINFO);
//	GetStartupInfo(&si);
//	si.hStdError = hWritePipe;
//	si.hStdOutput = hWritePipe;
//	si.wShowWindow = SW_HIDE; //隐藏命令行窗口
//	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
//
//	//创建获取命令行进程
//	bret = CreateProcess (NULL, szFetCmd, NULL, NULL, TRUE, 0, NULL,
//		NULL, &si, &pi );
//
//	char szBuffer[MAX_COMMAND_SIZE+1]; //放置命令行输出缓冲区
//	string strBuffer;
//
//	if (bret)
//	{
//		WaitForSingleObject (pi.hProcess, INFINITE);
//		unsigned long count;
//		CloseHandle(hWritePipe);
//
//		memset(szBuffer, 0x00, sizeof(szBuffer));
//		bret  =  ReadFile(hReadPipe,  szBuffer,  MAX_COMMAND_SIZE,  &count,  0);
//		if(!bret)
//		{
//			//关闭所有的句柄
//			CloseHandle(hWritePipe);
//			CloseHandle(pi.hProcess);
//			CloseHandle(pi.hThread);
//			CloseHandle(hReadPipe);
//			return -1;
//		}
//		else
//		{
//			strBuffer = szBuffer;
//			long ipos;
//			ipos = strBuffer.find(str4Search);
//
//			//提取MAC地址串
//			strBuffer = strBuffer.substr(ipos+str4Search.length());
//			ipos = strBuffer.find("\n");
//			strBuffer = strBuffer.substr(0, ipos);
//		}
//	}
//
//	memset(szBuffer, 0x00, sizeof(szBuffer));
//	strcpy(szBuffer, strBuffer.c_str());
//
//	//去掉中间的“00-50-EB-0F-27-82”中间的'-'得到0050EB0F2782
//	int j = 0;
//	for(int i=0; i<strlen(szBuffer); i++)
//	{
//		if(szBuffer[i] != '-')
//		{
//			apMac[j] = szBuffer[i];
//			j++;
//		}
//	}
//
//	//关闭所有的句柄
//	CloseHandle(hWritePipe);
//	CloseHandle(pi.hProcess);
//	CloseHandle(pi.hThread);
//	CloseHandle(hReadPipe);
//	return 1;
//}

INT GetLocalMacINT64(INT64& ai64Mac)
{
	return 0;
}

#endif //WIN32


#ifdef TRACE_LOG
//创建调试信息
//VOID CreateDebugInfo(const char* apAppName,int aiDISPLAYType)
//{
//	//生成TRACE文件名
//	time_t ltmNow;
//	time( &ltmNow );
//    struct tm loTimeFormat;
//    loTimeFormat = *localtime( &ltmNow );
//	struct tm * lstrutime = &loTimeFormat;
//
//	char lszLogFileName[512+1]={0};
//
//#ifdef WIN32
//	_snprintf(lszLogFileName,512, "%s%s_%4d%02d%02d_%d%s", 
//		GetAppPathA().c_str(), apAppName, (1900+lstrutime->tm_year),\
//		(1+lstrutime->tm_mon), lstrutime->tm_mday, rand()%10000,".log");
//#else
//	snprintf(lszLogFileName, 512,"%s%s%4d%02d%02d_%d%s", 
//		GetAppPathA().c_str(), apAppName, (1900+lstrutime->tm_year),\
//		(1+lstrutime->tm_mon), lstrutime->tm_mday, rand()%10000,".log");
//#endif 
//
//	string lstrLogFileName;
//	lstrLogFileName = lszLogFileName;
//	printf("logfile:%s \n\r",lstrLogFileName.c_str());
//	//
//
//	//LOG_INIT(DEF_DISPLAY_TYPE_ALL,5,
//	LOG_INIT(aiDISPLAYType,TRACE_LOG_CURRENT_LEVEL,
//		(char*)(lstrLogFileName.c_str()),(char*)apAppName,);
//	LOG_TRACE(1,true,(char*)apAppName,"CreateDebugInfo\t"<<apAppName<<"调试日志");
//
//	return;
//}
#endif

unsigned int   ELFHash(const char *apszData,unsigned int aiLen) 
{
	register unsigned int hash = 0;
	unsigned int x = 0, i=0;
	for(i = 0; i < aiLen; i++) {
		hash = (hash << 4) + apszData[i];
		if((x = hash & 0xF0000000L) != 0) {
			hash ^= (x >> 24);
			hash &= ~x;}
		
	}
	return (hash & 0x7FFFFFFF);
}
WORD   get_RandPackID()
{
	struct timeb loTime;
	ftime( &loTime ); 
	WORD lwPackageID=(rand()+loTime.millitm)%(65536);
	return lwPackageID;
}
// -----------------------------------
char *trimstr(char *s1)
{
	while (*s1)
	{
		if ((*s1 == ' ') || (*s1 == '\t'))
			s1++;
		else
			break;

	}

	char *s = s1;

	s1 = s1+strlen(s1);

	while (*--s1)
		if ((*s1 != ' ') && (*s1 != '\t'))
			break;

	s1[1] = 0;

	return s;
}

// -----------------------------------
char *stristr(const char *s1, const char *s2)
{
	while (*s1)
	{
		if (TOUPPER(*s1) == TOUPPER(*s2))
		{
			const char *c1 = s1;
			const char *c2 = s2;

			while (*c1 && *c2)
			{
				if (TOUPPER(*c1) != TOUPPER(*c2))
					break;
				c1++;
				c2++;
			}
			if (*c2==0)
				return (char *)s1;
		}

		s1++;
	}
	return NULL;
}

INT   Strcmp(const char *apszStr1,const char * apszStr2) 
{
	if (NULL == apszStr1)
	{
		if (NULL==apszStr2)
			return 0;
		else 
			return -1;
	}
	else
	{
		if (NULL==apszStr2)
			return 1;
		else 
			return strcmp(apszStr1,apszStr2);
	}
}
INT   Tolower_String(char *apszDest,const char *apszSrc)
{
	int i=0;
	register char lcChar=0;
	while (0 !=(lcChar = apszSrc[i]))
	{
		if ((lcChar >='A')&&(lcChar <= 'Z'))
			lcChar+=32;
		apszDest[i]=lcChar;
		i++;
	}
	apszDest[i]=0;
	return 1;
}
INT CompareStringM(const char *apszStr1,const char *apszStr2)
{//完全比较
	int i=0;
	char lcChar1 = 0;
	char lcChar2  = 0;
	if (NULL == apszStr1)
	{
		if (NULL==apszStr2)
			return 0;
		else 
			return -1;
	}
	else
	{
		if (NULL==apszStr2)
			return 1;
		
		while(1){
			lcChar1=apszStr1[i];
			lcChar2 = apszStr2[i];
			
			if ((lcChar1 >='A')&&(lcChar1 <='Z'))
				lcChar1+=32;
			
			if ((lcChar2 >='A')&&(lcChar2 <='Z'))
				lcChar2+=32;

			if (0 ==lcChar1 ){
				if(0 == lcChar2)
					return 0;
				else 
					return -1;
			}
			else{
				if (lcChar1 <lcChar2)
					return -1;
				else if (lcChar1 >lcChar2)
					return 1;
			}
			
			i++;
		}

	}

    
}
int   Strstr_Slur(unsigned char *apData,int aiDataLen,char *apszKey)
{//可以再优化,
	int i=0,j=0,k=0;
	unsigned char lcKeyChar=0;//关键字字符
	unsigned char lcChar=0;//被检测字符
	int liDetectLen=0;
	if ((NULL == apData)||(NULL ==apszKey )||(0 == aiDataLen))
		return 0;
	
	liDetectLen =(int) strlen(apszKey);
	while (1){
		lcChar=apData[i];
		if (lcChar>=0x80){//被检测字符是unicode
			lcKeyChar= apszKey[j];
			if (lcKeyChar >=0x80){//都是unicode
				if (lcChar !=lcKeyChar)//不相等
					return 0;
				j++;
				if (j >=liDetectLen)
					return 0;
				
				i++;
				if (i>=aiDataLen)
					return 0;
								
				lcChar=apData[i];
				lcKeyChar=apszKey[j];
				if (lcChar !=lcKeyChar)//不相等
					return 0;
				//相等
				j++;
				if (j >=liDetectLen)//分析结束
					return 1;
				
				i++;
				if (i>=aiDataLen)
					return 0;
			}
			else{//被检测字符是unicode，关键字字符是ascic
				return 0;
			}
		}
		else{//被检测字符是ascii
			lcKeyChar= apszKey[j];
			if (lcKeyChar >=0x80){//被检测字符是ascic，关键字字符是unicode
				if (0==j)//第一次检测
					return 0;
				//除去干扰，容忍度是6个字符，
				k=0;
				while (k<7){
					i++;
					if (i>=aiDataLen)
						return 0;
					if (apData[i]>=0x80)
						break;
					k++;
				}
				if (k>=6)
					return 0;
			}
			else{//都是ascii
				//除去大小写敏感,都转换成小写比较
				if ((lcKeyChar >='A')&&(lcKeyChar <='Z'))
					lcKeyChar+=32;
				if ((lcChar >='A')&&(lcChar <= 'Z'))
					lcChar+=32;

				if (lcKeyChar != lcChar)//找到相等的
					return 0;
				
				//相等
				j++;
				if (j >=liDetectLen)//分析结束
					return 1;
				
				i++;
				if (i>=aiDataLen)
					return 0;
			}
		}//else
	}//while	
}

char *   Strstr(unsigned char *apData,int aiDataLen,char *apszKey)
{//可以再优化,
	int i=0,j=0;
	unsigned char lcKeyChar=0;//关键字字符
	unsigned char lcChar=0;//被检测字符
	int liDetectLen=0;
	if ((NULL == apData)||(NULL ==apszKey )||(0 == aiDataLen))
		return 0;
	
	liDetectLen = (int)strlen(apszKey);
	while (1){
		lcChar=apData[i];
		if (lcChar>=0x80){//被检测字符是unicode
			lcKeyChar= apszKey[j];
			if (lcKeyChar >=0x80){//都是unicode
				if (lcChar !=lcKeyChar)//不相等
					return NULL;
				j++;
				if (j >=liDetectLen)
					return NULL;
				
				i++;
				if (i>=aiDataLen)
					return NULL;
								
				lcChar=apData[i];
				lcKeyChar=apszKey[j];
				if (lcChar !=lcKeyChar)//不相等
					return NULL;
				//相等
				j++;
				if (j >=liDetectLen)//分析结束
					return (char *)(apData+i);
				
				i++;
				if (i>=aiDataLen)
					return NULL;
			}
			else{//被检测字符是unicode，关键字字符是ascic
				return NULL;
			}
		}
		else{//被检测字符是ascii
			lcKeyChar= apszKey[j];
			if (lcKeyChar >=0x80){//被检测字符是ascic，关键字字符是unicode
				i++;
				if (i>=aiDataLen)
					return NULL;
			}
			else{//都是ascii
				//除去大小写敏感,都转换成小写比较
				if ((lcKeyChar >='A')&&(lcKeyChar <='Z'))
					lcKeyChar+=32;
				if ((lcChar >='A')&&(lcChar <='Z'))
					lcChar+=32;

				if (lcKeyChar != lcChar)//找到相等的
					return NULL;
				
				//相等
				j++;
				if (j >=liDetectLen)//分析结束
					return (char *) (apData+i);
				
				i++;
				if (i>=aiDataLen)
					return NULL;
			}
		}//else
	}//while	
}

int   HasWordinString(unsigned char *apData,int aiDataLen,char ** appWord,int aiWordCount)
{//
	register int i=0;
	for(i=0;i<aiWordCount;i++){
		if (1==Strstr_Slur(apData,aiDataLen,appWord[i]))
			return 1;
	}
	return 0;
}
int   KeyInText(unsigned char *apData,int aiDataLen,char * apKey)
{
	return WordsInText(apData,aiDataLen,&apKey,1);
}
int   WordsInText(unsigned char *apData,int aiDataLen,char ** appWord,int aiWordCount)
{	
	int i=0;
	unsigned char *lpcTemp=NULL;
	unsigned char lcChar=0,lcFirstChar=0;

	while (i<aiDataLen){
		lcChar=apData[i];
		if ('\r' == lcChar ){//这里假设关键字字符中没有\r\n
			i+=2;//跳过"\n\r"
			continue;
		}
		else if ('\n' == lcChar ){
			i++;//跳过'\n'
			continue;
		}	
		else if (lcChar >=0x80){//unicode
			lcFirstChar=lcChar;
			i++;
			if (i<aiDataLen){
				lcChar=apData[i];
				if (lcChar>=0x80){//是 GBK或GB2312  内码
					lpcTemp = apData+i-1;
					if (1==HasWordinString(lpcTemp,aiDataLen-i+1,appWord,aiWordCount))
						return 1;
				}
				else{//看第二字节是否是汉字低字节
					if ((0x40 <= lcChar) && (lcChar <= 0x7E)){//是 GBK 内码
						lpcTemp = apData+i-1;
						if (1==HasWordinString(lpcTemp,aiDataLen-i+1,appWord,aiWordCount))
							return 1;
					}
					else{//跳过第一个>0x80的字节。按ascii处理
						lpcTemp = apData+i;
						if (1==HasWordinString(lpcTemp,aiDataLen-i,appWord,aiWordCount))
							return 1;
					}
				}//else
			}//if
			else{//到结尾,出现一个单字节，但它>0x80，忽略
				return 0;
			}
			i++;//进行下一个匹配
		}
		else{//Ascii
			lpcTemp = apData+i;
			if (1==HasWordinString(lpcTemp,aiDataLen-i,appWord,aiWordCount))
				return 1;
			i++;//进行下一个匹配
		}
	}//while

	return 0;
}
void   CountWordinString(unsigned char *apData,int aiDataLen,char ** appWord,
					  int aiWordCount,int *apiWordList)
{//
	register int i=0;
	for(i=0;i<aiWordCount;i++){
		if (1==Strstr_Slur(apData,aiDataLen,appWord[i])){
			apiWordList[i]++;	
		}
	}
}

int   StrCountinText(unsigned char  *apData,int aiDataLen,char ** appWord,
				   int aiWordCount,int *apiWordListCount,int *aiLetterCount)
{//可以再优化,
	int i=0;
	unsigned char *lpcTemp=NULL;
	unsigned char lcChar=0,lcFirstChar=0;
	int liLen=0;
	*apiWordListCount=0;
	while (i<aiDataLen){
		lcChar=apData[i];
		if ('\r' == lcChar ){//这里假设关键字字符中没有\r\n
			i+=2;//跳过"\n\r"
			continue;
		}
		else if ('\n' == lcChar ){
			i++;//跳过'\n'
			continue;
		}	
		else if (lcChar >=0x80){//unicode
			lcFirstChar=lcChar;
			i++;
			if (i<aiDataLen){
				lcChar=apData[i];
				if (lcChar>=0x80){//是 GBK或GB2312  内码
					lpcTemp = apData+i-1;
					CountWordinString(lpcTemp,aiDataLen-i+1,appWord,aiWordCount,apiWordListCount);
					liLen+=2;
				}
				else{//看第二字节是否是汉字低字节
					if ((0x40 <= lcChar) && (lcChar <= 0x7E)){//是 GBK 内码
						lpcTemp = apData+i-1;
						CountWordinString(lpcTemp,aiDataLen-i+1,appWord,aiWordCount,apiWordListCount);
						liLen+=2;	
					}
					else{//跳过第一个>0x80的字节。按ascii处理
						lpcTemp = apData+i;
						CountWordinString(lpcTemp,aiDataLen-i+1,appWord,aiWordCount,apiWordListCount);
						if (' ' != lcChar)
							liLen++;
					}
				}//else
			}//if
			else{//到结尾,出现一个单字节，但它>0x80，忽略
				if (NULL != aiLetterCount)
					*aiLetterCount=liLen;
				return 0;
			}
			i++;//进行下一个匹配
		}
		else{//Ascii
			lpcTemp = apData+i;
			CountWordinString(lpcTemp,aiDataLen-i+1,appWord,aiWordCount,apiWordListCount);
			if (' ' != lcChar)
				liLen++;

			i++;//进行下一个匹配
		}
	}//while
	if (NULL != aiLetterCount)
		*aiLetterCount=liLen;
	return 0;
}
void   IgnoreBlank(char * apData,int aiDataLen,int *apiIndex)
{
	int i= *apiIndex;
	char lctemp=0;
	//暂时不需要，所以跳过
	while (i<aiDataLen){
		lctemp=apData[i];
		if ((' ' == lctemp)||('\n' == lctemp)){//仅跳过'\n'和' '
			i++;
		}
		else if ('\r' == lctemp){
			i+=2;//跳过"\r\n"
		}
		else
			break;
	}
	*apiIndex = i;
}
int  CompareStringbyList(char *apData,char **apWordList,int aiCount)
{//这一版本假设 data只有一个数据
	int i=0;
	//apRuleItem->wordlist
	//apRuleItem->wordcount

	if (NULL == apData)
		return 0;
	if (NULL == apWordList )
		return 0;
	for (i=0;i<aiCount;i++){
		if (0 == CompareStringM(apData,apWordList[i])){//相等
			return 1;
		}
	}
	return 0;
}
INT   ReadLineData(FILE *fp,char *apBuffer,unsigned int aiBufferLen,unsigned int* apiLen)
{
	unsigned int i=0;
	char lcChar=0;
	if (NULL ==fp)
		return 0;
	if (0 == apBuffer)
		return 0;
	if (0 == aiBufferLen)
		return 0;
	lcChar=(char )getc(fp);
	while (!feof(fp)){
		if ('\r' == lcChar){
			lcChar=(char )getc(fp);//读"\r\n"
			*apiLen=i;
			return 1;

		}
		else if ('\n' == lcChar){
			*apiLen=i;
			return 1;//跳过"\n
		}
		else{
			apBuffer[i]=lcChar;
			i++;
			if (i>=aiBufferLen)
				return -1;
		}
		lcChar=(char )getc(fp);
	}
	*apiLen=i;
	return 0;
}

string GetIPString(unsigned int aiIP)
{
	struct  in_addr addr;
	memset(&addr,0,sizeof(struct in_addr));
#ifdef WIN32
	addr.S_un.S_addr =aiIP;
#else
	addr.s_addr=aiIP;
#endif //WIN32
	string lstrTemp= inet_ntoa(addr);
	return lstrTemp;
}


u9_string& ReplaceAll(u9_string& str,const u9_string& old_value,const u9_string& new_value)
{
	while(true) 
	{
		u9_string::size_type pos(0);
		if( (pos=str.find(old_value, pos))!=u9_string::npos )
			str.replace(pos,old_value.length(),new_value);
		else break;
	}

	return str;
}

void ReparePath(u9_string& astrPath)
{
	ReplaceAll(astrPath, _T("/"), _T("\\"));
}

char* StrReplace(char* aSrcStr)
{
	if (NULL == aSrcStr )
	{
		return NULL;
	}
	INT i = 0;
	char c = aSrcStr[0];
	while (c != '\0')
	{
		if (c == '\n' || c == '\r')
		{
			aSrcStr[i] = ' ';
		}
		i++;
		c = aSrcStr[i];
	}
	return aSrcStr;
}
string FormatTime(ULONG aulTime, BOOL abDataTime)
{
	char lszBuffer[256+1] = {0};
    struct tm loTimeFormat = *localtime((time_t*)&aulTime);
	if (abDataTime)
	{
		strftime(lszBuffer, 256, "%Y:%m:%d %H:%M:%S", &loTimeFormat);
	}	
	else
	{
		strftime(lszBuffer, 256, "%H:%M:%S", &loTimeFormat);
	}
	return (string)lszBuffer;
}

string GetTimeString(ULONG aulTime)
{
	struct tm lstruTime;
	time_t ltTime =aulTime;

	memcpy(&lstruTime,localtime((time_t*)&ltTime),sizeof(lstruTime));

	string lstrRet = "";
	lstrRet = asctime(&lstruTime);

	return lstrRet;
}

// -----------------------------------
static int base64chartoval(char input)
{
	if(input >= 'A' && input <= 'Z')
		return input - 'A';
	else if(input >= 'a' && input <= 'z')
		return input - 'a' + 26;
	else if(input >= '0' && input <= '9')
		return input - '0' + 52;
	else if(input == '+')
		return 62;
	else if(input == '/')
		return 63;
	else if(input == '=')
		return -1;
	else
		return -2;
}

int base64WordToChars(char *out,const char *input)
{
	char *start = out;
	signed char vals[4];

	vals[0] = base64chartoval(*input++);
	vals[1] = base64chartoval(*input++);
	vals[2] = base64chartoval(*input++);
	vals[3] = base64chartoval(*input++);

	if(vals[0] < 0 || vals[1] < 0 || vals[2] < -1 || vals[3] < -1) 
		return 0;

	*out++ = vals[0]<<2 | vals[1]>>4;
	if(vals[2] >= 0)
		*out++ = ((vals[1]&0x0F)<<4) | (vals[2]>>2);
	else
		*out++ = 0;

	if(vals[3] >= 0)
		*out++ = ((vals[2]&0x03)<<6) | (vals[3]);
	else
		*out++ = 0;

	return int(out-start);
}
int itoa_IP(IPTYPE aIP, char* apszIPAddr,INT aiLen)
{
	if(NULL ==apszIPAddr)
		return -1;
	int li = 0;
	int larrR[4] = {0};

	while (aIP)
	{
		larrR[li++] = aIP & 255;
		aIP = aIP >> 8;
	}
	if (li != 4) 
	{
		if (li == 3) {larrR[3] = 0;}
		else if (li == 2) {larrR[3] = 0;larrR[2] = 0;}
		else if (li == 1) {larrR[3] = 0;larrR[2] = 0;larrR[1] = 0;}
		else if (li == 0) {larrR[3] = 0;larrR[2] = 0;larrR[1] = 0;larrR[0] = 0;}
		else { return -1; }
	}
#ifdef WIN32
	_snprintf(apszIPAddr,aiLen,"%03d.%03d.%03d.%03d",larrR[3],larrR[2],larrR[1],larrR[0]);
#else
	snprintf(apszIPAddr,aiLen,"%03d.%03d.%03d.%03d",larrR[3],larrR[2],larrR[1],larrR[0]);
#endif
	return 1;
}

IPTYPE atoi_IP(const char* apszIPAddr)
{
	IPTYPE lTemp = 0;
	IPTYPE lIP=0;
	int li = 0;
	char *lpIP=(char *)apszIPAddr;
	while (*lpIP)
	{
		if (*lpIP == '.')
		{
			lIP = (lIP << 8) | lTemp;
			lTemp = 0;
			++lpIP;
			++li;
			continue;
		}
		if (*lpIP < '0' || *lpIP > '9')
		{
			++lpIP;
			continue;
		}
		lTemp = lTemp * 10 + *lpIP - '0';
		++lpIP;
	}
	if (li != 3) return 0;
	lIP = (lIP << 8) | lTemp;
	return htonl(lIP);
}
// $_FUNCTION_BEGIN ******************************
// 函数名称：  GetIPAddress
// 函数参数：
// 返 回 值：  IPTYPE 
// 函数说明：得到ip地址 
// $_FUNCTION_END ********************************
IPTYPE GetIPAddress(const char *apszIPAddr)
{//
	if (NULL == apszIPAddr)
		return 0;

	hostent *lhphostent=NULL;
	try
	{
		lhphostent = ::gethostbyname(apszIPAddr);
	}
	catch (...)
	{
		return 0;
	}
	if(NULL == lhphostent)
		return 0;
	struct in_addr *lpAddr=((struct in_addr *)lhphostent->h_addr);
	return lpAddr->s_addr;
}

//目前只能取IPV4的内网地址
string GetIntranetIP()
{
	struct ifaddrs *stru_if_addr = NULL;
	void * addr_ptr = NULL;

	getifaddrs(&stru_if_addr);

	while (stru_if_addr!=NULL)
	{

		if (stru_if_addr->ifa_addr->sa_family == AF_INET)
		{
			addr_ptr = &((struct sockaddr_in *)stru_if_addr->ifa_addr)->sin_addr;
			char addr_buff[INET_ADDRSTRLEN];
			inet_ntop(AF_INET, addr_ptr, addr_buff, INET_ADDRSTRLEN);
			string intranet_ip = addr_buff;
			string head_ip = intranet_ip.substr(0,3);
			if(head_ip == "172" ||
			   head_ip == "10." ||
			   head_ip == "192")
			{
				printf("address ip name:%s ip:%s\n", stru_if_addr->ifa_name, addr_buff);
				if(strcmp(stru_if_addr->ifa_name,"eth0") == 0 ||
				   strcmp(stru_if_addr->ifa_name,"eth1") == 0 ||
				   strcmp(stru_if_addr->ifa_name,"em1") == 0 ||
				   strcmp(stru_if_addr->ifa_name,"em2") == 0)
				{
					printf("address ip get name:%s ip:%s\n", stru_if_addr->ifa_name, addr_buff);
					return intranet_ip;
				}
			}
		}

//		else if (stru_if_addr->ifa_addr->sa_family == AF_INET6)
//		{
//			addr_ptr=&((struct sockaddr_in *)stru_if_addr->ifa_addr)->sin_addr;
//			char addr_buff[INET6_ADDRSTRLEN];
//			inet_ntop(AF_INET6, addr_ptr, addr_buff, INET6_ADDRSTRLEN);
//			printf("%s IPV6 Address %s\n", stru_if_addr->ifa_name, addr_buff);
//		}

		stru_if_addr = stru_if_addr->ifa_next;
	}

	return "";
}

int PublicRandom()
{
	// return 32 bits of random stuff
	assert( sizeof(int) == 4 );
	static bool init=false;
	if ( !init )
	{ 
		init = true;

		UINT64 tick;

#if defined(WIN32) 
		volatile unsigned int lowtick=0,hightick=0;
		__asm
		{
			rdtsc 
				mov lowtick, eax
				mov hightick, edx
		}
		tick = hightick;
		tick <<= 32;
		tick |= lowtick;
#elif defined(__GNUC__) && ( defined(__i686__) || defined(__i386__) )
		asm("rdtsc" : "=A" (tick));
#elif defined (__SUNPRO_CC) || defined( __sparc__ )	
		tick = gethrtime();
#elif defined(__MACH__) 
		int fd=open("/dev/random",O_RDONLY);
		read(fd,&tick,sizeof(tick));
		closesocket(fd);
#else
//#     error Need some way to seed the random number generator 
#endif 
		int seed = int(tick);
#ifdef WIN32
		srand(seed);
#else
		srandom(seed);
#endif
	}

#ifdef WIN32
	assert( RAND_MAX == 0x7fff );
	int r1 = rand();
	int r2 = rand();

	int ret = (r1<<16) + r2;

	return ret;
#else
	return random(); 
#endif
}

int32 Double2Int(double adValue)
{
	if (adValue > 0)
	{
		return (int32)(adValue+0.5);
	}
	else
	{
		return (int32)(adValue-0.5);
	}
}

// $_FUNCTION_BEGIN ******************************
// 函数名称： 	IsFileExist
// 函数参数：	lpszFile 物理文件全路径
// 返 回 值：	
// 函数说明： 	判断指定物理文件是否存在
// $_FUNCTION_END ********************************
#ifdef WIN32
bool IsFileExist(const U9_CHAR* apszFile)
{
    FILE* lpf = NULL;
    //#ifdef UNICODE
    //	f = _wfopen((const wchar_t*)apszFile, _T("r");
    //#else
    lpf = u9_fopen(apszFile, _T("r"));
    //#endif
    if (lpf != NULL)
    {
        fclose(lpf);
        return true;
    }
    return false;
}

#else
bool IsFileExist(const char* apszFile)
{
	FILE* lpf = NULL;
	//#ifdef UNICODE
	//	f = _wfopen((const wchar_t*)apszFile, _T("r");
	//#else
	lpf = fopen(apszFile, "r");
	//#endif
	if (lpf != NULL)
	{
		fclose(lpf);
		return true;
	}
	return false;
}
#endif 



int UNICODE2UTF8(const wchar_t *awszInput_UNICODE,
				  char ** appszOutput_UTF8)
{
	if (awszInput_UNICODE == NULL) {
		return -1;
	}
	int size_d = 8;
	int buffer_size = 0;

	const wchar_t* p_unicode = awszInput_UNICODE;
	// count for the space need to allocate
	wchar_t w_char;
	do {
		w_char = *p_unicode;
		if (w_char < 0x80) {
			// utf char size is 1
			buffer_size += 1;
		} else if (w_char < 0x800) {
			// utf char size is 2
			buffer_size += 2;
		} else if (w_char < 0x10000) {
			// utf char size is 3
			buffer_size += 3;
		} else if (w_char < 0x200000) {
			// utf char size is 4
			buffer_size += 4;
		} else if (w_char < 0x4000000) {
			// utf char size is 5
			buffer_size += 5;
		} else {
			// utf char size is 6
			buffer_size += 6;
		}
		p_unicode++;
	}
	while (w_char != static_cast<char>(0));

	// allocate the memory ,call delete,
	char* utf8 = new char[buffer_size];
	if(utf8  == NULL)
		return -1;
	p_unicode = awszInput_UNICODE;
	int index_buffer = 0;
	// do the conversion
	do {
		w_char = *awszInput_UNICODE;  // the unicode char current being converted
		awszInput_UNICODE++;

		if (w_char < 0x80) {
			// apiOutput_UTF8_Len = 1;
			utf8[index_buffer++] = static_cast<char>(w_char);
		} else if (w_char < 0x800) {
			// apiOutput_UTF8_Len = 2;
			utf8[index_buffer++] = 0xc0 | (w_char >> 6);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else if (w_char < 0x10000) {
			// apiOutput_UTF8_Len = 3;
			utf8[index_buffer++] = 0xe0 | (w_char >> 12);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else if (w_char < 0x200000) {
			// apiOutput_UTF8_Len = 4;
			utf8[index_buffer++] = 0xf0 | (static_cast<int>(w_char) >> 18);
			utf8[index_buffer++] = 0x80 | ((w_char >> 12) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else if (w_char < 0x4000000) {
			// apiOutput_UTF8_Len = 5
			utf8[index_buffer++] = 0xf8| (static_cast<int>(w_char) >> 24);
			utf8[index_buffer++] = 0x80 | ((static_cast<int>(w_char) >> 18) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 12) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else {  // if(wchar >= 0x4000000)
			// all other cases apiOutput_UTF8_Len = 6
			utf8[index_buffer++] = 0xfc | (static_cast<int>(w_char) >> 30);
			utf8[index_buffer++] = 0x80 | ((static_cast<int>(w_char) >> 24) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((static_cast<int>(w_char) >> 18) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 12) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		}
	}
	while (w_char !=  static_cast<char>(0));

	// set the output charset
	*appszOutput_UTF8 = utf8;
	return buffer_size;
}

int UNICODE2UTF8(const wchar_t *awszInput_UNICODE,
						char * appszOutput_UTF8, uint32 auiBufferLen)
{
	//LMASSERT(auiBufferLen >= wcslen(awszInput_UNICODE));
	//LMASSERT(awszInput_UNICODE != NULL);
	//LMASSERT(appszOutput_UTF8 != NULL);

	if(auiBufferLen < wcslen(awszInput_UNICODE))
	{
		return -1;
	}
	if (awszInput_UNICODE == NULL && appszOutput_UTF8 == NULL) 
	{
		return -1;
	}

	int buffer_size = 0;

	wchar_t w_char;
	char *utf8  = appszOutput_UTF8;

	int index_buffer = 0;
	// do the conversion
	do {
		w_char = *awszInput_UNICODE;  // the unicode char current being converted
		awszInput_UNICODE++;

		if (w_char < 0x80) {
			// apiOutput_UTF8_Len = 1;
			utf8[index_buffer++] = static_cast<char>(w_char);
		} else if (w_char < 0x800) {
			// apiOutput_UTF8_Len = 2;
			utf8[index_buffer++] = 0xc0 | (w_char >> 6);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else if (w_char < 0x10000) {
			// apiOutput_UTF8_Len = 3;
			utf8[index_buffer++] = 0xe0 | (w_char >> 12);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else if (w_char < 0x200000) {
			// apiOutput_UTF8_Len = 4;
			utf8[index_buffer++] = 0xf0 | (static_cast<int>(w_char) >> 18);
			utf8[index_buffer++] = 0x80 | ((w_char >> 12) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else if (w_char < 0x4000000) {
			// apiOutput_UTF8_Len = 5
			utf8[index_buffer++] = 0xf8 | (static_cast<int>(w_char) >> 24);
			utf8[index_buffer++] = 0x80 | ((static_cast<int>(w_char) >> 18) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 12) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		} else {  // if(wchar >= 0x4000000)
			// all other cases apiOutput_UTF8_Len = 6
			utf8[index_buffer++] = 0xfc | (static_cast<int>(w_char) >> 30);
			utf8[index_buffer++] = 0x80 | ((static_cast<int>(w_char) >> 24) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((static_cast<int>(w_char) >> 18) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 12) & 0x3f);
			utf8[index_buffer++] = 0x80 | ((w_char >> 6) & 0x3f);
			utf8[index_buffer++] = 0x80 | (w_char & 0x3f);
		}
	}
	while (w_char !=  static_cast<char>(0));

	// set the output charset
	return buffer_size;
}
int UTF82UNICODE(const char *apszInput_UTF8,wchar_t * appszOutput_UNICODE, int32 aiUnicodeLen)
{
	const uint8 *lpUTF =(const uint8*) apszInput_UTF8;
	uint8 *lpTemp = (uint8*) appszOutput_UNICODE;
	//int32 liSize =0;
	int32 liTempUnicodeLen = 0;
	int32 liUnicodeLen = aiUnicodeLen-1;

	if(liUnicodeLen <=0)
		return -1;
	while(*lpUTF != 0)
	{
		if (*lpUTF >= 0x00 && *lpUTF <= 0x7f)
		{
			*lpTemp = *lpUTF;
			lpTemp++;
			*lpTemp = '\0';
			lpTemp++;
			//	liSize += 2;
		}
		else if ((*lpUTF & (0xff << 5))== 0xc0)
		{
			uint8 t1 = 0;
			uint8 t2 = 0;

			t1 = *lpUTF & (0xff >> 3);
			lpUTF++;
			t2 = *lpUTF & (0xff >> 2);

			*lpTemp = t2 | ((t1 & (0xff >> 6)) << 6);//t1 >> 2;
			lpTemp++;

			*lpTemp = t1 >> 2;//t2 | ((t1 & (0xff >> 6)) << 6);
			lpTemp++;

			//	liSize += 2;
		}
		else if ((*lpUTF & (0xff << 4))== 0xe0)
		{
			uint8 t1 = 0;
			uint8 t2 = 0;
			uint8 t3 = 0;

			t1 = *lpUTF & (0xff >> 3);
			lpUTF++;
			t2 = *lpUTF & (0xff >> 2);
			lpUTF++;
			t3 = *lpUTF & (0xff >> 2);

			//Little Endian
			*lpTemp = ((t2 & (0xff >> 6)) << 6) | t3;//(t1 << 4) | (t2 >> 2);
			lpTemp++;

			*lpTemp = (t1 << 4) | (t2 >> 2);//((t2 & (0xff >> 6)) << 6) | t3;
			lpTemp++;
			//	liSize += 2;
		}
		liTempUnicodeLen++;
		if(liTempUnicodeLen>=liUnicodeLen)
			break;
		lpUTF++;
	}

	*lpTemp = '\0';
	lpTemp++;
	*lpTemp = '\0';
	//liSize += 2;
	liTempUnicodeLen++;

	return liTempUnicodeLen;
}


u9_string TimeToString(long alFileTime)
{
	time_t ltimet = (time_t)alFileTime;
	time( &ltimet );
	struct tm loTimeFormat;
	loTimeFormat = *localtime( &ltimet );
	struct tm * lstrutime = &loTimeFormat;

	const int MAXSIZE = 512;
	U9_CHAR lszTime[MAXSIZE+1]={0};

#ifdef WIN32
	u9_sprintf(lszTime,_T("%4d%02d%02d"), 
		(1900+lstrutime->tm_year),(1+lstrutime->tm_mon), lstrutime->tm_mday);
#else
	u9_sprintf(lszTime, MAXSIZE, _T("%4d%02d%02d"),
		(1900+lstrutime->tm_year),(1+lstrutime->tm_mon), lstrutime->tm_mday);
#endif 
	u9_string lstrTime = lszTime;
	return lstrTime; 
}

bool GetVer(const U9_CHAR* astrFullVer, int32 aiLen,
			U9_CHAR *astrMajorVer,U9_CHAR *astrMinorVer,U9_CHAR *astrMiniVer)
{
	if(aiLen>DEF_VERSION_LEN || aiLen<DEF_MIN_VERSION_LEN)
		return false;
	U9_CHAR lszVer[DEF_VERSION_LEN+1];
	int i=0;
	int j=0;
	ZeroMemory(lszVer,sizeof(lszVer));
	for(;i<aiLen;i++)
	{
		if(astrFullVer[i]==_T('.'))
		{
			i++;//跳开这个 <.>标记
			break;
		}
		lszVer[j]= astrFullVer[i];
		j++;
	}
	if(j==0)
		return false;
	u9_strncpy(astrMajorVer,lszVer,j);
	
	if(i>=aiLen)
		return false;

	j=0;
	ZeroMemory(lszVer,sizeof(lszVer));
	for(;i<aiLen;i++)
	{
		if(astrFullVer[i]==_T('.'))
		{
			i++;//跳开这个 <.>标记
			break;
		}
		lszVer[j]= astrFullVer[i];
		j++;
	}
	if(j==0)
		return false;
	u9_strncpy(astrMinorVer,lszVer,j);

	if(i>=aiLen)
		return false;

	j=0;
	ZeroMemory(lszVer,sizeof(lszVer));
	for(;i<aiLen;i++)
	{
		if(astrFullVer[i]==_T('.'))
		{
			i++;//跳开这个 <.>标记
			break;
		}
		lszVer[j]= astrFullVer[i];
		j++;
	}
	if(j==0)
		return false;
	u9_strncpy(astrMiniVer,lszVer,j);
	return  true;
}



bool GetVer(const char* astrFullVer, int32 aiLen,
	char *astrMajorVer,char *astrMinorVer,char *astrMiniVer)
{
	if(aiLen>DEF_VERSION_LEN || aiLen<DEF_MIN_VERSION_LEN)
		return false;
	char lszVer[DEF_VERSION_LEN+1];
	int i=0;
	int j=0;
	ZeroMemory(lszVer,sizeof(lszVer));
	for(;i<aiLen;i++)
	{
		if(astrFullVer[i]=='.')
		{
			i++;//跳开这个 <.>标记
			break;
		}
		lszVer[j]= astrFullVer[i];
		j++;
	}
	if(j==0)
		return false;
	strncpy(astrMajorVer,lszVer,j);

	if(i>=aiLen)
		return false;

	j=0;
	ZeroMemory(lszVer,sizeof(lszVer));
	for(;i<aiLen;i++)
	{
		if(astrFullVer[i]=='.')
		{
			i++;//跳开这个 <.>标记
			break;
		}
		lszVer[j]= astrFullVer[i];
		j++;
	}
	if(j==0)
		return false;
	strncpy(astrMinorVer,lszVer,j);

	if(i>=aiLen)
		return false;

	j=0;
	ZeroMemory(lszVer,sizeof(lszVer));
	for(;i<aiLen;i++)
	{
		if(astrFullVer[i]=='.')
		{
			i++;//跳开这个 <.>标记
			break;
		}
		lszVer[j]= astrFullVer[i];
		j++;
	}
	if(j==0)
		return false;
	strncpy(astrMiniVer,lszVer,j);
	return  true;
}

string WStr2Str(wstring astr)
{
	if (astr.empty())
	{
		return string("");
	}
	
#ifdef WIN32
	int32 liLen = (int32)astr.size() * sizeof(wchar_t) + 1;
	char* lpTemp = new char[liLen];
	memset(lpTemp, 0, sizeof(char)*liLen);
	wtoa(astr.c_str(), lpTemp, liLen-1);
	SAFE_DELETE_ARRAY(lpTemp);
#else
	char* lpTemp = NULL;
	UNICODE2UTF8(astr.c_str(), &lpTemp);
#endif
	string lstrTemp = lpTemp;
	SAFE_DELETE_ARRAY(lpTemp);
	return lstrTemp;
}

time_t MakeHourRefreshTime(int32 aiTime )
{
	time_t liRefreshTime = aiTime - ((aiTime) % ENUM_SECONDS_PER_HOUR);
	return liRefreshTime;
}
time_t MakeDailiyRefreshTime(int32 aiTime)
{
	//giLocalTimer转换tm时间是按UTC时间转换,需要加8小时,成为北京时区
	time_t liRefreshTime = aiTime - ((aiTime + 8 * ENUM_SECONDS_PER_HOUR) % ENUM_SECONDS_PER_DAY);
	return liRefreshTime;
}

time_t MakeWeeklyRefreshTime(int32 aiTime)
{
	//giLocalTimer转换tm时间是按UTC时间转换,需要加8小时,成为北京时区
	const time_t liStartTime = 4 * ENUM_SECONDS_PER_DAY - 8 * ENUM_SECONDS_PER_HOUR;

	time_t liLastWeekRefreshTime = aiTime;
	liLastWeekRefreshTime -= liStartTime;
	liLastWeekRefreshTime -= liLastWeekRefreshTime % ENUM_SECONDS_PER_WEEK;
	liLastWeekRefreshTime += liStartTime;
	return liLastWeekRefreshTime;
}

uint64 atoull(const char* pszSrc)
{
    if (NULL == pszSrc)
    {
        return 0 ;
    }
    return strtoul(pszSrc, (char**)NULL, 0) ;
}

uint64 PhoneNumer2PhoneID(const char* pszPhoneNumber)
{
	return 0;
}

bool utf8cpy_ex(char* des,const char *src, int32 aiLen)
{
	const uint8 *lpUTF =(const uint8*) src;
	uint8 *lpTemp = (uint8*) des;

	int32 liTempLen = 0;

	if(aiLen <=0)
		return false;

	while(*lpUTF != 0)
	{
		if (*lpUTF >= 0x00 && *lpUTF <= 0x7f)
		{
			if(liTempLen+1 >aiLen)
				break;

			*lpTemp = *lpUTF;
			lpUTF++;
			lpTemp++;
			*lpTemp = '\0';
			liTempLen += 1;
		}
		else if ((*lpUTF & (0xff << 5))== 0xc0)
		{
			if(liTempLen+2 >aiLen)
				break;

			*lpTemp = *lpUTF;
			lpUTF++;
			lpTemp++;

			*lpTemp = *lpUTF;

			lpUTF++;
			lpTemp++;	

			liTempLen += 2;
		}
		else if ((*lpUTF & (0xff << 4))== 0xe0)
		{
			if(liTempLen+3 >aiLen)
				break;	

			*lpTemp = *lpUTF;
			lpUTF++;
			lpTemp++;

			*lpTemp = *lpUTF;

			lpUTF++;
			lpTemp++;	

			*lpTemp = *lpUTF;

			lpUTF++;
			lpTemp++;	

			liTempLen += 3;
		}
		else
		{
			if(liTempLen+1 >aiLen)
				break;

			*lpTemp = *lpUTF;
			lpUTF++;
			lpTemp++;
			*lpTemp = '\0';
			liTempLen += 1;
		}
	}

	*lpTemp = '\0';

	return true;
}