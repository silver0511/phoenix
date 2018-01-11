// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9File.cpp
// 创 建 人: 史云杰
// 文件说明: 标准文件处理
// $_FILEHEADER_END ******************************

#include "stdafx.h"
#include "common/basefun.h"
#include "common/U9File.h"


//-----------------------------------------------------------------------------
#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4389)		// warning C4389: '==' : signed/unsigned mismatch
#pragma warning(disable : 4267)		// warning C4267: 'argument' : conversion from 'size_t' to 'unsigned int', possible loss of data
#endif

FILE * __cdecl U9_OpenFile(const U9_CHAR * apszFileName, const U9_CHAR * apszMode)
{
#ifdef WIN32 
	return u9_fopen(apszFileName,apszMode);
#else

#ifndef UNICODE
	return fopen(apszFileName,apszMode);
#else
	char *lszFileName=NULL;
	char *lszMode=NULL;
	if(0>=UNICODE2UTF8(apszFileName,&lszFileName))
		return NULL;

	if(0>=UNICODE2UTF8(apszMode,&lszMode))
	{
		if(NULL != lszFileName)
		{
			delete[] lszFileName;
		}
		return NULL;
	}
	FILE *lpFile=fopen(lszFileName,lszMode);
	if(NULL != lszFileName)
	{
		delete[] lszFileName;
	}
	if(NULL != lszMode)
	{
		delete[] lszMode;
	}
	return lpFile;
#endif 

#endif
}
size_t U9_GetFileSize(const U9_CHAR* apszFileName)
{
	FILE* fp = NULL;
	fp = U9_OpenFile(apszFileName, _T("rb"));
	if (NULL == fp)
		return 0;
	fseek(fp, 0, SEEK_END);
	size_t liFileSize = ftell(fp);
	fclose(fp);
	return liFileSize;
}

int	U9_Rename(const U9_CHAR* apszOldPath,const U9_CHAR* apszNewPath )
{
#ifdef WIN32
#ifdef UNICODE
	int fail = _wrename( apszOldPath, apszNewPath );
#else
	int fail = rename( apszOldPath, apszNewPath );
#endif
#else
#ifndef UNICODE
	int fail = rename( apszOldPath, apszNewPath );
#else

	char *lszOldPath = NULL;
	char *lszNewPath = NULL;
	if(0>=UNICODE2UTF8(apszOldPath, &lszOldPath))
		return -1;

	if(0>=UNICODE2UTF8(apszNewPath, &lszNewPath))
	{
		if(NULL != lszOldPath)
		{
			delete[] lszOldPath;
			lszOldPath = NULL;
		}
		return -1;
	}
	int fail = rename( lszOldPath, lszNewPath );
	if(NULL != lszOldPath)
	{
		delete[] lszOldPath;
		lszOldPath = NULL;

	}
	if(NULL != lszNewPath)
	{
		delete[] lszNewPath;
		lszNewPath = NULL;
	}
#endif
#endif

	return fail;	
}


int U9_Unlink(const U9_CHAR* aszRelativePath)
{
#ifdef WIN32
#ifdef UNICODE
	int fail = _wunlink( aszRelativePath );
#else
	int fail = _unlink( aszRelativePath );
#endif
#else
#ifndef UNICODE
	int fail = unlink( aszRelativePath );
#else

	char *lszRelativePath = NULL;
	if(0>=UNICODE2UTF8(aszRelativePath,&lszRelativePath))
		return -1;

	int fail = unlink( lszRelativePath );

	if(NULL != lszRelativePath)
	{
		delete[] lszRelativePath;
		lszRelativePath = NULL;
	}
#endif
#endif

	return fail;
}


int U9_Stat(const U9_CHAR *aszPath, struct _stat *abuf)
{
	int liFail = -1;
#ifdef WIN32
#ifdef UNICODE
	liFail = _wstat(aszPath, abuf);
#else
	liFail = _stat(aszPath, abuf);
#endif
#else
#ifdef UNICODE
	char *lszPath;
	if(UNICODE2UTF8(aszPath, &lszPath))
	{
		liFail = stat(lszPath, abuf);
	}
	if(NULL !=lszPath)
	{
		delete[] lszPath;
		lszPath = NULL;
	}
#else
	liFail = stat(aszPath, abuf);	
#endif	
#endif
	return liFail;
}



// $_FUNCTION_BEGIN ******************************
// 方法名: MD5_2_BinaryFromFile
// 访  问: public 
// 参  数: U9_CHAR * lszFileName	输入的文件名
// 参  数: BYTE * apszOutput	输出数据，以二进制方式
// 参  数: int aiLen	输出数据的长度,应该为16
// 参  数: int aiIteration
// 返回值: bool 为了不添加错误代码,加大md5库的大小,仅仅以bool表示成功还是失败
// 说  明:	以文件名为输入的md5散列运算,输出为二进制
// $_FUNCTION_END ********************************
bool CMd5FromFile::MD5_2_BinaryFromFile(const U9_CHAR* aszFileName,  BYTE* apszOutput, int aiLen,int aiIteration)
{
	if(aiLen<16) return false;

	bool lbReturn = true;
	FILE* lpFile = NULL;
	lpFile = U9_OpenFile(aszFileName, _T("rb"));

	if(NULL == lpFile)
	{
		return false;
	}

	struct _stat lbuf;
	int rt = U9_Stat(aszFileName, &lbuf);

	if(-1 == rt)
	{
		return false;
	}

	uint8* lucBuffer = NULL;
	lucBuffer = new uint8[lbuf.st_size];
	if(NULL == lucBuffer)
	{
		return false;
	}
	size_t luiTransferred = ::fread(lucBuffer, 1, lbuf.st_size, lpFile);
	assert(luiTransferred == lbuf.st_size);

	if(luiTransferred != lbuf.st_size)
	{
		lbReturn = false;
	}

	MD5_2_Binary(lucBuffer, lbuf.st_size, apszOutput, aiLen, aiIteration);

	SAFE_DELETE_ARRAY(lucBuffer);

	if(lpFile != NULL)
	{
		fclose(lpFile);
		lpFile = NULL;
	}

	return lbReturn;
}

