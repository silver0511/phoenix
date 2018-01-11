// $_FILEHEADER_BEGIN ****************************
// 版权声明：xxxxx版权所有
//			  
// 文件名称：PackSerial.h
// 创建人： 史云杰
// 文件说明：包id
// $_FILEHEADER_END ******************************
#ifndef __PACKSERIAL_H
#define __PACKSERIAL_H

//网络数据包
template<class T>
class CPackSerial 
{
public:
	CPackSerial()
	{
		mPackSerial=rand()&65535;	
	};

	T GetPackSerial()
	{
		T lSerialID=0;
		moCriticalSection.Enter();
		lSerialID=mPackSerial++;
		moCriticalSection.Leave();
		return lSerialID;
	};
private:
	T mPackSerial;
	U9::CCriticalSection moCriticalSection;
 };

#endif// __PACKSERIAL_H

