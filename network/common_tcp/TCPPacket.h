// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: TCPPacket.h
// 创 建 人: 史云杰
// 文件说明: TCP 数据包定义类实现文件
// $_FILEHEADER_END *****************************

#ifndef __TCPPACKET_H
#define __TCPPACKET_H

#include "base0/platform.h"
#include "const/constdef.h"
#include "const/SvrConstDef.h"
#include "const/ErrorCodeDef.h"

extern int giLocalTime;
//TCP原始包
class CRawTCPRecvPack
{
public:
	BYTE	mpData[DEF_MAX_TCP_PACKET_LEN];//数据
	int		miDataLen;//长度
	SOCKET	mhSocket;
	IPTYPE	mulIPAddr;	//来源地址.网络字节序
	WORD	mwIPPort;	//from端口
	BOOL	mbRawData;	//是否是原始数据
	void	*mpSocketInfo;//socket信息
};

///////////////////////////////////////////////////////////////////////////////
// CTCPPacket
class CTCPPacketHeader
{
public:
	CTCPPacketHeader();
	virtual ~CTCPPacketHeader(){};


	// 设置是否需要返回Ack
	inline void setAckFlag(bool abNeedAck);

	// 设置是否是Ack数据包
	inline void setAckPacket(bool abIsAck);

	// 设置包的版本号
	inline void setPacketVersion(unsigned char abyVer);

	// 设置包的加密版本号
	inline void setPacketEncryVersion(unsigned char abyVer);

	inline BYTE getPacketEncryVersion();

	// 获取是否需要Ack
	inline BOOL getAckFlag();

	// 获取是否是Ack的包
	inline BOOL getAckPacket();

	static INT Pack(BYTE *apOutBuf, INT aiOutBufLen,
		BYTE *apData,INT aiLength,int8 aiEncryptType);

protected:
	enum {TF_ACK = 0x80, TP_ACK = 0x40};
	unsigned char mpFlags[2];
};

class CTCPPacket:public CTCPPacketHeader
{

public:
	CTCPPacket();

	virtual ~CTCPPacket();

	void init();

public:
	// 设置对端的socket 值
	inline void setPeerSocket(SOCKET aiSock);

	INT static GetPack(BYTE *lpbuf, INT aiLen);
	INT static GetBPack(BYTE *lpbuf, INT aiLen);
public:
	// 对一段缓冲区进行打包
	int  Pack(BYTE *apData, INT aiLength,int8 aiEncryptType);
	// 对接收到的数据进行解包
	int  UnPack(BYTE *apData, INT aiLength,int8 aiEncryptType);
	INT UnPackB(BYTE *apData, INT aiLength, SESSIONID &aiSessionID);

    // 对一段缓冲区进行打包
    inline int  PackRawData(BYTE *apData, INT aiLength,int8 aiEncryptType);
    // 对接收到的数据进行解包
    inline int  UnPackRawData(BYTE *apData, INT aiLength,int8 aiEncryptType);

public:
	// 获取缓冲区指针
	inline BYTE *getDataPtr();

	// 获取包头的长度
	inline unsigned short getHeaderLen();

	// 获取整个包的长度
	inline unsigned short getPacketLen();

	// 获取数据的长度
	inline unsigned short getDataLen();

protected:
	// 存放数据buffer
	BYTE mpData[DEF_PACKET_LEN];

	unsigned short miPacketLen;
	unsigned short miDataLen;
	unsigned short miHeaderLen;
};

int CTCPPacket::PackRawData( BYTE *apData, INT aiLength,int8 aiEncryptType )
{
    if (aiLength >= DEF_PACKET_LEN)
    {
        return -1;
    }
    miDataLen = aiLength;
    miHeaderLen = 0;
    miPacketLen = aiLength;
    memcpy(&mpData[0], apData, aiLength);
    return miPacketLen;
}

int CTCPPacket::UnPackRawData(BYTE *apData, INT aiLength,int8 aiEncryptType)
{
    //确认数据必须是正确的
    U9_ASSERT(apData);
    U9_ASSERT(aiLength < DEF_PACKET_LEN);

    miDataLen = aiLength;
    miPacketLen = aiLength;
    memcpy(mpData, apData, miDataLen);
    return miDataLen;
}


