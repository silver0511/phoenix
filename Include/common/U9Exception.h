// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称: U9Exception.h
// 创 建 人: 史云杰
// 文件说明:  异常处理
// $_FILEHEADER_END ******************************
#ifndef __U9EXCEPTION_H_
#define __U9EXCEPTION_H_


#define DEF_RESULT_DESC_LEN (256)

class CU9Exception
{
public:

	char mszFunctionName[DEF_RESULT_DESC_LEN + 1];

	char mszExceptionDesc[DEF_RESULT_DESC_LEN + 1];

private:
	uint8 miExceptionLv;
public:
	CU9Exception();

	void Reset();

	void SetExceptionLv(uint8 aiLevel);

	void Dump();

	void Execute();
};

#endif//__U9EXCEPTION_H_