// $_FUNCTION_BEGIN ******************************
// 方法名: MD5_2_HEXASCIIFromFile
// 访  问: public 
// 参  数: U9_CHAR * lszFileName	输入的文件名
// 参  数: BYTE * apszOutput	输出数据，以16进制ascii码方式输出
// 参  数: int aiLen	输出数据的长度,应该为32
// 参  数: int aiIteration
// 返回值: bool	为了不添加错误代码,加大md5库的大小,仅仅以bool表示成功还是失败
// 说  明: 以文件名为输入的md5散列运算,输出为ASC2值
// $_FUNCTION_END ********************************
bool CMd5FromFile::MD5_2_HEXASCIIFromFile(const U9_CHAR* aszFileName, BYTE * apszOutput, int aiLen, int aiIteration)
{
	if(aiLen<16) return false;

	bool lbReturn = true;
	FILE* lpFile = NULL;
	lpFile = U9_OpenFile(aszFileName, _T("rb"));

	if(NULL == lpFile)
	{
		return false;
	}

	struct _stat lbuf;
	int rt = U9_Stat(aszFileName, &lbuf);

	if(-1 == rt)
	{
		return false;
	}

	uint8* lucBuffer = NULL;
	lucBuffer = new uint8[lbuf.st_size];
	if(NULL == lucBuffer)
	{
		return false;
	}
	size_t luiTransferred = ::fread(lucBuffer, 1, lbuf.st_size, lpFile);
	assert(luiTransferred == lbuf.st_size);

	if(luiTransferred != lbuf.st_size)
	{
		lbReturn = false;
	}

	MD5_2_HEXASCII(lucBuffer, lbuf.st_size, apszOutput, aiLen, aiIteration);

	SAFE_DELETE_ARRAY(lucBuffer);

	if(lpFile != NULL)
	{
		fclose(lpFile);
		lpFile = NULL;
	}

	return lbReturn;
}

extern wchar_t BIN_2_U16[16];
bool CMd5FromFile::MD5_2_HEXU16FromFile(const U9_CHAR* aszFileName, wchar_t* apszOutput, int aiLen,int aiIteration/* =1 */)
{
	if(aiLen<16) return false;

	bool lbReturn = true;
	FILE* lpFile = NULL;
	lpFile = U9_OpenFile(aszFileName, _T("rb"));

	if(NULL == lpFile)
	{
		return false;
	}

	const size_t ONCEREAD = 4096;
	uint8* lucBuffer = NULL;
	lucBuffer = new uint8[ONCEREAD];
	if(NULL == lucBuffer)
	{
		return false;
	}
	BYTE lpBinMD5[16];
	Init(&mstruContext);

	size_t luiTransferred = 0;
	while( (luiTransferred = ::fread(lucBuffer, 1, ONCEREAD, lpFile)) != 0)
	{
		Update(&mstruContext, lucBuffer, luiTransferred);
	}

	Final(lpBinMD5, &mstruContext);

	SAFE_DELETE_ARRAY(lucBuffer);

	if(lpFile != NULL)
	{
		fclose(lpFile);
		lpFile = NULL;
	}

	for (int i=0, j=0;i<16;i++)
	{
		apszOutput[j++]=BIN_2_U16[lpBinMD5[i]>>4];//低4位
		apszOutput[j++]=BIN_2_U16[lpBinMD5[i]&0xf];//高4位
	}

	return lbReturn;
}

bool CCrc32FromFile::ComputeCrc32FromFile(const U9_CHAR* aszFileName, uint32* apuiOut)
{
	FILE* lpFile = NULL;
	lpFile = U9_OpenFile(aszFileName, _T("rb"));

	if(NULL == lpFile)
	{
		return false;
	}

	struct _stat lbuf;
	int rt = U9_Stat(aszFileName, &lbuf);

	if(-1 == rt)
	{
		return false;
	}

	uint8* lucBuffer = NULL;
	lucBuffer = new uint8[lbuf.st_size];
	if(NULL == lucBuffer)
	{
		return false;
	}
	size_t luiTransferred = ::fread(lucBuffer, 1, lbuf.st_size, lpFile);
	assert(luiTransferred == lbuf.st_size);

	bool lbReturn = true;
	if(luiTransferred != lbuf.st_size)
	{
		lbReturn = false;
	}

	if(lbReturn)
	{
		lbReturn = ComputeCrc32(lucBuffer, lbuf.st_size, apuiOut);
	}

	if(NULL != lucBuffer)
	{
		delete[] lucBuffer;
		lucBuffer = NULL;
	}

	if(lpFile != NULL)
	{
		fclose(lpFile);
		lpFile = NULL;
	}

	return lbReturn;
}
//-----------------------------------------------------------------------------

#ifdef _WIN32
#pragma warning(pop)
#endif