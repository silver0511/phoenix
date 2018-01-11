// $_FILEHEADER_BEGIN ***************************
// 版权声明:xxxxx版权所有
//  
// 文件名称: TCPPacket.cpp
// 创 建 人: 史云杰
// 文件说明: TCP 数据包定义类实现文件
// $_FILEHEADER_END *****************************
#include "stdafx.h"
#include "encrypt/Encrypt.h"
#include "encrypt/TeaEncrypt.h"
#include "./TCPPacket.h"

int giLocalTime=0;
CTCPPacketHeader::CTCPPacketHeader()
{
	*(WORD*)mpFlags = 0;
	setAckFlag(false);
	setAckPacket(false);
}
//////////////////////////////////////////////////////////////////////////////
// CTCPPacket 构造
CTCPPacket::CTCPPacket():CTCPPacketHeader()
,	miPacketLen(0)
, miHeaderLen (0)
, miDataLen (0)
{
		
}
void CTCPPacket::init()
{
	miPacketLen = 0;
	miHeaderLen = 0;
	miDataLen = 0;
	mpFlags[0] = 0;
	mpFlags[1] = 0;

}
// CTCPPacket 析构
CTCPPacket::~CTCPPacket()
{
}


INT CTCPPacketHeader::Pack(BYTE *apOutBuf, INT aiOutBufLen, 
								  BYTE *apData,INT aiLength,
								  int8 aiEncryptType)
{
	if (!apOutBuf ||
		!apData   ||
		aiOutBufLen < (aiLength+DEF_TCP_PACKHEAD_LEN))
	{
		return -1;
	}

	BYTE *lpBuffer = apOutBuf;
	*(WORD*)lpBuffer = aiLength + DEF_TCP_PACKHEAD_LEN;
	lpBuffer += 2;

	memcpy(lpBuffer, apData, aiLength);
	return (aiLength + DEF_TCP_PACKHEAD_LEN);
}

CSendTCPPacket::CSendTCPPacket()
{
	miPeerSocket = 0;
	miPeerAddr = 0;
	mwPeerPort = 0;
}

CSendTCPPacket::~CSendTCPPacket()
{
}


INT CTCPPacket::Pack(BYTE *apData, INT aiLength,int8 aiEncryptType)
{
	// 包长度
	BYTE *lpBuffer = mpData;
	*(WORD*)lpBuffer = aiLength + DEF_TCP_PACKHEAD_LEN;
	lpBuffer += 2;

	miPacketLen = aiLength + DEF_TCP_PACKHEAD_LEN;
	miDataLen = aiLength;
	miHeaderLen = DEF_TCP_PACKHEAD_LEN;


	memcpy(lpBuffer, apData, aiLength);

	return miPacketLen;
}

INT CTCPPacket::UnPack(BYTE *apData, INT aiLength,int8 aiEncryptType)
{
	//确认数据必须是正确的
	U9_ASSERT(apData);
	U9_ASSERT(aiLength < DEF_PACKET_LEN);

	miDataLen = aiLength - DEF_TCP_PACKHEAD_LEN;

	// 包长度
	BYTE *lpBuffer = apData;
	miPacketLen = *(WORD*)lpBuffer;
	lpBuffer += 2;

	LOG_TRACE(LOG_DEBUG_1, TRUE, __FUNCTION__, "CTCPPacket packetlen: " << miPacketLen
		<< "datalen = " << miDataLen);

	//包里面没有加密类型，需要判断本地的
	memcpy(mpData, lpBuffer, miDataLen);
	return miDataLen;
}

//后台解包
INT CTCPPacket::UnPackB(BYTE *apData, INT aiLength, SESSIONID &aiSessionID)
{
	//确认数据必须是正确的
	U9_ASSERT(apData);
	U9_ASSERT(aiLength < DEF_PACKET_LEN);

	aiSessionID = 0;
	miDataLen = aiLength - DEF_TCP_PACKHEAD_LEN;

	// 包长度
	BYTE *lpBuffer = apData;
	miPacketLen = *(WORD*)lpBuffer;
	lpBuffer += 2;

	//packtype
	BYTE *p_buf = lpBuffer;
	p_buf += 2;
	aiSessionID = *(SESSIONID *)p_buf;

	LOG_TRACE(LOG_DEBUG_1, TRUE, __FUNCTION__, "CTCPPacket packetlen: " << miPacketLen
											 <<"sessionid: " << aiSessionID
											 << "datalen = " << miDataLen);

	//包里面没有加密类型，需要判断本地的
	memcpy(mpData, lpBuffer, miDataLen);
	return miDataLen;
}