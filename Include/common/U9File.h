// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9File.h
// 创 建 人: 史云杰
// 文件说明: 标准文件处理
// $_FILEHEADER_END ******************************

#ifndef __U9FILE_H
#define __U9FILE_H

#include "sys/types.h"
#include "sys/stat.h"
#include "base0/platform.h"
#include "base0/u9_string.h"
#include "common/md5.h"
#include "common/crc32.h"

#ifndef WIN32
#define _vsnprintf vsnprintf
// stat函数
#define _stat stat
// 文件FILE*转文件句柄
#define _fileno fileno
#endif

// $_FUNCTION_BEGIN ******************************
// 方法名: U9_OpenFile
// 访  问: public 
// 参  数: const U9_CHAR * apszFileName
// 参  数: const U9_CHAR * apszMode
// 返回值: FILE * __cdecl
// 说  明: 文件打开
// $_FUNCTION_END ********************************
FILE * __cdecl U9_OpenFile(const U9_CHAR * apszFileName, const U9_CHAR * apszMode);


// $_FUNCTION_BEGIN ******************************
// 方法名: U9_GetFileSize
// 访  问: public 
// 参  数: const U9_CHAR * apszFileName	文件名
// 返回值: size_t	大小
// 说  明: 得到文件大小
// $_FUNCTION_END ********************************
size_t U9_GetFileSize(const U9_CHAR* apszFileName);

// $_FUNCTION_BEGIN ******************************
// 方法名: U9_Rename
// 访  问: public 
// 参  数: const U9_CHAR * apszOldPath	旧文件件名
// 参  数: const U9_CHAR * apszNewPath	新文件名
// 返回值: int	0为正确,-1为错误
// 说  明:	 文件改名
// $_FUNCTION_END ********************************
int	U9_Rename(const U9_CHAR* apszOldPath,const U9_CHAR* apszNewPath );

// $_FUNCTION_BEGIN ******************************
// 方法名: U9_Unlink
// 访  问: public 
// 参  数: const U9_CHAR * aszRelativePath	文件名
// 返回值: int	0为正确,-1为错误
// 说  明:	删除文件
// $_FUNCTION_END ********************************
int U9_Unlink(const U9_CHAR* aszRelativePath);

// $_FUNCTION_BEGIN ******************************
// 方法名: U9_Stat
// 访  问: public 
// 参  数: const U9_CHAR * aszPath 文件名
// 参  数: struct _stat * abuf	状态缓存
// 返回值: int	0为正确,-1为错误
// 说  明:	获得文件的状态信息
// $_FUNCTION_END ********************************
int U9_Stat(const U9_CHAR *aszPath, struct _stat *abuf);


//-------------------------------------------------------------------------------------
//类定义,从效率考虑,为了不要每次计算都声明一个类,而是一次声明多次计算,所以定义成类形式
//------------------------------------------------------------------------------------

// 从文件中计算md5值的类
class CMd5FromFile : public CMD5
{
public:

	// $_FUNCTION_BEGIN ******************************
	// 方法名: MD5_2_BinaryFromFile
	// 访  问: public 
	// 参  数: U9_CHAR * lszFileName	输入的文件名
	// 参  数: BYTE * apszOutput	输出数据，以二进制方式
	// 参  数: int aiLen	输出数据的长度,应该为16
	// 参  数: int aiIteration 迭代次数,一般默认即可
	// 返回值: bool 为了不添加错误代码,加大md5库的大小,仅仅以bool表示成功还是失败
	// 说  明:	以文件名为输入的md5散列运算,输出为二进制
	// $_FUNCTION_END ********************************
	bool MD5_2_BinaryFromFile(const U9_CHAR* aszFileName,  BYTE* apszOutput, int aiLen,int aiIteration=1);

	// $_FUNCTION_BEGIN ******************************
	// 方法名: MD5_2_HEXASCIIFromFile
	// 访  问: public 
	// 参  数: U9_CHAR * lszFileName	输入的文件名
	// 参  数: BYTE * apszOutput	输出数据，以16进制ascii码方式输出
	// 参  数: int aiLen	输出数据的长度,应该为32
	// 参  数: int aiIteration	迭代次数,一般默认即可
	// 返回值: bool	为了不添加错误代码,加大md5库的大小,仅仅以bool表示成功还是失败
	// 说  明: 以文件名为输入的md5散列运算,输出为ASC2值
	// $_FUNCTION_END ********************************
	bool MD5_2_HEXASCIIFromFile(const U9_CHAR* aszFileName, BYTE * apszOutput, int aiLen, int aiIteration=1);


	// $_FUNCTION_BEGIN ******************************
	// 方法名: MD5_2_HEXU16
	// 访  问: public 
	// 参  数: const BYTE * apszInput	输入数据
	// 参  数: unsigned int aiInputLen	输入数据长度
	// 参  数: wchar_t * apszOutput	输出数据，以16进制ascii码方式输出
	// 参  数: int aiLen
	// 参  数: int aiIteration
	// 返回值: void
	// 说  明: md5散列运算 到unicode16,以宽字节输出
	// $_FUNCTION_END ********************************
	bool MD5_2_HEXU16FromFile(const U9_CHAR* aszFileName, wchar_t* apszOutput, int aiLen,int aiIteration=1);
};

// 从文件中计算Crc32值的类
class CCrc32FromFile : public CCrc32
{
public :

	// $_FUNCTION_BEGIN ******************************
	// 方法名: ComputeCrc32FromFile
	// 访  问: public 
	// 参  数: U9_CHAR * aszFileName	文件名
	// 参  数: uint32 * apuiOut	[OUT]输出的crc32值
	// 返回值: bool	计算是否成功
	// 说  明:	 计算一个文件的Crc32值
	// $_FUNCTION_END ********************************
	bool ComputeCrc32FromFile(const U9_CHAR* aszFileName, uint32* apuiOut);
};



#endif //__U9_FILE_H_