class CSendTCPPacket:public CTCPPacket
{
public:
	CSendTCPPacket();
	virtual ~CSendTCPPacket();

	inline SOCKET getPeerSocket(){return miPeerSocket;}
	inline IPTYPE getPeerAddr(){return miPeerAddr;}
	inline WORD   getPeerPort(){return mwPeerPort;}
	inline WORD   getNeedClose(){return mbNeedClose;}
	// 设置对端的socket 值
	inline void setPeerSocket(SOCKET ahSocket){miPeerSocket = ahSocket;}
	inline void setPeerAddr(IPTYPE aValue){miPeerAddr=aValue;}
	inline void setPeerPort(WORD aValue){mwPeerPort=aValue;}
	inline void setNeedClose(BOOL abValue){mbNeedClose=abValue;}
private:
	SOCKET         miPeerSocket;
	IPTYPE         miPeerAddr;
	WORD           mwPeerPort;
	BOOL		   mbNeedClose;
};

///////////////////////////////////////////////////////////////////////////////
// implementation the inline functions

inline void CTCPPacketHeader::setAckFlag(bool abNeedAck)
{
	if (abNeedAck)
		mpFlags[1] |= TF_ACK;
	else
		mpFlags[1] &= ~TF_ACK;
}


inline void CTCPPacketHeader::setAckPacket(bool abIsAck)
{
	if (abIsAck)
		mpFlags[1] |= TP_ACK;
	else
		mpFlags[1] &= ~TP_ACK;
}


inline void CTCPPacketHeader::setPacketVersion(unsigned char abyVer)
{
	abyVer &= 0xF0;
	mpFlags[0] &= 0xF0;
	mpFlags[0] |= abyVer;
}

//加密版本只能支持16个版本
inline void CTCPPacketHeader::setPacketEncryVersion(unsigned char abyVer)
{
	abyVer &= 0x0F;
	mpFlags[0] &= 0x0F;
	mpFlags[0] |= abyVer;
}
inline BYTE CTCPPacketHeader::getPacketEncryVersion()
{
	BYTE lbyEncry = mpFlags[0] & 0x0f;
	return lbyEncry;
}


inline BOOL CTCPPacketHeader::getAckFlag()
{
	return (BOOL)(mpFlags[1] & TF_ACK);
}


inline BOOL CTCPPacketHeader::getAckPacket()
{
	return (BOOL)(mpFlags[1] & TP_ACK);
}

//////////////////////////////////////////////////////////////////////////////
//CTCPPacket

inline  BYTE *CTCPPacket::getDataPtr()
{
	return mpData;
}

inline unsigned short CTCPPacket::getHeaderLen()
{
	return miHeaderLen;
}

inline unsigned short CTCPPacket::getPacketLen()
{
	return miPacketLen;
}

inline unsigned short CTCPPacket::getDataLen()
{
	return miDataLen;
}


inline INT CTCPPacket::GetPack(BYTE *lpbuf, INT aiLen)
{
	//数据头没有收到
	if(aiLen < DEF_TCP_PACKHEAD_LEN)
	{
		return 0;
	}

	unsigned short liPackLen = *(unsigned short*)lpbuf;

	if(liPackLen > DEF_PACKET_LEN || liPackLen < 4)
	{
		//数据非法
		return -1;
	}

	if(liPackLen > aiLen)
	{
		//包没有全
		return 0;
	}

	return liPackLen;
}

//后台服务器解包
inline INT CTCPPacket::GetBPack(BYTE *lpbuf, INT aiLen)
{
	//数据头没有收到
	if(aiLen < DEF_TCP_PACKHEAD_LEN)
	{
		return 0;
	}

	unsigned short liPackLen = *(unsigned short*)lpbuf;

	//len+packtype+sessionid
	if(liPackLen > DEF_PACKET_LEN || liPackLen < 12)
	{
		//数据非法
		return -1;
	}

	if(liPackLen > aiLen)
	{
		//包没有全
		return 0;
	}

	return liPackLen;
}

#endif // __TCP_PACKET_H__
