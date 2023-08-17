#include "pch.h"
#include "Packet.h"

CPacket::CPacket() :sHead(0), nLength(0), sCmd(0), sSum(0) {}

CPacket& CPacket::operator=(const CPacket& pack)
{
	if (this != &pack)
	{
		sHead = pack.sHead;
		nLength = pack.nLength;
		sCmd = pack.sCmd;
		strData = pack.strData;
		sSum = pack.sSum;
	}
	return *this;
}

CPacket::CPacket(WORD nCmd, const BYTE* pData, size_t nSize)
{
	sHead = 0xFEFF;
	nLength = nSize + 4;
	sCmd = nCmd;
	if (nSize > 0)
	{ // 直接就是一次性发完的
		strData.resize(nSize);
		memcpy((void*)strData.c_str(), pData, nSize);
	}
	else {
		strData.clear();
	}
	sSum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sSum += BYTE(strData[j]) & 0xFF;
	}
}

CPacket::CPacket(const BYTE* pData, size_t& nSize) :sHead(0), nLength(0), sCmd(0), sSum(0)
{
	size_t i = 0;
	for (; i < nSize; i++)
	{
		if (*(WORD*)(pData + i) == 0xFEFF) {
			sHead = *(WORD*)(pData + i);//?
			i += 2;
			break;
		}
	}
	if (i + 4 + 2 + 2 > nSize)
	{
		nSize = 0;
		return;
	}
	nLength = *(DWORD*)(pData + i);//?  nLength长度
	i += 4;
	if (nLength + i > nSize)
	{
		nSize = 0;
		return;
	}
	sCmd = *(WORD*)(pData + i);//?
	i += 2;
	if (nLength > 4)
	{
		strData.resize(nLength - 2 - 2);
		memcpy((void*)strData.c_str(), pData + i, nLength - 4);
		i += nLength - 4;
	}
	sSum = *(WORD*)(pData + i); i += 1;
	WORD sum = 0;
	for (size_t j = 0; j < strData.size(); j++)
	{
		sum += BYTE(strData[j]) & 0xFF;//保持二进制补码的一致性，消除负数
	}
	if (sum == sSum) {
		nSize = nLength + 2 + 4;
		return;
	}
}

CPacket::CPacket(const CPacket& pack)
{
	sHead = pack.sHead;
	nLength = pack.nLength;
	sCmd = pack.sCmd;
	strData = pack.strData;
	sSum = pack.sSum;
}

int CPacket::pacSize()
{
	return nLength + 6;
}

const char* CPacket::pacData()
{
	strOut.resize(nLength + 6);
	BYTE* pData = (BYTE*)strOut.c_str();
	*(WORD*)pData = sHead; pData += 2;
	*(DWORD*)pData = nLength; pData += 4;
	*(WORD*)pData = sCmd; pData += 2;
	memcpy(pData, strData.c_str(), strData.size()); pData += strData.size();
	*(WORD*)pData = sSum;
	return strOut.c_str();
}