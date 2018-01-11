/*****************************************************************************/
/* crc32.h                                Copyright (c) Ladislav Zezula 2007 */
/*---------------------------------------------------------------------------*/
/* Description:                                                              */
/*---------------------------------------------------------------------------*/
/*   Date    Ver   Who  Comment                                              */
/* --------  ----  ---  -------                                              */
/* 11.06.07  1.00  Lad  The first version of crc32.h                         */
/*****************************************************************************/

// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: crc32.h
// 创 建 人: 史云杰
// 文件说明: 计算Crc32值的类,对原有代码进行封装,封装成CCrc32类
// $_FILEHEADER_END ******************************

#ifndef __CRC32_H_
#define __CRC32_H_
#include "base0/platform.h"
#include "base0/u9_string.h"
class CCrc32
{
public:

	// $_FUNCTION_BEGIN ******************************
	// 方法名: ComputeCrc32
	// 访  问: public 
	// 参  数: uint8 * apuiFileData	数据
	// 参  数: uint32 auiLength	数据的长度
	// 参  数: uint32 * apuiOut	[OUT]输出的crc32值
	// 返回值: bool
	// 说  明:	计算一段二进制数据的Crc32值
	// $_FUNCTION_END ********************************
	bool ComputeCrc32(const uint8* apuiFileData, uint32 auiLength, uint32* apuiOut);


private:
	struct crc32_context
	{
		unsigned long value;
	};

	void CRC32_Init(crc32_context * ctx);
	void CRC32_Update(crc32_context * ctx, const unsigned char *input, int ilen);
	void CRC32_Finish(crc32_context * ctx, uint32 * value);

#endif // __CRC32_H__
};